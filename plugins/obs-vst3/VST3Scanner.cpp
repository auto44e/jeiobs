/******************************************************************************
    Copyright (C) 2025-2026 pkv <pkv@obsproject.com>
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

#include "VST3Scanner.h"

#include <pluginterfaces/base/ipluginbase.h>
#include <public.sdk/source/vst/hosting/module.h>
#include <public.sdk/source/vst/moduleinfo/moduleinfo.h>
#include <public.sdk/source/vst/moduleinfo/moduleinfoparser.h>

#include <util/base.h>
#include <util/bmem.h>
#include <util/platform.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <system_error>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

void saveVST3ClassInfo(obs_data_t *obj, const VST3ClassInfo &classInfo)
{
	if (!obj) {
		return;
	}

	obs_data_set_string(obj, "name", classInfo.name.c_str());
	obs_data_set_string(obj, "id", classInfo.id.c_str());
	obs_data_set_string(obj, "path", classInfo.path.c_str());
	obs_data_set_string(obj, "pluginName", classInfo.pluginName.c_str());
	obs_data_set_bool(obj, "discardable", classInfo.discardable);
	obs_data_set_bool(obj, "isFX", classInfo.isFX);
}

VST3ClassInfo loadVST3ClassInfo(obs_data_t *obj)
{
	VST3ClassInfo classInfo;
	if (!obj) {
		return classInfo;
	}

	classInfo.name = obs_data_get_string(obj, "name");
	classInfo.id = obs_data_get_string(obj, "id");
	classInfo.path = obs_data_get_string(obj, "path");
	classInfo.pluginName = obs_data_get_string(obj, "pluginName");
	classInfo.discardable = obs_data_get_bool(obj, "discardable");
	classInfo.isFX = obs_data_get_bool(obj, "isFX");
	return classInfo;
}

namespace {
namespace fs = std::filesystem;

bool isPermissionDenied(const std::error_code &error)
{
	return error == std::errc::permission_denied ||
	       error.default_error_condition() == std::errc::permission_denied;
}

#ifdef _WIN32
bool isWindowsVST3Architecture(const fs::path &path)
{
	if (path.parent_path().parent_path().filename() != "Contents") {
		return true;
	}

	const std::string architecture = path.parent_path().filename().u8string();
#if defined(_M_ARM64EC)
	return architecture == "arm64ec-win" || architecture == "arm64x-win" || architecture == "x86_64-win";
#elif defined(_M_ARM64) || defined(__aarch64__)
	return architecture == "arm64-win";
#else
	return architecture == "x86_64-win";
#endif
}
#endif

bool isVST3Entry(const fs::directory_entry &entry, std::error_code &error)
{
#ifdef _WIN32
	if (!entry.is_regular_file(error) || error || entry.path().extension() != ".vst3") {
		return false;
	}

	return isWindowsVST3Architecture(entry.path());
#else
	return entry.is_directory(error) && !error && entry.path().extension() == ".vst3";
#endif
}

void logDirectoryIterationError(const fs::path &root, const std::error_code &error)
{
	if (error && !isPermissionDenied(error)) {
		blog(LOG_WARNING, "[VST3 Scanner] Directory iteration stopped at %s: %s", root.u8string().c_str(),
		     error.message().c_str());
	}
}

bool collectVST3Paths(const std::vector<std::string> &searchPaths, std::vector<std::string> &paths,
		      bool stopAfterFirst)
{
	for (const auto &folder : searchPaths) {
		const fs::path root = fs::u8path(folder);
		std::error_code error;
		if (!fs::is_directory(root, error) || error) {
			logDirectoryIterationError(root, error);
			continue;
		}

		fs::recursive_directory_iterator entries(root, fs::directory_options::skip_permission_denied, error);
		const fs::recursive_directory_iterator end;
		while (entries != end) {
			if (error) {
				logDirectoryIterationError(root, error);
				error.clear();
				entries.increment(error);
				continue;
			}

			const auto &entry = *entries;
			std::error_code entryError;
			if (isVST3Entry(entry, entryError)) {
				paths.push_back(entry.path().u8string());
				if (stopAfterFirst) {
					return true;
				}
			}

			entries.increment(error);
		}

		logDirectoryIterationError(root, error);
	}

	return !paths.empty();
}

std::string pluginNameForBundle(const std::string &bundlePath)
{
	return fs::u8path(bundlePath).stem().u8string();
}
} // namespace

std::vector<std::string> VST3Scanner::getDefaultSearchPaths()
{
	std::vector<std::string> paths;

#ifdef _WIN32
	char *programFiles = std::getenv("ProgramFiles");
	if (programFiles) {
		paths.emplace_back(std::string(programFiles) + "\\Common Files\\VST3");
	}

	const wchar_t *localAppData = _wgetenv(L"LOCALAPPDATA");
	if (localAppData) {
		char *utf8 = nullptr;
		if (os_wcs_to_utf8_ptr(localAppData, 0, &utf8)) {
			paths.emplace_back(std::string(utf8) + "\\Programs\\Common\\VST3");
		}
		bfree(utf8);
	}
#elif defined(__APPLE__)
	paths.emplace_back("/Library/Audio/Plug-Ins/VST3");
	if (const char *home = std::getenv("HOME")) {
		paths.emplace_back(std::string(home) + "/Library/Audio/Plug-Ins/VST3");
	}
#elif defined(__linux__)
	paths.emplace_back("/usr/lib/vst3");
	paths.emplace_back("/usr/local/lib/vst3");
	if (const char *home = std::getenv("HOME")) {
		paths.emplace_back(std::string(home) + "/.vst3");
	}
#endif
	return paths;
}

std::vector<std::string> VST3Scanner::getVST3Paths()
{
	std::vector<std::string> fsPaths;
	collectVST3Paths(getDefaultSearchPaths(), fsPaths, false);

	std::sort(fsPaths.begin(), fsPaths.end());
	fsPaths.erase(std::unique(fsPaths.begin(), fsPaths.end()), fsPaths.end());
	return fsPaths;
}

static std::string lowerAscii(std::string s) noexcept
{
	for (auto &c : s) {
		if (c >= 'A' && c <= 'Z') {
			c = static_cast<char>(c - 'A' + 'a');
		}
	}
	return s;
}

void VST3Scanner::sort()
{
	std::sort(pluginList.begin(), pluginList.end(), [](const VST3ClassInfo &a, const VST3ClassInfo &b) {
		const auto an = lowerAscii(a.name);
		const auto bn = lowerAscii(b.name);
		if (an != bn) {
			return an < bn;
		}

		const auto ap = lowerAscii(a.pluginName);
		const auto bp = lowerAscii(b.pluginName);
		if (ap != bp) {
			return ap < bp;
		}

		if (a.path != b.path) {
			return a.path < b.path;
		}

		return a.id < b.id;
	});
}

bool VST3Scanner::hasVST3()
{
	std::vector<std::string> paths;
	return collectVST3Paths(getDefaultSearchPaths(), paths, true);
}

static bool isInstrument(const std::vector<std::string> &subCategories)
{
	for (const auto &category : subCategories) {
		if (category.find("Instrument") != std::string::npos) {
			return true;
		}
	}

	return false;
}

bool VST3Scanner::scanForVST3Plugins()
{
	pluginList.clear();
	classCount.clear();
	auto paths = getVST3Paths();

	for (const auto &bundlePath : paths) {
		if (!tryReadModuleInfo(bundlePath)) {
			addModuleClasses(bundlePath);
		}
	}
	sort();
	return !pluginList.empty();
}

void VST3Scanner::updateModulesList(std::unordered_map<std::string, ModuleCache> &modules)
{
	const std::vector<std::string> fsPaths = getVST3Paths();
	const std::unordered_set<std::string> fsPathSet(fsPaths.begin(), fsPaths.end());

	for (auto it = modules.begin(); it != modules.end();) {
		if (!fsPathSet.count(it->first)) {
			it = modules.erase(it);
		} else {
			++it;
		}
	}

	for (const auto &path : fsPaths) {
		if (!modules.count(path)) {
			if (!tryReadModuleInfo(path)) {
				addModuleClasses(path);
			}
		}
	}
}

bool VST3Scanner::tryReadModuleInfo(const std::string &bundlePath)
{
	namespace fs = std::filesystem;

	fs::path p = fs::u8path(bundlePath);
	fs::path bundleRoot;
	std::error_code error;

#ifdef _WIN32
	if (!fs::is_regular_file(p, error) || error) {
		return false;
	}

	bundleRoot = p.parent_path().parent_path().parent_path();
#else
	if (!fs::is_directory(p, error) || error) {
		return false;
	}

	bundleRoot = p;
#endif

	fs::path jsonPath = bundleRoot / "Contents" / "Resources" / "moduleinfo.json";

	error.clear();
	if (!fs::is_regular_file(jsonPath, error) || error) {
		return false;
	}

	char *jsonBuf = os_quick_read_utf8_file(jsonPath.u8string().c_str());
	if (!jsonBuf) {
		return false;
	}

	std::string json(jsonBuf);
	bfree(jsonBuf);

	auto parsed = Steinberg::ModuleInfoLib::parseJson(json, nullptr);
	if (!parsed) {
		return false;
	}

	bool discardable = parsed.value().factoryInfo.flags & Steinberg::PFactoryInfo::kClassesDiscardable;

	if (discardable) {
		addModuleClasses(bundlePath);
		return true;
	}

	loadFromModuleInfo(*parsed, bundleRoot.u8string());
	return true;
}

bool VST3Scanner::loadFromModuleInfo(const Steinberg::ModuleInfo &info, const std::string &bundleRoot)
{
	const std::string pluginName = std::filesystem::u8path(bundleRoot).stem().u8string();
	size_t added = 0;
	bool discardable = info.factoryInfo.flags & Steinberg::PFactoryInfo::kClassesDiscardable;
	for (const auto &c : info.classes) {
		if (c.category != kVstAudioEffectClass) {
			continue;
		}

		VST3ClassInfo entry;
		entry.id = c.cid;
		entry.name = c.name;
		entry.path = bundleRoot;
		entry.pluginName = pluginName;
		entry.discardable = discardable;
		entry.isFX = !isInstrument(c.subCategories);

		appendClassInfo(std::move(entry), bundleRoot);
		++added;
	}

	return added > 0;
}

bool VST3Scanner::addModuleClasses(const std::string &bundlePath)
{
	if (!moduleProbe_) {
		return false;
	}

	std::vector<VST3ClassInfo> classes;
	if (!moduleProbe_(bundlePath, classes)) {
		return false;
	}

	const std::string pluginName = pluginNameForBundle(bundlePath);
	for (auto &classInfo : classes) {
		classInfo.path = bundlePath;
		classInfo.pluginName = pluginName;
		appendClassInfo(std::move(classInfo), bundlePath);
	}

	return true;
}

bool VST3Scanner::loadModuleClassesInProcess(const std::string &bundlePath)
{
	std::string error;
	const std::string pluginName = pluginNameForBundle(bundlePath);
	loadedModule_ = VST3::Hosting::Module::create(bundlePath, error);

	if (!loadedModule_) {
		return false;
	}

	VST3::Hosting::PluginFactory factory = loadedModule_->getFactory();
	bool discardable = factory.info().classesDiscardable();
	for (const auto &classInfo : factory.classInfos()) {
		if (classInfo.category() != kVstAudioEffectClass) {
			continue;
		}

		VST3ClassInfo entry;
		entry.id = classInfo.ID().toString();
		entry.name = classInfo.name();
		entry.pluginName = pluginName;
		entry.path = bundlePath;
		entry.discardable = discardable;
		entry.isFX = !isInstrument(classInfo.subCategories());

		appendClassInfo(std::move(entry), bundlePath);
	}

	return true;
}

void VST3Scanner::appendClassInfo(VST3ClassInfo classInfo, const std::string &bundlePath)
{
	pluginList.push_back(std::move(classInfo));
	++classCount[bundlePath];
}

void VST3Scanner::setModuleProbe(ModuleProbe probe)
{
	moduleProbe_ = std::move(probe);
}

bool VST3Scanner::moduleHasMultipleClasses(const std::string &bundlePath) const
{
	auto it = classCount.find(bundlePath);
	return it != classCount.end() && it->second > 1;
}

std::string VST3Scanner::getNameById(const std::string &class_id) const
{
	for (const auto &c : pluginList) {
		if (c.id == class_id) {
			return c.name;
		}
	}
	return {};
}

std::string VST3Scanner::getPathById(const std::string &class_id) const
{
	for (const auto &c : pluginList) {
		if (c.id == class_id) {
			return c.path;
		}
	}
	return {};
}
