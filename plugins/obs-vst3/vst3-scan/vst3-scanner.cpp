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

#include <obs.h>
#include <obs-module.h>
#include <util/crc32.h>
#include <util/pipe.h>
#include <util/platform.h>

#include <algorithm>
#include <atomic>
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
#include <sstream>
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
#elif defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#else
#include <fcntl.h>
#include <sys/file.h>
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
constexpr size_t kFingerprintReadSize = 64 * 1024;
constexpr int64_t kMissingEntryRetentionSeconds = 30 * 24 * 60 * 60;
constexpr int64_t kTimeoutBackoffSeconds = 7 * 24 * 60 * 60;

#ifdef _WIN32
constexpr char kVST3ScannerExecutable[] = "obs-vst3-scanner.exe";
#else
constexpr char kVST3ScannerExecutable[] = "obs-vst3-scanner";
#endif

struct BinaryFingerprint {
	std::string binary;
	int64_t size = 0;
	int64_t mtimeTicks = 0;
	uint32_t crc32 = 0;
};

struct SkipEntry {
	std::string path;
	std::string binary;
	int64_t size = 0;
	int64_t mtimeTicks = 0;
	uint32_t crc32 = 0;
	std::string obsVersion;
	std::string kind;
	std::string reason;
	int64_t failures = 0;
	int64_t firstFailed = 0;
	int64_t lastFailed = 0;
	int64_t retryAfter = 0;
	int64_t missingSince = 0;
};

struct SkipListState {
	std::string path;
	std::unordered_map<std::string, SkipEntry> entries;
	bool enabled = false;
};

enum class ProbeMarker {
	Invalid,
	Setup,
	LoadFailed,
	SaveFailed,
	Timeout,
};

struct ProbeStatus {
	ProbeMarker marker = ProbeMarker::Invalid;
	std::string nonce;
};

SkipListState skipList;
size_t skippedBundleCount = 0;
bool retryFailed = false;
bool scanLockHeld = false;

std::string cacheDirectory();

int64_t currentTimeSeconds()
{
	return std::chrono::duration_cast<std::chrono::seconds>(
		       std::chrono::system_clock::now().time_since_epoch())
		.count();
}

std::string skipListPath()
{
	return cacheDirectory() + "vst3skip.json";
}

std::string probeStatusPath(const std::string &resultPath)
{
	return resultPath + ".status";
}

std::string probeStatusTemporaryPath(const std::string &resultPath, bool watchdog)
{
	return resultPath + (watchdog ? ".status.watchdog.tmp" : ".status.tmp");
}

const char *probeMarkerName(ProbeMarker marker)
{
	switch (marker) {
	case ProbeMarker::Setup:
		return "setup";
	case ProbeMarker::LoadFailed:
		return "load-failed";
	case ProbeMarker::SaveFailed:
		return "save-failed";
	case ProbeMarker::Timeout:
		return "timeout";
	case ProbeMarker::Invalid:
	default:
		return "";
	}
}

ProbeMarker parseProbeMarker(const std::string &marker)
{
	if (marker == "setup") {
		return ProbeMarker::Setup;
	}
	if (marker == "load-failed") {
		return ProbeMarker::LoadFailed;
	}
	if (marker == "save-failed") {
		return ProbeMarker::SaveFailed;
	}
	if (marker == "timeout") {
		return ProbeMarker::Timeout;
	}
	return ProbeMarker::Invalid;
}

bool writeProbeStatus(const std::string &resultPath, ProbeMarker marker, const std::string &nonce, bool watchdog)
{
	const char *markerName = probeMarkerName(marker);
	if (!*markerName) {
		return false;
	}

	const std::string statusPath = probeStatusPath(resultPath);
	const std::string temporaryPath = probeStatusTemporaryPath(resultPath, watchdog);
	const std::string status = std::string(markerName) + "\n" + nonce + "\n";
	if (!os_quick_write_utf8_file(temporaryPath.c_str(), status.c_str(), status.size(), false)) {
		return false;
	}

	if (os_rename(temporaryPath.c_str(), statusPath.c_str()) != 0) {
		os_unlink(temporaryPath.c_str());
		return false;
	}

	return true;
}

ProbeStatus readProbeStatus(const std::string &resultPath)
{
	ProbeStatus status;
	char *contents = os_quick_read_utf8_file(probeStatusPath(resultPath).c_str());
	if (!contents) {
		return status;
	}

	std::istringstream stream(contents);
	std::string marker;
	stream >> marker >> status.nonce;
	bfree(contents);
	status.marker = parseProbeMarker(marker);
	if (status.marker == ProbeMarker::Invalid || status.nonce.empty()) {
		status.marker = ProbeMarker::Invalid;
		status.nonce.clear();
	}
	return status;
}

void removeProbeFiles(const std::string &resultPath)
{
	os_unlink(resultPath.c_str());
	os_unlink((resultPath + ".tmp").c_str());
	os_unlink(probeStatusPath(resultPath).c_str());
	os_unlink(probeStatusTemporaryPath(resultPath, false).c_str());
	os_unlink(probeStatusTemporaryPath(resultPath, true).c_str());
}

bool sameFingerprint(const SkipEntry &entry, const BinaryFingerprint &fingerprint)
{
	return entry.binary == fingerprint.binary && entry.size == fingerprint.size &&
	       entry.mtimeTicks == fingerprint.mtimeTicks && entry.crc32 == fingerprint.crc32;
}

bool skipEntryHasRequiredFields(obs_data_t *obj)
{
	const char *path = obs_data_get_string(obj, "path");
	const char *binary = obs_data_get_string(obj, "binary");
	const char *obsVersion = obs_data_get_string(obj, "obs_version");
	const char *kind = obs_data_get_string(obj, "kind");
	const char *reason = obs_data_get_string(obj, "reason");
	if (!path || !*path || !binary || !*binary || !obsVersion || !*obsVersion || !kind || !*kind || !reason ||
	    !*reason) {
		return false;
	}

	if (!obs_data_has_user_value(obj, "size") || !obs_data_has_user_value(obj, "mtime_ticks") ||
	    !obs_data_has_user_value(obj, "crc32") || !obs_data_has_user_value(obj, "failures") ||
	    !obs_data_has_user_value(obj, "first_failed") || !obs_data_has_user_value(obj, "last_failed") ||
	    !obs_data_has_user_value(obj, "retry_after") || !obs_data_has_user_value(obj, "missing_since")) {
		return false;
	}

	if (std::strcmp(kind, "failure") != 0 && std::strcmp(kind, "timeout") != 0) {
		return false;
	}

	const long long size = obs_data_get_int(obj, "size");
	const long long crc32 = obs_data_get_int(obj, "crc32");
	const long long failures = obs_data_get_int(obj, "failures");
	const long long firstFailed = obs_data_get_int(obj, "first_failed");
	const long long lastFailed = obs_data_get_int(obj, "last_failed");
	const long long retryAfter = obs_data_get_int(obj, "retry_after");
	const long long missingSince = obs_data_get_int(obj, "missing_since");
	return size >= 0 && crc32 >= 0 && crc32 <= std::numeric_limits<uint32_t>::max() && failures > 0 &&
	       firstFailed > 0 && lastFailed >= firstFailed && retryAfter >= 0 && missingSince >= 0;
}

bool saveSkipList()
{
	if (!skipList.enabled) {
		return false;
	}

	obs_data_t *root = obs_data_create();
	obs_data_array_t *bundles = obs_data_array_create();
	if (!root || !bundles) {
		if (bundles) {
			obs_data_array_release(bundles);
		}
		if (root) {
			obs_data_release(root);
		}
		return false;
	}

	std::vector<std::string> paths;
	paths.reserve(skipList.entries.size());
	for (const auto &entry : skipList.entries) {
		paths.push_back(entry.first);
	}
	std::sort(paths.begin(), paths.end());

	for (const auto &path : paths) {
		const SkipEntry &entry = skipList.entries.at(path);
		obs_data_t *obj = obs_data_create();
		obs_data_set_string(obj, "path", entry.path.c_str());
		obs_data_set_string(obj, "binary", entry.binary.c_str());
		obs_data_set_int(obj, "size", entry.size);
		obs_data_set_int(obj, "mtime_ticks", entry.mtimeTicks);
		obs_data_set_int(obj, "crc32", entry.crc32);
		obs_data_set_string(obj, "obs_version", entry.obsVersion.c_str());
		obs_data_set_string(obj, "kind", entry.kind.c_str());
		obs_data_set_string(obj, "reason", entry.reason.c_str());
		obs_data_set_int(obj, "failures", entry.failures);
		obs_data_set_int(obj, "first_failed", entry.firstFailed);
		obs_data_set_int(obj, "last_failed", entry.lastFailed);
		obs_data_set_int(obj, "retry_after", entry.retryAfter);
		obs_data_set_int(obj, "missing_since", entry.missingSince);
		obs_data_array_push_back(bundles, obj);
		obs_data_release(obj);
	}

	obs_data_set_int(root, "version", 1);
	obs_data_set_array(root, "bundles", bundles);
	obs_data_array_release(bundles);
	const bool saved = obs_data_save_json_safe(root, skipList.path.c_str(), "tmp", "bak");
	obs_data_release(root);
	if (!saved) {
		blog(LOG_WARNING, "[VST3 Scanner] Cannot save skip list: %s", skipList.path.c_str());
	}
	return saved;
}

void loadSkipList()
{
	skipList.path = skipListPath();
	skipList.entries.clear();
	if (!scanLockHeld) {
		skipList.enabled = false;
		return;
	}

	skipList.enabled = true;
	obs_data_t *root = obs_data_create_from_json_file_safe(skipList.path.c_str(), "bak");
	if (!root || obs_data_get_int(root, "version") != 1) {
		if (root) {
			obs_data_release(root);
		}
		return;
	}

	obs_data_array_t *bundles = obs_data_get_array(root, "bundles");
	if (!bundles) {
		obs_data_release(root);
		return;
	}

	const size_t count = obs_data_array_count(bundles);
	for (size_t i = 0; i < count; ++i) {
		obs_data_t *obj = obs_data_array_item(bundles, i);
		if (skipEntryHasRequiredFields(obj)) {
			SkipEntry entry;
			entry.path = obs_data_get_string(obj, "path");
			entry.binary = obs_data_get_string(obj, "binary");
			entry.size = obs_data_get_int(obj, "size");
			entry.mtimeTicks = obs_data_get_int(obj, "mtime_ticks");
			entry.crc32 = static_cast<uint32_t>(obs_data_get_int(obj, "crc32"));
			entry.obsVersion = obs_data_get_string(obj, "obs_version");
			entry.kind = obs_data_get_string(obj, "kind");
			entry.reason = obs_data_get_string(obj, "reason");
			entry.failures = obs_data_get_int(obj, "failures");
			entry.firstFailed = obs_data_get_int(obj, "first_failed");
			entry.lastFailed = obs_data_get_int(obj, "last_failed");
			entry.retryAfter = obs_data_get_int(obj, "retry_after");
			entry.missingSince = obs_data_get_int(obj, "missing_since");
			skipList.entries[entry.path] = std::move(entry);
		}
		obs_data_release(obj);
	}

	obs_data_array_release(bundles);
	obs_data_release(root);
}

void pruneMissingSkipEntries()
{
	if (!skipList.enabled) {
		return;
	}

	const int64_t now = currentTimeSeconds();
	bool changed = false;
	for (auto it = skipList.entries.begin(); it != skipList.entries.end();) {
		std::error_code error;
		const bool exists = std::filesystem::exists(std::filesystem::u8path(it->first), error);
		if (error || exists) {
			if (it->second.missingSince != 0) {
				it->second.missingSince = 0;
				changed = true;
			}
			++it;
			continue;
		}

		if (it->second.missingSince == 0) {
			it->second.missingSince = now;
			changed = true;
			++it;
			continue;
		}

		if (now - it->second.missingSince >= kMissingEntryRetentionSeconds) {
			it = skipList.entries.erase(it);
			changed = true;
		} else {
			++it;
		}
	}

	if (changed) {
		saveSkipList();
	}
}

void removeSkipEntry(const std::string &bundlePath)
{
	if (!skipList.enabled) {
		return;
	}

	if (skipList.entries.erase(bundlePath) != 0) {
		saveSkipList();
	}
}

void rememberProbeFailure(const std::string &bundlePath, const BinaryFingerprint &fingerprint, const char *kind,
			  const char *reason)
{
	if (!skipList.enabled) {
		return;
	}

	const int64_t now = currentTimeSeconds();
	SkipEntry entry;
	auto existing = skipList.entries.find(bundlePath);
	if (existing != skipList.entries.end() && sameFingerprint(existing->second, fingerprint)) {
		entry = existing->second;
	} else {
		entry.path = bundlePath;
		entry.binary = fingerprint.binary;
		entry.size = fingerprint.size;
		entry.mtimeTicks = fingerprint.mtimeTicks;
		entry.crc32 = fingerprint.crc32;
		entry.failures = 0;
		entry.firstFailed = now;
	}

	entry.path = bundlePath;
	entry.binary = fingerprint.binary;
	entry.size = fingerprint.size;
	entry.mtimeTicks = fingerprint.mtimeTicks;
	entry.crc32 = fingerprint.crc32;
	entry.obsVersion = obs_get_version_string() ? obs_get_version_string() : "";
	entry.kind = kind;
	entry.reason = reason;
	entry.failures++;
	entry.lastFailed = now;
	entry.missingSince = 0;
	entry.retryAfter = std::strcmp(kind, "timeout") == 0 && entry.failures >= 2
				   ? now + kTimeoutBackoffSeconds
				   : 0;
	skipList.entries[bundlePath] = std::move(entry);
	saveSkipList();
}

void countSkippedBundle(const std::string &bundlePath, const std::string &reason)
{
	++skippedBundleCount;
	blog(LOG_WARNING, "[VST3 Scanner] Skipped %s: %s", bundlePath.c_str(), reason.c_str());
}

void countRememberedBundle(const std::string &bundlePath, const SkipEntry &entry)
{
	++skippedBundleCount;
	blog(LOG_WARNING, "[VST3 Scanner] Skipped %s: remembered %s (Rescan to retry)", bundlePath.c_str(),
	     entry.reason.c_str());
}

bool shouldSkipBundle(const std::string &bundlePath, const BinaryFingerprint &fingerprint)
{
	if (!skipList.enabled || retryFailed) {
		return false;
	}

	const auto it = skipList.entries.find(bundlePath);
	if (it == skipList.entries.end() || !sameFingerprint(it->second, fingerprint)) {
		return false;
	}

	const SkipEntry &entry = it->second;
	const std::string currentVersion = obs_get_version_string() ? obs_get_version_string() : "";
	if (entry.obsVersion != currentVersion) {
		return false;
	}

	if (entry.kind == "failure") {
		countRememberedBundle(bundlePath, entry);
		return true;
	}

	if (entry.kind == "timeout" && entry.retryAfter > currentTimeSeconds()) {
		countRememberedBundle(bundlePath, entry);
		return true;
	}

	return false;
}

bool resolveVst3Binary(const std::string &bundlePath, std::string &binaryPath)
{
	namespace fs = std::filesystem;
	const fs::path bundle = fs::u8path(bundlePath);

#ifdef _WIN32
	binaryPath = bundle.u8string();
	return true;
#elif defined(__APPLE__)
	CFURLRef bundleUrl = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
								      reinterpret_cast<const UInt8 *>(bundlePath.c_str()),
								      bundlePath.size(), true);
	if (!bundleUrl) {
		return false;
	}

	CFBundleRef bundleRef = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
	CFRelease(bundleUrl);
	if (!bundleRef) {
		return false;
	}

	CFURLRef executableUrl = CFBundleCopyExecutableURL(bundleRef);
	CFRelease(bundleRef);
	if (!executableUrl) {
		return false;
	}

	CFStringRef executablePath = CFURLCopyFileSystemPath(executableUrl, kCFURLPOSIXPathStyle);
	CFRelease(executableUrl);
	if (!executablePath) {
		return false;
	}

	const CFIndex length = CFStringGetMaximumSizeOfFileSystemRepresentation(executablePath);
	std::vector<char> pathBuffer(static_cast<size_t>(length) + 1);
	const bool converted = CFStringGetFileSystemRepresentation(executablePath, pathBuffer.data(), pathBuffer.size());
	CFRelease(executablePath);
	if (!converted) {
		return false;
	}

	binaryPath = pathBuffer.data();
	return true;
#elif defined(__linux__)
#if defined(__x86_64__)
	constexpr char architecture[] = "x86_64-linux";
#elif defined(__aarch64__)
	constexpr char architecture[] = "arm64-linux";
#elif defined(__i386__)
	constexpr char architecture[] = "x86-linux";
#else
	return false;
#endif
	binaryPath = (bundle / "Contents" / architecture / (bundle.stem().u8string() + ".so")).u8string();
	return true;
#else
	UNUSED_PARAMETER(bundlePath);
	UNUSED_PARAMETER(binaryPath);
	return false;
#endif
}

bool readBinaryStat(const std::string &path, int64_t &size, int64_t &mtimeTicks)
{
	namespace fs = std::filesystem;
	std::error_code error;
	const fs::path binary = fs::u8path(path);
	if (!fs::is_regular_file(binary, error) || error) {
		return false;
	}

	const auto fileSize = fs::file_size(binary, error);
	if (error) {
		return false;
	}
	const auto modified = fs::last_write_time(binary, error);
	if (error) {
		return false;
	}

	size = static_cast<int64_t>(fileSize);
	mtimeTicks = static_cast<int64_t>(modified.time_since_epoch().count());
	return true;
}

bool calculateBinaryCrc32(const std::string &path, uint32_t &crc)
{
	FILE *file = os_fopen(path.c_str(), "rb");
	if (!file) {
		return false;
	}

	std::vector<uint8_t> buffer(kFingerprintReadSize);
	uint32_t value = 0;
	bool success = true;
	for (;;) {
		const size_t bytes = std::fread(buffer.data(), 1, buffer.size(), file);
		if (bytes != 0) {
			value = calc_crc32(value, buffer.data(), bytes);
		}
		if (bytes < buffer.size()) {
			if (std::ferror(file)) {
				success = false;
			}
			break;
		}
	}
	std::fclose(file);
	if (success) {
		crc = value;
	}
	return success;
}

bool getBinaryFingerprint(const std::string &bundlePath, BinaryFingerprint &fingerprint)
{
	std::string binaryPath;
	if (!resolveVst3Binary(bundlePath, binaryPath)) {
		return false;
	}

	int64_t firstSize = 0;
	int64_t firstMtimeTicks = 0;
	if (!readBinaryStat(binaryPath, firstSize, firstMtimeTicks)) {
		return false;
	}

	uint32_t crc = 0;
	if (!calculateBinaryCrc32(binaryPath, crc)) {
		return false;
	}

	int64_t secondSize = 0;
	int64_t secondMtimeTicks = 0;
	if (!readBinaryStat(binaryPath, secondSize, secondMtimeTicks) || firstSize != secondSize ||
	    firstMtimeTicks != secondMtimeTicks) {
		return false;
	}

	fingerprint.binary = std::move(binaryPath);
	fingerprint.size = secondSize;
	fingerprint.mtimeTicks = secondMtimeTicks;
	fingerprint.crc32 = crc;
	return true;
}

class ScanLock {
public:
	explicit ScanLock(const std::string &path)
	{
#ifdef _WIN32
		wchar_t *widePath = nullptr;
		if (!os_utf8_to_wcs_ptr(path.c_str(), 0, &widePath)) {
			return;
		}

		handle_ = CreateFileW(widePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
				      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		bfree(widePath);
		if (handle_ == INVALID_HANDLE_VALUE) {
			handle_ = nullptr;
			return;
		}

		OVERLAPPED overlapped = {};
		if (LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
			locked_ = true;
			return;
		}
		CloseHandle(handle_);
		handle_ = nullptr;
#else
		fd_ = open(path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0600);
		if (fd_ < 0) {
			return;
		}

		for (;;) {
			if (flock(fd_, LOCK_EX) == 0) {
				locked_ = true;
				return;
			}
			if (errno == EINTR) {
				continue;
			}
			break;
		}
		close(fd_);
		fd_ = -1;
#endif
	}

	~ScanLock()
	{
#ifdef _WIN32
		if (handle_) {
			OVERLAPPED overlapped = {};
			UnlockFileEx(handle_, 0, MAXDWORD, MAXDWORD, &overlapped);
			CloseHandle(handle_);
		}
#else
		if (fd_ >= 0) {
			flock(fd_, LOCK_UN);
			close(fd_);
		}
#endif
	}

	bool locked() const
	{
		return locked_;
	}

private:
	bool locked_ = false;
#ifdef _WIN32
	HANDLE handle_ = nullptr;
#else
	int fd_ = -1;
#endif
};

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

	return hasSuffix(".json") || hasSuffix(".json.tmp") || hasSuffix(".json.status") ||
	       hasSuffix(".json.status.tmp") || hasSuffix(".json.status.watchdog.tmp");
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

bool startProbeWatchdog(int timeoutMs, unsigned long long parentProcessId, const std::string &resultPath,
			const std::string &nonce, std::atomic<bool> &setupWritten)
{
	std::promise<bool> startPromise;
	std::future<bool> startResult = startPromise.get_future();
	std::thread watchdog([timeoutMs, parentProcessId, resultPath, nonce, &setupWritten,
			      startPromise = std::move(startPromise)]() mutable {
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
				if (setupWritten.load(std::memory_order_acquire)) {
					writeProbeStatus(resultPath, ProbeMarker::Timeout, nonce, true);
				}
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
	std::atomic<bool> setupWritten = false;
	if (!startProbeWatchdog(timeoutMs, parentProcessId, resultPath, nonce, setupWritten)) {
		return EXIT_FAILURE;
	}

	if (!prepareProbeOutput()) {
		probeExit(EXIT_FAILURE);
	}

	VST3Scanner scanner;
	setupWritten.store(writeProbeStatus(resultPath, ProbeMarker::Setup, nonce, false), std::memory_order_release);
	if (!scanner.loadModuleClassesInProcess(bundlePath)) {
		if (setupWritten.load(std::memory_order_acquire)) {
			writeProbeStatus(resultPath, ProbeMarker::LoadFailed, nonce, false);
		}
		probeExit(EXIT_FAILURE);
	}

	obs_data_t *root = obs_data_create();
	obs_data_array_t *plugins = obs_data_array_create();
	if (!root || !plugins) {
		if (plugins) {
			obs_data_array_release(plugins);
		}
		if (root) {
			obs_data_release(root);
		}
		if (setupWritten.load(std::memory_order_acquire)) {
			writeProbeStatus(resultPath, ProbeMarker::SaveFailed, nonce, false);
		}
		probeExit(EXIT_FAILURE);
	}
	for (const auto &classInfo : scanner.pluginList) {
		obs_data_t *obj = obs_data_create();
		if (!obj) {
			obs_data_array_release(plugins);
			obs_data_release(root);
			if (setupWritten.load(std::memory_order_acquire)) {
				writeProbeStatus(resultPath, ProbeMarker::SaveFailed, nonce, false);
			}
			probeExit(EXIT_FAILURE);
		}
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
		if (setupWritten.load(std::memory_order_acquire)) {
			writeProbeStatus(resultPath, ProbeMarker::SaveFailed, nonce, false);
		}
		probeExit(EXIT_FAILURE);
	}

	probeExit(EXIT_SUCCESS);
}

bool probeVst3Module(const std::string &bundlePath, std::vector<VST3ClassInfo> &classes, size_t sequence)
{
	const std::string resultPath = probeResultPath(sequence);
	const std::string nonce = "probe-" + std::to_string(currentProcessId()) + "-" + std::to_string(sequence) + "-" +
				  std::to_string(os_gettime_ns());
	removeProbeFiles(resultPath);

	BinaryFingerprint fingerprint;
	const bool hasFingerprint = getBinaryFingerprint(bundlePath, fingerprint);
	if (hasFingerprint && shouldSkipBundle(bundlePath, fingerprint)) {
		return false;
	}

	auto reject = [&](const std::string &reason) {
		countSkippedBundle(bundlePath, reason);
		removeProbeFiles(resultPath);
		return false;
	};

	auto rememberFailure = [&](const char *kind, const char *reason, const char *logReason) {
		if (hasFingerprint) {
			rememberProbeFailure(bundlePath, fingerprint, kind, reason);
		}
		return reject(logReason);
	};

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
	const ProbeStatus probeStatus = readProbeStatus(resultPath);
	const bool validStatus = probeStatus.marker != ProbeMarker::Invalid && probeStatus.nonce == nonce;
	const bool resultExists = os_file_exists(resultPath.c_str());
	std::vector<VST3ClassInfo> parsedClasses;
	bool validResult = false;
	bool resultMalformed = false;
	bool resultNonceMismatch = false;
	if (resultExists) {
		obs_data_t *root = obs_data_create_from_json_file(resultPath.c_str());
		if (!root) {
			resultMalformed = true;
		} else {
			if (obs_data_get_int(root, "version") != 1) {
				resultMalformed = true;
			} else {
				const std::string resultNonce = obs_data_get_string(root, "nonce");
				if (resultNonce != nonce) {
					resultNonceMismatch = true;
				} else {
					obs_data_array_t *jsonPluginList = obs_data_get_array(root, "plugins");
					if (!jsonPluginList) {
						resultMalformed = true;
					} else {
						const size_t pluginCount = obs_data_array_count(jsonPluginList);
						for (size_t i = 0; i < pluginCount; ++i) {
							obs_data_t *obj = obs_data_array_item(jsonPluginList, i);
							VST3ClassInfo classInfo = loadVST3ClassInfo(obj);
							obs_data_release(obj);

							if (classInfo.id.empty()) {
								resultMalformed = true;
								break;
							}

							classInfo.path = bundlePath;
							classInfo.pluginName = std::filesystem::u8path(bundlePath).stem().u8string();
							parsedClasses.push_back(std::move(classInfo));
						}
						obs_data_array_release(jsonPluginList);
						validResult = !resultMalformed;
					}
				}
			}
		}
		if (root) {
			obs_data_release(root);
		}
	}

	if (!validStatus) {
		if (resultNonceMismatch) {
			return reject("probe nonce mismatch");
		}
		if (resultMalformed) {
			return reject("invalid probe result");
		}
		return reject("probe produced no valid status");
	}

	if (probeStatus.marker == ProbeMarker::SaveFailed) {
		return reject("probe save failed");
	}
	if (resultNonceMismatch || resultMalformed) {
		return reject(resultNonceMismatch ? "probe nonce mismatch" : "invalid probe result");
	}
	if (validResult) {
		removeSkipEntry(bundlePath);
		removeProbeFiles(resultPath);
		classes = std::move(parsedClasses);
		return true;
	}

	if (probeStatus.marker == ProbeMarker::Timeout) {
		return rememberFailure("timeout", "timeout",
				      coordinatorKilled ? "probe timed out (killed)" : "probe timed out");
	}
	if (coordinatorKilled && probeStatus.marker == ProbeMarker::Setup) {
		return rememberFailure("timeout", "timeout", "probe timed out (killed)");
	}
	if (probeStatus.marker == ProbeMarker::LoadFailed) {
		return rememberFailure("failure", "load-failed", "probe failed");
	}
	if (probeStatus.marker == ProbeMarker::Setup && !resultExists) {
		return rememberFailure("failure", "probe failed", "probe failed");
	}

	if (status == kProbeWatchdogExitCode) {
		return reject("probe timed out");
	}
	if (status != EXIT_SUCCESS) {
		return reject("probe failed");
	}
	return reject("probe produced no result");
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

	bool saved = false;
	if (scanLockHeld) {
		saved = obs_data_save_json_safe(root, vst3ConfigPath.c_str(), "tmp", "bak");
	} else {
		const char *json = obs_data_get_json(root);
		const std::string temporaryPath = cacheDirectory() + "tmp-" + std::to_string(currentProcessId());
		if (json && os_quick_write_utf8_file(temporaryPath.c_str(), json, std::strlen(json), false) &&
		    os_rename(temporaryPath.c_str(), vst3ConfigPath.c_str()) == 0) {
			saved = true;
		} else {
			os_unlink(temporaryPath.c_str());
		}
	}
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
		loadSkipList();
		pruneMissingSkipEntries();

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
	skippedBundleCount = 0;
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
		fprintf(stderr, "Usage: %s <vst3list.json> [--retry-failed]\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc > 3 || (argc == 3 && std::strcmp(argv[2], "--retry-failed") != 0)) {
		fprintf(stderr, "Usage: %s <vst3list.json> [--retry-failed]\n", argv[0]);
		return EXIT_FAILURE;
	}
	vst3ConfigPath = argv[1];
	retryFailed = argc == 3;

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

	ScanLock scanLock(cacheDirectory() + "vst3scan.lock");
	scanLockHeld = scanLock.locked();
	if (!scanLockHeld) {
		blog(LOG_WARNING, "[VST3 Scanner] Scan lock unavailable; failed-bundle memory disabled for this scan");
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
	fprintf(stdout, "VST3_SKIPPED %zu\n", skippedBundleCount);
	fprintf(stdout, "VST3_SCAN_DONE\n");
	std::fflush(stdout);
	delete list;
	return EXIT_SUCCESS;
}
