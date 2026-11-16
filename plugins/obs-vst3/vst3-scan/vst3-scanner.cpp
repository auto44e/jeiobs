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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

VST3Scanner *list;
std::string vst3ConfigPath;

namespace {
constexpr int kDefaultProbeTimeoutMs = 10000;
constexpr int kMinimumProbeTimeoutMs = 500;
constexpr int kMaximumProbeTimeoutMs = 120000;
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
		return "./";
	}

	return vst3ConfigPath.substr(0, separator + 1);
}

std::string probeResultPath(size_t sequence)
{
	return cacheDirectory() + "probe-" + std::to_string(currentProcessId()) + "-" + std::to_string(sequence) + ".json";
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
	const HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	const HANDLE stderrHandle = GetStdHandle(STD_ERROR_HANDLE);
	const bool stdoutReady = stdoutHandle == nullptr || stdoutHandle == INVALID_HANDLE_VALUE ||
				 !!SetHandleInformation(stdoutHandle, HANDLE_FLAG_INHERIT, 0);
	const bool stderrReady = stderrHandle == nullptr || stderrHandle == INVALID_HANDLE_VALUE ||
				 !!SetHandleInformation(stderrHandle, HANDLE_FLAG_INHERIT, 0);
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

void startProbeWatchdog(int timeoutMs)
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
#ifndef _WIN32
	const pid_t parentProcessId = getppid();
#endif

	std::thread([deadline
#ifndef _WIN32
		     ,
		     parentProcessId
#endif
		     ] {
		for (;;) {
			if (std::chrono::steady_clock::now() >= deadline) {
				probeExit(3);
			}

#ifndef _WIN32
			if (getppid() != parentProcessId) {
				probeExit(3);
			}
#endif

			std::this_thread::sleep_for(std::chrono::milliseconds(150));
		}
	}).detach();
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

int runProbe(const char *bundlePath, const char *resultPath, const char *nonce, const char *timeoutValue)
{
	int timeoutMs = 0;
	if (!parseProbeTimeout(timeoutValue, timeoutMs)) {
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif
	startProbeWatchdog(timeoutMs);

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

	const std::string timeout = std::to_string(getProbeTimeoutMs());
	os_process_args_add_arg(args, "--probe");
	os_process_args_add_arg(args, bundlePath.c_str());
	os_process_args_add_arg(args, resultPath.c_str());
	os_process_args_add_arg(args, nonce.c_str());
	os_process_args_add_arg(args, timeout.c_str());

	os_process_pipe_t *pipe = os_process_pipe_create2(args, "r");
	os_process_args_destroy(args);
	if (!pipe) {
		return reject("could not start probe");
	}

	std::thread errorReader([pipe] { drainProbeOutput(pipe, true); });
	drainProbeOutput(pipe, false);
	errorReader.join();

	const int status = os_process_pipe_destroy(pipe);
	if (status != EXIT_SUCCESS) {
		return reject(status == 3 ? "probe timed out" : "probe failed");
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

	(void)scanVst3List();
	return true;
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
		if (argc != 6) {
			fprintf(stderr, "Usage: %s --probe <bundle-path> <result-file> <nonce> <timeout-ms>\n", argv[0]);
			return EXIT_FAILURE;
		}
		return runProbe(argv[2], argv[3], argv[4], argv[5]);
	}

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

	if (!retrieveVst3List()) {
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
