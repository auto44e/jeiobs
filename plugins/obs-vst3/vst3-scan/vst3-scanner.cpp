/******************************************************************************
    Copyright (C) 2026 pkv <pkv@obsproject.com>
    This file is part of obs-vst3.
    It uses the Steinberg VST3 SDK, which is licensed under MIT license.
    See https://github.com/steinbergmedia/vst3sdk for details.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <VST3Scanner.h>

#include <obs-module.h>
#include <util/pipe.h>
#include <util/platform.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <future>
#include <limits>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

VST3Scanner *list;
std::string vst3ConfigPath;

namespace {
constexpr int kDefaultProbeTimeoutMs = 30000;
constexpr int kMinimumProbeTimeoutMs = 500;
constexpr int kMaximumProbeTimeoutMs = 120000;
constexpr int kProbeWatchdogExitCode = 86;
constexpr size_t kProbeOutputBufferSize = 4096;

#ifdef _WIN32
constexpr char kVST3ScannerExecutable[] = "obs-vst3-scanner.exe";
#else
constexpr char kVST3ScannerExecutable[] = "obs-vst3-scanner";
#endif

[[noreturn]] void probeExit(int status)
{
#ifdef _WIN32
	TerminateProcess(GetCurrentProcess(), static_cast<UINT>(status));
	std::abort();
#else
	_exit(status);
#endif
}

unsigned long long currentProcessId()
{
#ifdef _WIN32
	return static_cast<unsigned long long>(_getpid());
#else
	return static_cast<unsigned long long>(getpid());
#endif
}

int clampProbeTimeout(long timeoutMs)
{
	if (timeoutMs < kMinimumProbeTimeoutMs) {
		return kMinimumProbeTimeoutMs;
	}
	if (timeoutMs > kMaximumProbeTimeoutMs) {
		return kMaximumProbeTimeoutMs;
	}
	return static_cast<int>(timeoutMs);
}

int getProbeTimeoutMs()
{
	const char *value = std::getenv("OBS_VST3_PROBE_TIMEOUT_MS");
	if (!value || !*value) {
		return kDefaultProbeTimeoutMs;
	}

	char *end = nullptr;
	const long parsed = std::strtol(value, &end, 10);
	if (end == value || *end != '\0') {
		return kDefaultProbeTimeoutMs;
	}

	return clampProbeTimeout(parsed);
}

std::string cacheDirectory()
{
#ifdef _WIN32
	const auto separator = vst3ConfigPath.find_last_of("/\\");
#else
	const auto separator = vst3ConfigPath.find_last_of('/');
#endif
	if (separator == std::string::npos) {
		return {};
	}

	return vst3ConfigPath.substr(0, separator + 1);
}

std::string probeResultPath(size_t sequence)
{
	return cacheDirectory() + "probe-" + std::to_string(currentProcessId()) + "-" + std::to_string(sequence) + ".json";
}

bool isPermissionDenied(const std::error_code &error)
{
	return error == std::errc::permission_denied ||
	       error.default_error_condition() == std::errc::permission_denied;
}

bool isProbeResultFileName(const std::string &name)
{
	if (name.compare(0, 6, "probe-") != 0) {
		return false;
	}

	const auto hasSuffix = [&name](const char *suffix) {
		const size_t suffixLength = std::strlen(suffix);
		return name.size() >= suffixLength && name.compare(name.size() - suffixLength, suffixLength, suffix) == 0;
	};

	return hasSuffix(".json") || hasSuffix(".json.tmp");
}

void removeStaleProbeResults()
{
	namespace fs = std::filesystem;

	const std::string directoryName = cacheDirectory();
	if (directoryName.empty()) {
		return;
	}

	const fs::path directory = fs::u8path(directoryName);
	std::error_code error;
	fs::directory_iterator entries(directory, fs::directory_options::skip_permission_denied, error);
	const fs::directory_iterator end;
	if (error) {
		if (!isPermissionDenied(error)) {
			blog(LOG_WARNING, "[VST3 Scanner] Cannot inspect probe results in %s: %s", directory.u8string().c_str(),
			     error.message().c_str());
		}
		return;
	}

	const auto cutoff = fs::file_time_type::clock::now() - std::chrono::hours(1);
	while (entries != end) {
		if (error) {
			if (!isPermissionDenied(error)) {
				blog(LOG_WARNING, "[VST3 Scanner] Probe result directory iteration stopped at %s: %s",
				     directory.u8string().c_str(), error.message().c_str());
			}
			error.clear();
			entries.increment(error);
			continue;
		}

		const fs::path path = entries->path();
		const std::string name = path.filename().u8string();
		if (isProbeResultFileName(name)) {
			std::error_code fileError;
			if (fs::is_regular_file(path, fileError) && !fileError) {
				const auto modified = fs::last_write_time(path, fileError);
				if (!fileError && modified < cutoff) {
					fs::remove(path, fileError);
					if (fileError) {
						blog(LOG_WARNING, "[VST3 Scanner] Cannot remove stale probe result %s: %s",
						     path.u8string().c_str(), fileError.message().c_str());
					}
				}
			}
		}

		entries.increment(error);
	}

	if (error && !isPermissionDenied(error)) {
		blog(LOG_WARNING, "[VST3 Scanner] Probe result directory iteration stopped at %s: %s",
		     directory.u8string().c_str(), error.message().c_str());
	}
}

void drainProbeOutput(os_process_pipe_t *pipe, bool errorOutput)
{
	uint8_t buffer[kProbeOutputBufferSize];
	for (;;) {
		const size_t length = errorOutput
					      ? os_process_pipe_read_err(pipe, buffer, sizeof(buffer))
					      : os_process_pipe_read(pipe, buffer, sizeof(buffer));
		if (!length) {
			return;
		}
	}
}

bool prepareProbeOutput()
{
#ifdef _WIN32
	auto redirectStream = [](DWORD standardHandle, int fileDescriptor) {
		const HANDLE originalHandle = GetStdHandle(standardHandle);
		/* _dup2 below closes the handle the CRT descriptor wraps, which is
		 * usually originalHandle; remember it so it is not closed twice. */
		const intptr_t descriptorHandle = _get_osfhandle(fileDescriptor);
		const HANDLE nullHandle = CreateFileW(L"NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
						      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (nullHandle == nullptr || nullHandle == INVALID_HANDLE_VALUE) {
			return false;
		}

		const int nullFileDescriptor = _open_osfhandle(reinterpret_cast<intptr_t>(nullHandle), _O_WRONLY | _O_BINARY);
		if (nullFileDescriptor == -1) {
			CloseHandle(nullHandle);
			return false;
		}

		if (_dup2(nullFileDescriptor, fileDescriptor) != 0) {
			_close(nullFileDescriptor);
			return false;
		}
		_close(nullFileDescriptor);

		const intptr_t redirectedValue = _get_osfhandle(fileDescriptor);
		if (redirectedValue == -1) {
			return false;
		}

		const HANDLE redirectedHandle = reinterpret_cast<HANDLE>(redirectedValue);
		if (!SetStdHandle(standardHandle, redirectedHandle)) {
			return false;
		}

		if (originalHandle != nullptr && originalHandle != INVALID_HANDLE_VALUE && originalHandle != redirectedHandle &&
		    reinterpret_cast<intptr_t>(originalHandle) != descriptorHandle) {
			CloseHandle(originalHandle);
		}
		return true;
	};

	/* MSVC has no STDOUT_FILENO/STDERR_FILENO. */
	const bool stdoutReady = redirectStream(STD_OUTPUT_HANDLE, _fileno(stdout));
	const bool stderrReady = redirectStream(STD_ERROR_HANDLE, _fileno(stderr));
	return stdoutReady && stderrReady;
#else
	const int nullFile = open("/dev/null", O_RDWR | O_CLOEXEC);
	if (nullFile < 0) {
		return false;
	}

	const bool stdoutReady = dup2(nullFile, STDOUT_FILENO) >= 0;
	const bool stderrReady = stdoutReady && dup2(nullFile, STDERR_FILENO) >= 0;
	if (nullFile > STDERR_FILENO) {
		close(nullFile);
	}
	return stdoutReady && stderrReady;
#endif
}

#ifdef _WIN32
bool clearCoordinatorHandleInheritance()
{
	const DWORD standardHandles[] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
	for (const DWORD standardHandle : standardHandles) {
		const HANDLE handle = GetStdHandle(standardHandle);
		if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
			continue;
		}

		if (!SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0)) {
			blog(LOG_ERROR, "[VST3 Scanner] Cannot clear inheritance on standard handle: %lu", GetLastError());
			return false;
		}
	}

	return true;
}
#endif

bool parseProcessId(const char *value, unsigned long long &processId)
{
	if (!value || !*value || *value == '-') {
		return false;
	}

	char *end = nullptr;
	errno = 0;
	const unsigned long long parsed = std::strtoull(value, &end, 10);
	if (errno == ERANGE || end == value || *end != '\0' || parsed == 0) {
		return false;
	}

	processId = parsed;
	return true;
}

bool startProbeWatchdog(int timeoutMs, unsigned long long parentProcessId)
{
	std::promise<bool> startPromise;
	std::future<bool> startResult = startPromise.get_future();
	std::thread watchdog([timeoutMs, parentProcessId, startPromise = std::move(startPromise)]() mutable {
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
#ifdef _WIN32
		if (parentProcessId > std::numeric_limits<DWORD>::max()) {
			startPromise.set_value(false);
			return;
		}

		HANDLE parentHandle = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(parentProcessId));
		if (!parentHandle) {
			startPromise.set_value(false);
			return;
		}
		if (WaitForSingleObject(parentHandle, 0) == WAIT_OBJECT_0) {
			CloseHandle(parentHandle);
			startPromise.set_value(false);
			return;
		}
#else
		if (static_cast<unsigned long long>(getppid()) != parentProcessId) {
			startPromise.set_value(false);
			return;
		}
#endif

		startPromise.set_value(true);

		for (;;) {
			if (std::chrono::steady_clock::now() >= deadline) {
				probeExit(kProbeWatchdogExitCode);
			}

#ifndef _WIN32
			if (static_cast<unsigned long long>(getppid()) != parentProcessId) {
				probeExit(kProbeWatchdogExitCode);
			}
#else
			if (WaitForSingleObject(parentHandle, 0) == WAIT_OBJECT_0) {
				probeExit(kProbeWatchdogExitCode);
			}
#endif

			std::this_thread::sleep_for(std::chrono::milliseconds(150));
		}
	});

	if (!startResult.get()) {
		watchdog.join();
		return false;
	}

	watchdog.detach();
	return true;
}

bool parseProbeTimeout(const char *value, int &timeoutMs)
{
	if (!value || !*value) {
		return false;
	}

	char *end = nullptr;
	const long parsed = std::strtol(value, &end, 10);
	if (end == value || *end != '\0' || parsed < 1) {
		return false;
	}

	timeoutMs = clampProbeTimeout(parsed);
	return true;
}

int runProbe(const char *bundlePath, const char *resultPath, const char *nonce, const char *timeoutValue,
	     const char *parentProcessIdValue)
{
	int timeoutMs = 0;
	if (!parseProbeTimeout(timeoutValue, timeoutMs)) {
		return EXIT_FAILURE;
	}

	unsigned long long parentProcessId = 0;
	if (!parseProcessId(parentProcessIdValue, parentProcessId)) {
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif
	if (!startProbeWatchdog(timeoutMs, parentProcessId)) {
		return EXIT_FAILURE;
	}

	if (!prepareProbeOutput()) {
		probeExit(EXIT_FAILURE);
	}

	VST3Scanner scanner;
	if (!scanner.loadModuleClassesInProcess(bundlePath)) {
		probeExit(EXIT_FAILURE);
	}

	obs_data_t *root = obs_data_create();
	obs_data_array_t *plugins = obs_data_array_create();
	for (const auto &classInfo : scanner.pluginList) {
		obs_data_t *obj = obs_data_create();
		saveVST3ClassInfo(obj, classInfo);
		obs_data_array_push_back(plugins, obj);
		obs_data_release(obj);
	}

	obs_data_set_int(root, "version", 1);
	obs_data_set_string(root, "nonce", nonce);
	obs_data_set_array(root, "plugins", plugins);
	obs_data_array_release(plugins);

	const bool saved = obs_data_save_json_safe(root, resultPath, "tmp", nullptr);
	obs_data_release(root);
	if (!saved) {
		probeExit(EXIT_FAILURE);
	}

	probeExit(EXIT_SUCCESS);
}

bool probeVst3Module(const std::string &bundlePath, std::vector<VST3ClassInfo> &classes, size_t sequence)
{
	const std::string resultPath = probeResultPath(sequence);
	const std::string nonce = "probe-" + std::to_string(currentProcessId()) + "-" + std::to_string(sequence) + "-" +
				  std::to_string(os_gettime_ns());
	auto reject = [&](const std::string &reason) {
		blog(LOG_WARNING, "[VST3 Scanner] Skipped %s: %s", bundlePath.c_str(), reason.c_str());
		os_unlink(resultPath.c_str());
		os_unlink((resultPath + ".tmp").c_str());
		return false;
	};

	const std::string temporaryResultPath = resultPath + ".tmp";
	if ((os_file_exists(resultPath.c_str()) && os_unlink(resultPath.c_str()) != 0) ||
	    (os_file_exists(temporaryResultPath.c_str()) && os_unlink(temporaryResultPath.c_str()) != 0)) {
		return reject("cannot remove stale probe result");
	}

	char *executable = os_get_executable_path_ptr(kVST3ScannerExecutable);
	if (!executable) {
		return reject("scanner executable path unavailable");
	}

	os_process_args_t *args = os_process_args_create(executable);
	bfree(executable);
	if (!args) {
		return reject("cannot create probe arguments");
	}

	const int timeoutMs = getProbeTimeoutMs();
	const std::string timeout = std::to_string(timeoutMs);
	const std::string parentProcessId = std::to_string(currentProcessId());
	os_process_args_add_arg(args, "--probe");
	os_process_args_add_arg(args, bundlePath.c_str());
	os_process_args_add_arg(args, resultPath.c_str());
	os_process_args_add_arg(args, nonce.c_str());
	os_process_args_add_arg(args, timeout.c_str());
	os_process_args_add_arg(args, parentProcessId.c_str());

	os_process_pipe_t *pipe = os_process_pipe_create2(args, "r");
	os_process_args_destroy(args);
	if (!pipe) {
		return reject("could not start probe");
	}

	const auto spawnTime = std::chrono::steady_clock::now();
	const auto margin = std::chrono::milliseconds(std::max(5000, timeoutMs / 4));
	const auto deadline = spawnTime + std::chrono::milliseconds(timeoutMs) + margin;
	struct ProbeTimeoutState {
		std::condition_variable condition;
		std::mutex mutex;
		bool killed = false;
		bool finished = false;
	};

	ProbeTimeoutState timeoutState;
	std::thread timeoutThread;
	std::thread errorReader;
	auto finishThreads = [&] {
		{
			std::lock_guard<std::mutex> lock(timeoutState.mutex);
			timeoutState.finished = true;
		}
		timeoutState.condition.notify_one();

		if (timeoutThread.joinable()) {
			timeoutThread.join();
		}
		if (errorReader.joinable()) {
			errorReader.join();
		}
	};
	auto destroyAfterThreadFailure = [&] {
		os_process_pipe_kill(pipe);
		finishThreads();
		os_process_pipe_destroy(pipe);
	};

	try {
		timeoutThread = std::thread([pipe, deadline, &timeoutState] {
			std::unique_lock<std::mutex> lock(timeoutState.mutex);
			if (timeoutState.condition.wait_until(lock, deadline, [&timeoutState] { return timeoutState.finished; })) {
				return;
			}

			timeoutState.killed = os_process_pipe_kill(pipe);
		});
		errorReader = std::thread([pipe] { drainProbeOutput(pipe, true); });
	} catch (...) {
		destroyAfterThreadFailure();
		return reject("could not start probe threads");
	}

	drainProbeOutput(pipe, false);
	errorReader.join();
	finishThreads();

	bool coordinatorKilled = false;
	{
		std::lock_guard<std::mutex> lock(timeoutState.mutex);
		coordinatorKilled = timeoutState.killed;
	}

	const auto now = std::chrono::steady_clock::now();
	uint32_t remainingMs = 0;
	if (now < deadline) {
		remainingMs = static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
	}

	if (!os_process_pipe_wait(pipe, remainingMs)) {
		coordinatorKilled = os_process_pipe_kill(pipe) || coordinatorKilled;
		os_process_pipe_wait(pipe, 5000);
	}

	const int status = os_process_pipe_destroy(pipe);
	if (coordinatorKilled) {
		return reject("probe timed out (killed)");
	}
	if (status != EXIT_SUCCESS) {
		return reject(status == kProbeWatchdogExitCode ? "probe timed out" : "probe failed");
	}

	if (!os_file_exists(resultPath.c_str())) {
		return reject("probe produced no result");
	}

	obs_data_t *root = obs_data_create_from_json_file(resultPath.c_str());
	if (!root) {
		return reject("invalid probe result");
	}

	const std::string resultNonce = obs_data_get_string(root, "nonce");
	if (resultNonce != nonce) {
		obs_data_release(root);
		return reject("probe nonce mismatch");
	}

	obs_data_array_t *jsonPluginList = obs_data_get_array(root, "plugins");
	if (!jsonPluginList) {
		obs_data_release(root);
		return reject("probe result has no class list");
	}

	std::vector<VST3ClassInfo> parsedClasses;
	std::string invalidReason;
	const size_t pluginCount = obs_data_array_count(jsonPluginList);
	for (size_t i = 0; i < pluginCount; ++i) {
		obs_data_t *obj = obs_data_array_item(jsonPluginList, i);
		VST3ClassInfo classInfo = loadVST3ClassInfo(obj);
		obs_data_release(obj);

		if (classInfo.id.empty()) {
			invalidReason = "probe result has an empty class id";
			break;
		}

		classInfo.path = bundlePath;
		classInfo.pluginName = std::filesystem::u8path(bundlePath).stem().u8string();
		parsedClasses.push_back(std::move(classInfo));
	}

	obs_data_array_release(jsonPluginList);
	obs_data_release(root);
	if (!invalidReason.empty()) {
		return reject(invalidReason);
	}

	os_unlink(resultPath.c_str());
	classes = std::move(parsedClasses);
	return true;
}

} // namespace

static bool vst3CacheSave()
{
	if (!list || vst3ConfigPath.empty()) {
		return false;
	}

	obs_data_t *root = obs_data_create();
	obs_data_array_t *arr = obs_data_array_create();

	for (const auto &classInfo : list->pluginList) {
		obs_data_t *obj = obs_data_create();
		saveVST3ClassInfo(obj, classInfo);
		obs_data_array_push_back(arr, obj);
		obs_data_release(obj);
	}

	obs_data_set_int(root, "version", 1);
	obs_data_set_array(root, "plugins", arr);
	obs_data_array_release(arr);

	bool saved = obs_data_save_json_safe(root, vst3ConfigPath.c_str(), "tmp", "bak");
	obs_data_release(root);

	return saved;
}

static bool vst3CacheLoad()
{
	if (vst3ConfigPath.empty()) {
		return false;
	}

	obs_data_t *root = obs_data_create_from_json_file_safe(vst3ConfigPath.c_str(), "bak");

	if (!root) {
		return false;
	}

	obs_data_array_t *jsonPluginList = obs_data_get_array(root, "plugins");
	if (!jsonPluginList) {
		obs_data_release(root);
		return false;
	}

	list->pluginList.clear();
	list->classCount.clear();

	std::unordered_map<std::string, ModuleCache> modules;

	const size_t jsonPluginNumber = obs_data_array_count(jsonPluginList);
	for (size_t i = 0; i < jsonPluginNumber; ++i) {
		obs_data_t *obj = obs_data_array_item(jsonPluginList, i);
		VST3ClassInfo classInfo = loadVST3ClassInfo(obj);
		obs_data_release(obj);

		if (classInfo.path.empty()) {
			continue;
		}

		std::error_code error;
		if (!std::filesystem::exists(std::filesystem::u8path(classInfo.path), error) || error) {
			continue;
		}

		auto &module = modules[classInfo.path];
		module.discardable = module.discardable || classInfo.discardable;
		module.classes.push_back(std::move(classInfo));
	}

	obs_data_array_release(jsonPluginList);
	obs_data_release(root);

	list->updateModulesList(modules);

	std::vector<std::string> modulePaths;
	modulePaths.reserve(modules.size());
	for (const auto &module : modules) {
		modulePaths.push_back(module.first);
	}
	std::sort(modulePaths.begin(), modulePaths.end());

	for (const auto &modulePath : modulePaths) {
		auto &module = modules[modulePath];
		// If a module has the flag kClassesDiscardable, the SDK compels us to load its binary.
		if (module.discardable) {
			list->addModuleClasses(modulePath);
		} else {
			for (auto &classInfo : module.classes) {
				list->pluginList.push_back(std::move(classInfo));
				++list->classCount[modulePath];
			}
		}
	}

	list->sort();

	// A parsed cache is usable even if all entries were stale or all probes failed.
	return true;
}

static bool scanVst3List()
{
	try {
		using clock = std::chrono::steady_clock;
		const auto start = clock::now();
		size_t probeNumber = 0;

		list->setModuleProbe([&probeNumber](const std::string &bundlePath, std::vector<VST3ClassInfo> &classes) {
			++probeNumber;
			return probeVst3Module(bundlePath, classes, probeNumber);
		});

		const bool loadedFromCache = vst3CacheLoad();
		if (!loadedFromCache && !list->scanForVST3Plugins()) {
			blog(LOG_WARNING, "[VST3 Scanner] No usable VST3 classes found");
		}

		size_t effectCount = 0;
		for (const auto &plugin : list->pluginList) {
			if (!plugin.isFX) {
				continue;
			}
			++effectCount;
		}
		const auto end = clock::now();
		const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		fprintf(stderr, "[VST3 Scanner] %s in %lld ms, found %zu plugins\n",
			loadedFromCache ? "Loaded cache & non-cacheable VST3s" : "Completed scan",
			static_cast<long long>(ms), effectCount);
		fflush(stderr);
		return vst3CacheSave();
	} catch (const std::exception &e) {
		blog(LOG_ERROR, "[VST3 Scanner] Scan aborted by an exception: %s\n", e.what());
		return false;
	} catch (...) {
		blog(LOG_ERROR, "[VST3 Scanner] Scan aborted by an unknown exception\n");
		return false;
	}
}

bool retrieveVst3List()
{
	list = new VST3Scanner();
	if (!list->hasVST3()) {
		blog(LOG_ERROR, "[VST3 Scanner] No VST3 were found\n");
		fprintf(stdout, "NO_VST3\n");
		fflush(stdout);
		return false;
	}

	return scanVst3List();
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv_w[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _WIN32
	char **argv;

	SetErrorMode(SEM_FAILCRITICALERRORS);
	SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	SetDllDirectoryW(L"");

	argv = static_cast<char **>(malloc(argc * sizeof(char *)));
	for (int i = 0; i < argc; i++) {
		size_t len = wcslen(argv_w[i]);
		int size;

		size = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], (int)len, NULL, 0, NULL, NULL);
		argv[i] = static_cast<char *>(malloc(size + 1));
		WideCharToMultiByte(CP_UTF8, 0, argv_w[i], (int)len, argv[i], size + 1, NULL, NULL);
		argv[i][size] = 0;
	}
#endif
	setvbuf(stderr, NULL, _IONBF, 0);

	if (argc >= 2 && std::strcmp(argv[1], "--probe") == 0) {
		if (argc != 7) {
			fprintf(stderr, "Usage: %s --probe <bundle-path> <result-file> <nonce> <timeout-ms> <parent-pid>\n",
				argv[0]);
			return EXIT_FAILURE;
		}
		return runProbe(argv[2], argv[3], argv[4], argv[5], argv[6]);
	}

#ifdef _WIN32
	if (!clearCoordinatorHandleInheritance()) {
		return EXIT_FAILURE;
	}
#endif

	if (argc < 2 || !argv[1]) {
		fprintf(stderr, "Usage: %s <vst3list.json>\n", argv[0]);
		return EXIT_FAILURE;
	}
	vst3ConfigPath = argv[1];

#ifdef _WIN32
	const auto separator = vst3ConfigPath.find_last_of("/\\");
#else
	const auto separator = vst3ConfigPath.find_last_of('/');
#endif
	if (separator != std::string::npos) {
		const auto directory = vst3ConfigPath.substr(0, separator + 1);

		if (os_mkdirs(directory.c_str()) == MKDIR_ERROR) {
			fprintf(stderr, "Cannot create cache directory: %s\n", directory.c_str());
			return EXIT_FAILURE;
		}
	}

	removeStaleProbeResults();

	if (!retrieveVst3List()) {
		fprintf(stderr, "Scan failed\n");
		delete list;
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	for (int i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
#endif
	fprintf(stdout, "VST3_SCAN_DONE\n");
	std::fflush(stdout);
	delete list;
	return EXIT_SUCCESS;
}
