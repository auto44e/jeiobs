/*
 * JeiOBS workspace snapshot support.
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
#include "jeiobs/workspace/Redactor.hpp"

#include "jeiobs/workspace/IniScan.hpp"
#include "jeiobs/workspace/JsonScan.hpp"
#include "jeiobs/workspace/Limits.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Jeiobs::Workspace {
namespace {

struct Edit {
	Redaction report;
	std::string replacement;
};

bool IsJsonRole(std::string_view role)
{
	return role == "profile.service" || role == "profile.service.bak" || role == "module.obs-websocket" ||
	       role == "collection" || role == "collection.bak" || role == "profile.encoder.stream" ||
	       role == "profile.encoder.stream.bak" || role == "profile.encoder.record" ||
	       role == "profile.encoder.record.bak" || role == "app.modules";
}

bool IsIniRole(std::string_view role)
{
	return role == "profile.basic" || role == "app.user";
}

bool HasRegistry(std::string_view role)
{
	return role != "profile.encoder.stream" && role != "profile.encoder.stream.bak" &&
	       role != "profile.encoder.record" && role != "profile.encoder.record.bak" && role != "app.modules" &&
	       role != "app.user";
}

bool IsPlaceholder(std::string_view placeholder)
{
	if (placeholder.size() != 32U) {
		return false;
	}
	for (const char character : placeholder) {
		if (!((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
		      (character >= '0' && character <= '9'))) {
			return false;
		}
	}
	return true;
}

bool GeneratePlaceholder(std::string &placeholder)
{
	static constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	try {
		std::random_device random;
		std::uniform_int_distribution<std::size_t> distribution(0U, sizeof(kAlphabet) - 2U);
		placeholder.clear();
		placeholder.reserve(32U);
		for (std::size_t index = 0U; index < 32U; ++index) {
			placeholder.push_back(kAlphabet[distribution(random)]);
		}
		return true;
	} catch (...) {
		return false;
	}
}

bool IsValuePointer(const JsonValueOccurrence &value, std::string_view pointer)
{
	return value.pointer == pointer;
}

std::vector<std::size_t> FindValues(const JsonScanResult &scan, std::string_view pointer)
{
	std::vector<std::size_t> matches;
	for (std::size_t index = 0U; index < scan.values.size(); ++index) {
		if (IsValuePointer(scan.values[index], pointer)) {
			matches.push_back(index);
		}
	}
	return matches;
}

const JsonValueOccurrence *FindFirstValue(const JsonScanResult &scan, std::string_view pointer)
{
	for (const JsonValueOccurrence &value : scan.values) {
		if (IsValuePointer(value, pointer)) {
			return &value;
		}
	}
	return nullptr;
}

std::size_t Utf8Width(unsigned char first)
{
	if (first <= 0x7fU) {
		return 1U;
	}
	if (first >= 0xc2U && first <= 0xdfU) {
		return 2U;
	}
	if (first >= 0xe0U && first <= 0xefU) {
		return 3U;
	}
	return 4U;
}

unsigned int HexValue(char character)
{
	if (character >= '0' && character <= '9') {
		return static_cast<unsigned int>(character - '0');
	}
	if (character >= 'a' && character <= 'f') {
		return static_cast<unsigned int>(character - 'a' + 10);
	}
	return static_cast<unsigned int>(character - 'A' + 10);
}

struct RawUnit {
	std::size_t raw_end = 0U;
	std::size_t decoded_bytes = 0U;
};

RawUnit DecodeRawUnit(std::string_view bytes, std::size_t position, std::size_t end)
{
	if (position >= end) {
		return {position, 0U};
	}
	if (bytes[position] != '\\') {
		const std::size_t width = Utf8Width(static_cast<unsigned char>(bytes[position]));
		return {position + width, width};
	}
	if (position + 1U >= end) {
		return {end, 0U};
	}
	const char escape = bytes[position + 1U];
	if (escape != 'u') {
		return {position + 2U, 1U};
	}
	if (position + 6U > end) {
		return {end, 0U};
	}
	unsigned int code_unit = 0U;
	for (std::size_t offset = 0U; offset < 4U; ++offset) {
		code_unit = (code_unit << 4U) | HexValue(bytes[position + 2U + offset]);
	}
	std::size_t raw_end = position + 6U;
	std::size_t decoded_bytes = code_unit <= 0x7fU ? 1U : (code_unit <= 0x7ffU ? 2U : 3U);
	if (code_unit >= 0xd800U && code_unit <= 0xdbffU) {
		raw_end += 6U;
		decoded_bytes = 4U;
	}
	return {raw_end, decoded_bytes};
}

std::size_t MapDecodedBoundary(std::string_view bytes, const JsonValueOccurrence &value, std::size_t boundary,
			       bool beginning)
{
	if (bytes.find('\\', value.content_start) == std::string_view::npos ||
	    bytes.find('\\', value.content_start) >= value.content_end) {
		return value.content_start + boundary;
	}
	std::size_t raw = value.content_start;
	std::size_t decoded = 0U;
	while (raw < value.content_end) {
		const std::size_t raw_start = raw;
		const std::size_t decoded_start = decoded;
		const RawUnit unit = DecodeRawUnit(bytes, raw, value.content_end);
		if (unit.raw_end <= raw || unit.raw_end > value.content_end || unit.decoded_bytes == 0U) {
			return beginning ? raw_start : raw;
		}
		raw = unit.raw_end;
		decoded += unit.decoded_bytes;
		if (boundary == decoded_start) {
			return raw_start;
		}
		if (boundary == decoded) {
			return raw;
		}
		if (boundary < decoded) {
			return beginning ? raw_start : raw;
		}
	}
	return value.content_end;
}

bool IsSchemeCharacter(char character)
{
	return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
	       (character >= '0' && character <= '9') || character == '+' || character == '-' || character == '.';
}

char AsciiLower(char character)
{
	if (character >= 'A' && character <= 'Z') {
		return static_cast<char>(character - 'A' + 'a');
	}
	return character;
}

bool IsCredentialParameter(std::string_view name)
{
	static constexpr std::string_view kNames[] = {"key",      "token",      "access_token", "auth",
						      "password", "passphrase", "secret",       "streamid"};
	for (const std::string_view expected : kNames) {
		if (name.size() != expected.size()) {
			continue;
		}
		bool equal = true;
		for (std::size_t index = 0U; index < name.size(); ++index) {
			if (AsciiLower(name[index]) != expected[index]) {
				equal = false;
				break;
			}
		}
		if (equal) {
			return true;
		}
	}
	return false;
}

struct DecodedRange {
	std::size_t start = 0U;
	std::size_t end = 0U;
};

std::vector<DecodedRange> UrlRanges(std::string_view value)
{
	std::vector<DecodedRange> ranges;
	std::size_t position = 0U;
	while (position < value.size()) {
		if (!IsSchemeCharacter(value[position])) {
			++position;
			continue;
		}
		while (position < value.size() && IsSchemeCharacter(value[position])) {
			++position;
		}
		if (position + 3U > value.size() || value.substr(position, 3U) != "://") {
			continue;
		}
		const std::size_t authority_start = position + 3U;
		std::size_t authority_end = authority_start;
		while (authority_end < value.size() && value[authority_end] != '/' && value[authority_end] != '?' &&
		       value[authority_end] != '#') {
			++authority_end;
		}
		const std::size_t at = value.rfind('@', authority_end == 0U ? 0U : authority_end - 1U);
		if (at != std::string_view::npos && at >= authority_start) {
			ranges.push_back({authority_start, at + 1U});
		}
		const std::size_t query = value.find('?', authority_start);
		if (query != std::string_view::npos && query < value.find('#', query)) {
			const std::size_t fragment = value.find('#', query);
			const std::size_t query_end = fragment == std::string_view::npos ? value.size() : fragment;
			std::size_t parameter_start = query + 1U;
			while (parameter_start <= query_end) {
				const std::size_t ampersand = value.find('&', parameter_start);
				const std::size_t parameter_end =
					ampersand == std::string_view::npos || ampersand > query_end ? query_end
												     : ampersand;
				const std::size_t equals = value.find('=', parameter_start);
				if (equals != std::string_view::npos && equals < parameter_end &&
				    IsCredentialParameter(value.substr(parameter_start, equals - parameter_start))) {
					ranges.push_back({equals + 1U, parameter_end});
				}
				if (parameter_end == query_end) {
					break;
				}
				parameter_start = parameter_end + 1U;
			}
		}
	}
	return ranges;
}

void AddEdit(std::vector<Edit> &edits, std::string_view pointer, std::string_view kind, std::size_t start,
	     std::size_t end, std::string replacement, bool has_was = false, bool was = false)
{
	Edit edit;
	edit.report.pointer = std::string(pointer);
	edit.report.kind = std::string(kind);
	edit.report.start = start;
	edit.report.end = end;
	edit.report.replacement_size = replacement.size();
	edit.report.has_was = has_was;
	edit.report.was = was;
	edit.replacement = std::move(replacement);
	edits.push_back(std::move(edit));
}

void AddJsonStringEdit(std::vector<Edit> &edits, const JsonValueOccurrence &value, std::string_view pointer,
		       std::string_view kind, std::string replacement)
{
	AddEdit(edits, pointer, kind, value.content_start, value.content_end, std::move(replacement));
}

void AddJsonUrlEdits(std::vector<Edit> &edits, std::string_view bytes, const JsonValueOccurrence &value,
		     std::string_view pointer, std::string_view value_to_scan)
{
	for (const DecodedRange range : UrlRanges(value_to_scan)) {
		const std::size_t start = MapDecodedBoundary(bytes, value, range.start, true);
		const std::size_t end = MapDecodedBoundary(bytes, value, range.end, false);
		AddEdit(edits, pointer, "url_credential", start, end, {});
	}
}

void AddJsonUrlAt(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan,
		  std::string_view pointer, bool whitespace_tokens, std::size_t &unreviewed)
{
	for (const std::size_t index : FindValues(scan, pointer)) {
		const JsonValueOccurrence &value = scan.values[index];
		if (value.kind != JsonValueKind::String) {
			++unreviewed;
			continue;
		}
		if (!whitespace_tokens) {
			AddJsonUrlEdits(edits, bytes, value, pointer, value.string_value);
			continue;
		}
		std::size_t token_start = 0U;
		while (token_start < value.string_value.size()) {
			while (token_start < value.string_value.size() &&
			       (value.string_value[token_start] == ' ' || value.string_value[token_start] == '\t' ||
				value.string_value[token_start] == '\r' || value.string_value[token_start] == '\n')) {
				++token_start;
			}
			if (token_start == value.string_value.size()) {
				break;
			}
			std::size_t token_end = token_start;
			while (token_end < value.string_value.size() && value.string_value[token_end] != ' ' &&
			       value.string_value[token_end] != '\t' && value.string_value[token_end] != '\r' &&
			       value.string_value[token_end] != '\n') {
				++token_end;
			}
			const std::string_view token =
				std::string_view(value.string_value).substr(token_start, token_end - token_start);
			if (token.find("://") != std::string_view::npos) {
				for (const DecodedRange range : UrlRanges(token)) {
					const std::size_t start =
						MapDecodedBoundary(bytes, value, token_start + range.start, true);
					const std::size_t end =
						MapDecodedBoundary(bytes, value, token_start + range.end, false);
					AddEdit(edits, pointer, "url_credential", start, end, {});
				}
			}
			token_start = token_end;
		}
	}
}

void AddJsonStringAt(std::vector<Edit> &edits, const JsonScanResult &scan, std::string_view pointer,
		     std::string_view kind, std::size_t &unreviewed)
{
	for (const std::size_t index : FindValues(scan, pointer)) {
		const JsonValueOccurrence &value = scan.values[index];
		if (value.kind != JsonValueKind::String) {
			++unreviewed;
			continue;
		}
		AddJsonStringEdit(edits, value, pointer, kind, {});
	}
}

bool IsSourceItemPointer(std::string_view pointer, std::size_t &index)
{
	const std::string_view prefix = "/sources/";
	if (pointer.substr(0U, prefix.size()) != prefix) {
		return false;
	}
	const std::string_view suffix = pointer.substr(prefix.size());
	if (suffix.empty() || suffix.find('/') != std::string_view::npos) {
		return false;
	}
	std::size_t parsed = 0U;
	for (const char character : suffix) {
		if (character < '0' || character > '9') {
			return false;
		}
		parsed = parsed * 10U + static_cast<std::size_t>(character - '0');
	}
	index = parsed;
	return true;
}

void RedactService(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan,
		   std::size_t &unreviewed)
{
	const JsonValueOccurrence *type = FindFirstValue(scan, "/type");
	if (type == nullptr || type->kind != JsonValueKind::String || type->string_value != "rtmp_common") {
		++unreviewed;
	}
	AddJsonStringAt(edits, scan, "/settings/key", "stream_key", unreviewed);
	AddJsonStringAt(edits, scan, "/settings/bearer_token", "bearer_token", unreviewed);
	AddJsonStringAt(edits, scan, "/settings/username", "rtmp_auth", unreviewed);
	AddJsonStringAt(edits, scan, "/settings/password", "rtmp_auth", unreviewed);
	AddJsonUrlAt(edits, bytes, scan, "/settings/server", false, unreviewed);
}

void RedactWebsocket(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan,
		     std::string_view placeholder, std::size_t &unreviewed)
{
	static constexpr std::string_view kAllowed[] = {"first_load",     "server_enabled", "server_port",
							"alerts_enabled", "debug_enabled",  "auth_required",
							"server_password"};
	for (const JsonMemberOccurrence &member : scan.members) {
		if (member.pointer.find('/', 1U) != std::string::npos) {
			continue;
		}
		bool known = false;
		for (const std::string_view allowed : kAllowed) {
			if (member.key == allowed) {
				known = true;
				break;
			}
		}
		if (!known) {
			++unreviewed;
		}
	}

	for (const std::size_t index : FindValues(scan, "/server_password")) {
		const JsonValueOccurrence &value = scan.values[index];
		if (value.kind != JsonValueKind::String) {
			++unreviewed;
		} else {
			AddJsonStringEdit(edits, value, "/server_password", "ws_password", std::string(placeholder));
		}
	}
	for (const std::size_t index : FindValues(scan, "/auth_required")) {
		const JsonValueOccurrence &value = scan.values[index];
		if (value.kind == JsonValueKind::Boolean && !value.boolean_value) {
			AddEdit(edits, "/auth_required", "forced", value.start, value.end, "true", true, false);
		}
	}
	(void)bytes;
}

std::vector<std::size_t> SourceIndices(const JsonScanResult &scan)
{
	std::set<std::size_t> indices;
	for (const JsonValueOccurrence &value : scan.values) {
		std::size_t index = 0U;
		if (IsSourceItemPointer(value.pointer, index)) {
			indices.insert(index);
		}
	}
	return {indices.begin(), indices.end()};
}

void RedactCollection(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan,
		      std::size_t &unreviewed)
{
	const JsonValueOccurrence *sources = FindFirstValue(scan, "/sources");
	const std::vector<std::size_t> indices = sources != nullptr && sources->kind == JsonValueKind::Array
							 ? SourceIndices(scan)
							 : std::vector<std::size_t>{};
	std::size_t unknown_sources = 0U;
	std::size_t ignored_unreviewed = 0U;
	for (const std::size_t source_index : indices) {
		const std::string id_pointer = "/sources/" + std::to_string(source_index) + "/id";
		const JsonValueOccurrence *id = FindFirstValue(scan, id_pointer);
		if (id == nullptr || id->kind != JsonValueKind::String ||
		    (id->string_value != "scene" && id->string_value != "group")) {
			++unknown_sources;
		}
		if (id == nullptr || id->kind != JsonValueKind::String) {
			continue;
		}
		const std::string source_prefix = "/sources/" + std::to_string(source_index);
		if (id->string_value == "browser_source") {
			AddJsonUrlAt(edits, bytes, scan, source_prefix + "/settings/url", false, ignored_unreviewed);
		} else if (id->string_value == "ffmpeg_source") {
			const JsonValueOccurrence *local =
				FindFirstValue(scan, source_prefix + "/settings/is_local_file");
			if (local != nullptr && local->kind == JsonValueKind::Boolean && !local->boolean_value) {
				AddJsonUrlAt(edits, bytes, scan, source_prefix + "/settings/input", false,
					     ignored_unreviewed);
			}
			AddJsonUrlAt(edits, bytes, scan, source_prefix + "/settings/ffmpeg_options", true,
				     ignored_unreviewed);
		} else if (id->string_value == "vlc_source") {
			const std::string playlist_prefix = source_prefix + "/settings/playlist/";
			for (const JsonValueOccurrence &value : scan.values) {
				if (value.pointer.substr(0U, playlist_prefix.size()) != playlist_prefix ||
				    value.pointer.size() <= playlist_prefix.size() + 6U ||
				    value.pointer.substr(value.pointer.size() - 6U) != "/value") {
					continue;
				}
				if (value.kind != JsonValueKind::String) {
					++ignored_unreviewed;
				} else {
					AddJsonUrlEdits(edits, bytes, value, value.pointer, value.string_value);
				}
			}
		}
	}
	unreviewed += std::max<std::size_t>(1U, unknown_sources);
	(void)ignored_unreviewed;
}

bool IsAllowedBasicSection(std::string_view section)
{
	static constexpr std::string_view kAllowed[] = {"General", "Video",   "Audio",    "Output",     "SimpleOutput",
							"AdvOut",  "Stream1", "Panels",   "Hotkeys",    "Auth",
							"Twitch",  "YouTube", "Restream", "BasicWindow"};
	for (const std::string_view allowed : kAllowed) {
		if (section == allowed) {
			return true;
		}
	}
	return false;
}

void AddIniUrlEdit(std::vector<Edit> &edits, std::string_view bytes, const IniKeyOccurrence &key)
{
	const std::string_view value = bytes.substr(key.value_start, key.value_end - key.value_start);
	for (const DecodedRange range : UrlRanges(value)) {
		AddEdit(edits, key.pointer, "url_credential", key.value_start + range.start,
			key.value_start + range.end, {});
	}
}

void RedactBasicWithBytes(std::vector<Edit> &edits, std::string_view bytes, const IniScanResult &scan,
			  std::size_t &unreviewed)
{
	std::set<std::string> unknown_sections;
	for (const IniSectionOccurrence &section : scan.sections) {
		if (!IsAllowedBasicSection(section.name)) {
			unknown_sections.insert(section.name);
		}
	}
	unreviewed += unknown_sections.size();
	for (const IniKeyOccurrence &key : scan.keys) {
		if ((key.section == "Twitch" || key.section == "Restream" || key.section == "YouTube") &&
		    (key.key == "Token" || key.key == "RefreshToken")) {
			AddEdit(edits, key.pointer, "oauth_token", key.value_start, key.value_end, {});
		} else if (key.section == "AdvOut" && key.key == "FFURL" && key.value_start != key.value_end) {
			++unreviewed;
			AddIniUrlEdit(edits, bytes, key);
		}
	}
}

void RedactUser(std::string_view bytes, const IniScanResult &scan, std::size_t &unreviewed)
{
	for (const IniKeyOccurrence &key : scan.keys) {
		if (key.section == "BasicWindow" && key.key == "ExtraBrowserDocks" &&
		    key.value_start != key.value_end &&
		    bytes.substr(key.value_start, key.value_end - key.value_start).size() > 0U) {
			unreviewed = std::max<std::size_t>(unreviewed, 1U);
		}
	}
}

bool ApplyEdits(std::string_view bytes, std::vector<Edit> &edits, RedactResult &result)
{
	std::sort(edits.begin(), edits.end(), [](const Edit &left, const Edit &right) {
		if (left.report.start != right.report.start) {
			return left.report.start < right.report.start;
		}
		if (left.report.end != right.report.end) {
			return left.report.end < right.report.end;
		}
		return left.report.pointer < right.report.pointer;
	});
	std::vector<Edit> unique;
	unique.reserve(edits.size());
	for (Edit &edit : edits) {
		if (edit.report.start > edit.report.end || edit.report.end > bytes.size()) {
			return false;
		}
		if (!unique.empty() && unique.back().report.start == edit.report.start &&
		    unique.back().report.end == edit.report.end) {
			continue;
		}
		if (!unique.empty() && unique.back().report.end > edit.report.start) {
			return false;
		}
		unique.push_back(std::move(edit));
	}
	result.bytes.reserve(bytes.size());
	std::size_t position = 0U;
	for (const Edit &edit : unique) {
		result.bytes.append(bytes.substr(position, edit.report.start - position));
		result.bytes += edit.replacement;
		position = edit.report.end;
		result.report.redactions.push_back(edit.report);
	}
	result.bytes.append(bytes.substr(position));
	return true;
}

RedactResult InvalidResult(std::string_view role, std::string_view bytes)
{
	RedactResult result;
	result.report.parse_invalid = true;
	if (HasRegistry(role)) {
		result.report.outcome = RedactOutcome::Omitted;
		result.report.reason = "unparseable";
		return result;
	}
	result.report.outcome = RedactOutcome::Stored;
	result.report.fidelity = "opaque-unreviewed";
	result.report.unreviewed_count = 1U;
	result.report.redactions.clear();
	result.bytes.assign(bytes);
	return result;
}

} // namespace

bool IsRedactRole(std::string_view role) noexcept
{
	return IsJsonRole(role) || IsIniRole(role);
}

RedactResult Redact(std::string_view role, std::string_view bytes, const RedactOptions &options)
{
	RedactResult result;
	try {
		if (!IsRedactRole(role)) {
			result.error = {ErrorCode::Schema, "role"};
			return result;
		}
		if (bytes.size() > Limits::kDocumentBytes) {
			result.error = {ErrorCode::Limit, "blob"};
			return result;
		}

		std::string placeholder;
		if (options.has_ws_placeholder || !options.ws_placeholder.empty()) {
			if (!IsPlaceholder(options.ws_placeholder)) {
				result.error = {ErrorCode::Schema, "ws-placeholder"};
				return result;
			}
			placeholder = options.ws_placeholder;
		}

		if (IsJsonRole(role)) {
			const JsonScanResult scan = JsonScan::Scan(bytes);
			if (!scan.ok() || scan.values.empty() || scan.values.front().kind != JsonValueKind::Object) {
				return InvalidResult(role, bytes);
			}
			if (role == "module.obs-websocket" && FindFirstValue(scan, "/server_password") != nullptr &&
			    placeholder.empty() && !GeneratePlaceholder(placeholder)) {
				result.error = {ErrorCode::Io, {}};
				return result;
			}
			std::vector<Edit> edits;
			std::size_t unreviewed = 0U;
			if (role == "profile.service" || role == "profile.service.bak") {
				RedactService(edits, bytes, scan, unreviewed);
			} else if (role == "module.obs-websocket") {
				RedactWebsocket(edits, bytes, scan, placeholder, unreviewed);
			} else if (role == "collection" || role == "collection.bak") {
				RedactCollection(edits, bytes, scan, unreviewed);
			}
			result.report.outcome = RedactOutcome::Stored;
			result.report.unreviewed_count = unreviewed;
			result.report.fidelity = unreviewed != 0U ? "opaque-unreviewed"
								  : (edits.empty() ? "exact" : "redacted");
			if (!ApplyEdits(bytes, edits, result)) {
				result = {};
				result.error = {ErrorCode::Json, {}};
			}
			return result;
		}

		const IniScanResult scan = IniScan::Scan(bytes);
		if (!scan.ok()) {
			return InvalidResult(role, bytes);
		}
		std::vector<Edit> edits;
		std::size_t unreviewed = 0U;
		if (role == "profile.basic") {
			RedactBasicWithBytes(edits, bytes, scan, unreviewed);
		} else {
			RedactUser(bytes, scan, unreviewed);
		}
		result.report.outcome = RedactOutcome::Stored;
		result.report.unreviewed_count = unreviewed;
		result.report.fidelity = unreviewed != 0U ? "opaque-unreviewed"
							  : (edits.empty() ? "exact" : "redacted");
		if (!ApplyEdits(bytes, edits, result)) {
			result = {};
			result.error = {ErrorCode::Json, {}};
		}
		return result;
	} catch (...) {
		result = {};
		result.error = {ErrorCode::Json, {}};
		return result;
	}
}

} // namespace Jeiobs::Workspace
