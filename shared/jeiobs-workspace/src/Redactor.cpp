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
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
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

struct MemberLookupKey {
	std::size_t parent = 0U;
	std::string_view key;

	bool operator==(const MemberLookupKey &other) const noexcept
	{
		return parent == other.parent && key == other.key;
	}
};

struct MemberLookupHash {
	std::size_t operator()(const MemberLookupKey &value) const noexcept
	{
		const std::size_t parent_hash = std::hash<std::size_t>{}(value.parent);
		const std::size_t key_hash = std::hash<std::string_view>{}(value.key);
		return parent_hash ^ (key_hash + 0x9e3779b9U + (parent_hash << 6U) + (parent_hash >> 2U));
	}
};

struct ArrayLookupKey {
	std::size_t parent = 0U;
	std::size_t index = 0U;

	bool operator==(const ArrayLookupKey &other) const noexcept
	{
		return parent == other.parent && index == other.index;
	}
};

struct ArrayLookupHash {
	std::size_t operator()(const ArrayLookupKey &value) const noexcept
	{
		const std::size_t parent_hash = std::hash<std::size_t>{}(value.parent);
		const std::size_t index_hash = std::hash<std::size_t>{}(value.index);
		return parent_hash ^ (index_hash + 0x9e3779b9U + (parent_hash << 6U) + (parent_hash >> 2U));
	}
};

class JsonIndex final {
	const JsonScanResult &scan_;
	std::unordered_multimap<MemberLookupKey, std::size_t, MemberLookupHash> members_;
	std::unordered_map<ArrayLookupKey, std::size_t, ArrayLookupHash> arrays_;
	std::unordered_multimap<std::size_t, std::size_t> array_children_;

	static bool DecodeToken(std::string_view encoded, std::string &decoded)
	{
		if (encoded.find('~') == std::string_view::npos) {
			decoded.assign(encoded);
			return true;
		}
		decoded.clear();
		decoded.reserve(encoded.size());
		for (std::size_t index = 0U; index < encoded.size(); ++index) {
			if (encoded[index] != '~') {
				decoded.push_back(encoded[index]);
				continue;
			}
			if (index + 1U >= encoded.size() ||
			    (encoded[index + 1U] != '0' && encoded[index + 1U] != '1')) {
				return false;
			}
			decoded.push_back(encoded[index + 1U] == '0' ? '~' : '/');
			++index;
		}
		return true;
	}

	std::vector<std::size_t> FindChildren(std::size_t parent, std::string_view key) const
	{
		std::vector<std::size_t> result;
		const MemberLookupKey lookup{parent, key};
		const auto range = members_.equal_range(lookup);
		for (auto iterator = range.first; iterator != range.second; ++iterator) {
			result.push_back(iterator->second);
		}
		return result;
	}

public:
	explicit JsonIndex(const JsonScanResult &scan) : scan_(scan)
	{
		if (!scan.members.empty()) {
			members_.reserve(scan.members.size() * 2U + 1U);
		}
		std::size_t array_value_count = 0U;
		for (const JsonValueOccurrence &value : scan.values) {
			if (value.parent_index != kJsonNoIndex && value.array_index != kJsonNoIndex) {
				++array_value_count;
			}
		}
		if (array_value_count != 0U) {
			arrays_.reserve(array_value_count * 2U + 1U);
			array_children_.reserve(array_value_count * 2U + 1U);
		}
		for (const JsonMemberOccurrence &member : scan.members) {
			members_.emplace(MemberLookupKey{member.parent_index, member.key}, member.value_index);
		}
		for (std::size_t index = 0U; index < scan.values.size(); ++index) {
			const JsonValueOccurrence &value = scan.values[index];
			if (value.parent_index != kJsonNoIndex && value.array_index != kJsonNoIndex) {
				arrays_.emplace(ArrayLookupKey{value.parent_index, value.array_index}, index);
				array_children_.emplace(value.parent_index, index);
			}
		}
	}

	std::vector<std::size_t> FindValues(std::string_view pointer) const
	{
		if (scan_.values.empty()) {
			return {};
		}
		if (pointer.empty()) {
			return {0U};
		}
		if (pointer.front() != '/') {
			return {};
		}
		std::vector<std::size_t> current{0U};
		std::size_t start = 1U;
		std::string decoded;
		while (start <= pointer.size()) {
			const std::size_t slash = pointer.find('/', start);
			const std::size_t end = slash == std::string_view::npos ? pointer.size() : slash;
			if (!DecodeToken(pointer.substr(start, end - start), decoded)) {
				return {};
			}
			std::vector<std::size_t> next;
			for (const std::size_t parent : current) {
				if (parent >= scan_.values.size()) {
					continue;
				}
				const JsonValueOccurrence &parent_value = scan_.values[parent];
				if (parent_value.kind == JsonValueKind::Object) {
					const std::vector<std::size_t> children = FindChildren(parent, decoded);
					next.insert(next.end(), children.begin(), children.end());
				} else if (parent_value.kind == JsonValueKind::Array) {
					std::size_t array_index = 0U;
					const std::string_view index_text = decoded;
					if (index_text.empty()) {
						continue;
					}
					const auto parsed = std::from_chars(
						index_text.data(), index_text.data() + index_text.size(), array_index);
					if (parsed.ec == std::errc{} &&
					    parsed.ptr == index_text.data() + index_text.size()) {
						const auto iterator = arrays_.find(ArrayLookupKey{parent, array_index});
						if (iterator != arrays_.end()) {
							next.push_back(iterator->second);
						}
					}
				}
			}
			current = std::move(next);
			if (slash == std::string_view::npos) {
				break;
			}
			start = slash + 1U;
		}
		return current;
	}

	const JsonValueOccurrence *FindFirst(std::string_view pointer) const
	{
		const std::vector<std::size_t> values = FindValues(pointer);
		return values.empty() ? nullptr : &scan_.values[values.front()];
	}

	std::vector<std::size_t> ArrayChildren(std::size_t array_index) const
	{
		std::vector<std::size_t> result;
		const auto range = array_children_.equal_range(array_index);
		for (auto iterator = range.first; iterator != range.second; ++iterator) {
			result.push_back(iterator->second);
		}
		std::sort(result.begin(), result.end(), [this](std::size_t left, std::size_t right) {
			return scan_.values[left].array_index < scan_.values[right].array_index;
		});
		return result;
	}
};

std::string JsonPointer(const JsonScanResult &scan, std::size_t value_index)
{
	std::vector<std::string> tokens;
	std::size_t current = value_index;
	while (current != kJsonNoIndex) {
		const JsonValueOccurrence &value = scan.values[current];
		if (value.member_index != kJsonNoIndex) {
			tokens.push_back(scan.members[value.member_index].key);
		} else if (value.array_index != kJsonNoIndex) {
			tokens.push_back(std::to_string(value.array_index));
		}
		current = value.parent_index;
	}
	std::string pointer;
	for (auto iterator = tokens.rbegin(); iterator != tokens.rend(); ++iterator) {
		pointer.push_back('/');
		for (const char character : *iterator) {
			if (character == '~') {
				pointer += "~0";
			} else if (character == '/') {
				pointer += "~1";
			} else {
				pointer.push_back(character);
			}
		}
	}
	return pointer;
}

std::vector<std::size_t> FindValues(const JsonIndex &index, std::string_view pointer)
{
	return index.FindValues(pointer);
}

const JsonValueOccurrence *FindFirstValue(const JsonIndex &index, std::string_view pointer)
{
	return index.FindFirst(pointer);
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
	if (bytes[position + 1U] != 'u') {
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

using Offset = std::uint32_t;
constexpr Offset kNoOffset = std::numeric_limits<Offset>::max();

void AddCredentialParameter(std::vector<DecodedRange> &ranges, std::string_view value, std::size_t start,
			    std::size_t end)
{
	if (start >= end) {
		return;
	}
	std::size_t equals = start;
	while (equals < end && value[equals] != '=') {
		if (equals - start >= 16U) {
			return;
		}
		++equals;
	}
	if (equals >= end || !IsCredentialParameter(value.substr(start, equals - start))) {
		return;
	}
	ranges.push_back({equals + 1U, end});
}

std::vector<DecodedRange> UrlRanges(std::string_view value)
{
	std::vector<DecodedRange> ranges;
	if (value.find("://") == std::string_view::npos) {
		return ranges;
	}

	const std::size_t size = value.size();
	std::vector<Offset> next_stop(size + 1U, static_cast<Offset>(size));
	std::vector<Offset> next_query_hash(size + 1U, static_cast<Offset>(size));
	std::vector<Offset> next_ampersand(size + 1U, static_cast<Offset>(size));
	std::vector<Offset> last_at(size + 1U, kNoOffset);
	for (std::size_t index = size; index-- > 0U;) {
		next_stop[index] = next_stop[index + 1U];
		next_query_hash[index] = next_query_hash[index + 1U];
		next_ampersand[index] = next_ampersand[index + 1U];
		if (value[index] == '/' || value[index] == '?' || value[index] == '#') {
			next_stop[index] = static_cast<Offset>(index);
		}
		if (value[index] == '?' || value[index] == '#') {
			next_query_hash[index] = static_cast<Offset>(index);
		}
		if (value[index] == '&') {
			next_ampersand[index] = static_cast<Offset>(index);
		}
	}
	for (std::size_t index = 0U; index < size; ++index) {
		last_at[index + 1U] = last_at[index];
		if (value[index] == '@') {
			last_at[index + 1U] = static_cast<Offset>(index);
		}
	}

	std::vector<std::uint8_t> query_starts(size, 0U);
	for (std::size_t position = 0U; position < size;) {
		if (!IsSchemeCharacter(value[position])) {
			++position;
			continue;
		}
		const std::size_t scheme_end_start = position;
		while (position < size && IsSchemeCharacter(value[position])) {
			++position;
		}
		if (position + 3U > size || value.substr(position, 3U) != "://") {
			continue;
		}
		const std::size_t authority_start = position + 3U;
		const std::size_t authority_end = static_cast<std::size_t>(
			next_stop[authority_start] == static_cast<Offset>(size) ? size : next_stop[authority_start]);
		const Offset at = last_at[authority_end];
		if (at != kNoOffset && static_cast<std::size_t>(at) >= authority_start) {
			ranges.push_back({authority_start, static_cast<std::size_t>(at)});
		}

		const std::size_t query_or_hash = next_query_hash[authority_start] == static_cast<Offset>(size)
							  ? size
							  : static_cast<std::size_t>(next_query_hash[authority_start]);
		if (query_or_hash < size && value[query_or_hash] == '?' &&
		    (authority_end == size || query_or_hash == authority_end ||
		     (authority_end < size && value[authority_end] == '/'))) {
			query_starts[query_or_hash] = 1U;
		}
		if (position <= scheme_end_start) {
			++position;
		}
	}

	bool query_active = false;
	for (std::size_t index = 0U; index < size; ++index) {
		if (query_starts[index] != 0U) {
			query_active = true;
			const std::size_t end = next_ampersand[index + 1U] == static_cast<Offset>(size)
							? size
							: static_cast<std::size_t>(next_ampersand[index + 1U]);
			AddCredentialParameter(ranges, value, index + 1U, end);
		}
		if (query_active && value[index] == '&') {
			const std::size_t start = index + 1U;
			const std::size_t end = next_ampersand[start] == static_cast<Offset>(size)
							? size
							: static_cast<std::size_t>(next_ampersand[start]);
			AddCredentialParameter(ranges, value, start, end);
		}
	}
	return ranges;
}

std::vector<Offset> BuildBoundaryMap(std::string_view bytes, const JsonValueOccurrence &value)
{
	std::vector<Offset> boundaries;
	boundaries.reserve(value.string_value.size() + 1U);
	boundaries.push_back(static_cast<Offset>(value.content_start));
	std::size_t raw = value.content_start;
	while (raw < value.content_end) {
		const RawUnit unit = DecodeRawUnit(bytes, raw, value.content_end);
		if (unit.raw_end <= raw || unit.raw_end > value.content_end || unit.decoded_bytes == 0U) {
			break;
		}
		for (std::size_t index = 0U; index < unit.decoded_bytes; ++index) {
			boundaries.push_back(static_cast<Offset>(unit.raw_end));
		}
		raw = unit.raw_end;
	}
	return boundaries;
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
		     std::string_view pointer)
{
	const std::vector<DecodedRange> ranges = UrlRanges(value.string_value);
	const std::size_t escape = bytes.find('\\', value.content_start);
	const bool has_escape = escape != std::string_view::npos && escape < value.content_end;
	std::vector<Offset> boundaries;
	if (has_escape) {
		boundaries = BuildBoundaryMap(bytes, value);
	}
	for (const DecodedRange range : ranges) {
		std::size_t start = value.content_start + range.start;
		std::size_t end = value.content_start + range.end;
		if (has_escape && range.start < boundaries.size() && range.end < boundaries.size()) {
			start = boundaries[range.start];
			end = boundaries[range.end];
		}
		AddEdit(edits, pointer, "url_credential", start, end, {});
	}
}

void AddJsonUrlValues(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan,
		      const std::vector<std::size_t> &values, std::string_view pointer, std::size_t &unreviewed)
{
	for (const std::size_t value_index : values) {
		const JsonValueOccurrence &value = scan.values[value_index];
		if (value.kind != JsonValueKind::String) {
			++unreviewed;
			continue;
		}
		AddJsonUrlEdits(edits, bytes, value, pointer);
	}
}

void AddJsonStringAt(std::vector<Edit> &edits, const JsonScanResult &scan, const JsonIndex &index,
		     std::string_view pointer, std::string_view kind, std::size_t &unreviewed)
{
	for (const std::size_t value_index : FindValues(index, pointer)) {
		const JsonValueOccurrence &value = scan.values[value_index];
		if (value.kind != JsonValueKind::String) {
			++unreviewed;
			continue;
		}
		AddJsonStringEdit(edits, value, pointer, kind, {});
	}
}

void RedactService(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan, const JsonIndex &index,
		   std::size_t &unreviewed)
{
	const JsonValueOccurrence *type = FindFirstValue(index, "/type");
	if (type == nullptr || type->kind != JsonValueKind::String || type->string_value != "rtmp_common") {
		++unreviewed;
	}
	AddJsonStringAt(edits, scan, index, "/settings/key", "stream_key", unreviewed);
	AddJsonStringAt(edits, scan, index, "/settings/bearer_token", "bearer_token", unreviewed);
	AddJsonStringAt(edits, scan, index, "/settings/username", "rtmp_auth", unreviewed);
	AddJsonStringAt(edits, scan, index, "/settings/password", "rtmp_auth", unreviewed);
	AddJsonUrlValues(edits, bytes, scan, FindValues(index, "/settings/server"), "/settings/server", unreviewed);
}

void RedactWebsocket(std::vector<Edit> &edits, const JsonScanResult &scan, const JsonIndex &index,
		     std::string_view placeholder, std::size_t &unreviewed)
{
	static constexpr std::string_view kAllowed[] = {"first_load",     "server_enabled", "server_port",
							"alerts_enabled", "debug_enabled",  "auth_required",
							"server_password"};
	for (const JsonMemberOccurrence &member : scan.members) {
		if (member.parent_index != 0U) {
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

	for (const std::size_t value_index : FindValues(index, "/server_password")) {
		const JsonValueOccurrence &value = scan.values[value_index];
		if (value.kind != JsonValueKind::String) {
			++unreviewed;
		} else {
			AddJsonStringEdit(edits, value, "/server_password", "ws_password", std::string(placeholder));
		}
	}
	for (const std::size_t value_index : FindValues(index, "/auth_required")) {
		const JsonValueOccurrence &value = scan.values[value_index];
		if (value.kind == JsonValueKind::Boolean && !value.boolean_value) {
			AddEdit(edits, "/auth_required", "forced", value.start, value.end, "true", true, false);
		}
	}
}

std::string_view SourceType(const JsonScanResult &scan, const JsonIndex &index, std::size_t source_index,
			    bool &non_string_registry)
{
	const JsonValueOccurrence *versioned = nullptr;
	for (const std::size_t value_index : index.FindValues(JsonPointer(scan, source_index) + "/versioned_id")) {
		versioned = &scan.values[value_index];
		break;
	}
	const std::string id_pointer = JsonPointer(scan, source_index) + "/id";
	const JsonValueOccurrence *id = FindFirstValue(index, id_pointer);
	if (versioned != nullptr) {
		if (versioned->kind == JsonValueKind::String) {
			if (id != nullptr && id->kind != JsonValueKind::String) {
				non_string_registry = true;
			}
			return versioned->string_value;
		}
		non_string_registry = true;
	}
	if (id == nullptr || id->kind != JsonValueKind::String) {
		if (id != nullptr) {
			non_string_registry = true;
		}
		return {};
	}
	return id->string_value;
}

void RedactSource(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan, const JsonIndex &index,
		  std::size_t source_index, std::string_view type, std::size_t &unreviewed)
{
	const std::string settings_pointer = JsonPointer(scan, source_index) + "/settings";
	const JsonValueOccurrence *settings = FindFirstValue(index, settings_pointer);
	if (settings == nullptr || settings->kind != JsonValueKind::Object) {
		return;
	}
	const auto add_url_child = [&](std::string_view key) {
		const std::string pointer = settings_pointer + "/" + std::string(key);
		AddJsonUrlValues(edits, bytes, scan, FindValues(index, pointer), pointer, unreviewed);
	};
	if (type == "browser_source") {
		add_url_child("url");
	} else if (type == "ffmpeg_source") {
		add_url_child("input");
		add_url_child("ffmpeg_options");
	} else if (type == "vlc_source") {
		const std::string playlist_pointer = settings_pointer + "/playlist";
		const JsonValueOccurrence *playlist = FindFirstValue(index, playlist_pointer);
		if (playlist == nullptr || playlist->kind != JsonValueKind::Array) {
			return;
		}
		for (const std::size_t item_index :
		     index.ArrayChildren(static_cast<std::size_t>(playlist - scan.values.data()))) {
			const std::string value_pointer = JsonPointer(scan, item_index) + "/value";
			AddJsonUrlValues(edits, bytes, scan, FindValues(index, value_pointer), value_pointer,
					 unreviewed);
		}
	}
}

void RedactCollection(std::vector<Edit> &edits, std::string_view bytes, const JsonScanResult &scan,
		      const JsonIndex &index, std::size_t &unreviewed)
{
	const JsonValueOccurrence *sources = FindFirstValue(index, "/sources");
	std::vector<std::size_t> source_values;
	if (sources != nullptr && sources->kind == JsonValueKind::Array) {
		source_values = index.ArrayChildren(static_cast<std::size_t>(sources - scan.values.data()));
	}
	std::size_t unknown_sources = 0U;
	std::size_t non_string_registry = 0U;

	for (const std::size_t source_index : source_values) {
		if (scan.values[source_index].kind != JsonValueKind::Object) {
			++unknown_sources;
			continue;
		}
		bool non_string = false;
		const std::string_view type = SourceType(scan, index, source_index, non_string);
		if (non_string) {
			++non_string_registry;
		}
		if (type != "scene" && type != "group") {
			++unknown_sources;
		}
		if (type == "browser_source" || type == "ffmpeg_source" || type == "vlc_source") {
			RedactSource(edits, bytes, scan, index, source_index, type, non_string_registry);
		}
	}
	unreviewed += std::max<std::size_t>(1U, unknown_sources + non_string_registry);
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

std::string_view IniSection(const IniScanResult &scan, const IniKeyOccurrence &key)
{
	return key.section_index == static_cast<std::size_t>(-1) ? std::string_view{}
								 : scan.sections[key.section_index].name;
}

std::string_view IniKey(std::string_view bytes, const IniKeyOccurrence &key)
{
	return bytes.substr(key.key_start, key.key_end - key.key_start);
}

std::string IniPointer(std::string_view section, std::string_view key)
{
	std::string pointer;
	const auto append = [&pointer](std::string_view token) {
		pointer.push_back('/');
		for (const char character : token) {
			if (character == '~') {
				pointer += "~0";
			} else if (character == '/') {
				pointer += "~1";
			} else {
				pointer.push_back(character);
			}
		}
	};
	append(section);
	append(key);
	return pointer;
}

void AddIniUrlEdit(std::vector<Edit> &edits, std::string_view bytes, const IniScanResult &scan,
		   const IniKeyOccurrence &key)
{
	const std::string_view value = bytes.substr(key.value_start, key.value_end - key.value_start);
	const std::string pointer = IniPointer(IniSection(scan, key), IniKey(bytes, key));
	for (const DecodedRange range : UrlRanges(value)) {
		AddEdit(edits, pointer, "url_credential", key.value_start + range.start, key.value_start + range.end,
			{});
	}
}

bool StartsWith(std::string_view value, std::string_view prefix)
{
	return value.size() >= prefix.size() && value.substr(0U, prefix.size()) == prefix;
}

void RedactBasic(std::vector<Edit> &edits, std::string_view bytes, const IniScanResult &scan, std::size_t &unreviewed)
{
	std::set<std::string_view> unknown_sections;
	for (const IniSectionOccurrence &section : scan.sections) {
		if (!IsAllowedBasicSection(section.name)) {
			unknown_sections.insert(section.name);
		}
	}
	unreviewed += unknown_sections.size();
	if (scan.uncertain) {
		++unreviewed;
	}
	bool ffurl_counted = false;
	for (const IniKeyOccurrence &key : scan.keys) {
		const std::string_view section = IniSection(scan, key);
		const std::string_view name = IniKey(bytes, key);
		const bool oauth_name = name == "Token" || name == "RefreshToken";
		const bool oauth_section = StartsWith(section, "Twitch") || StartsWith(section, "Restream") ||
					   StartsWith(section, "YouTube");
		if (oauth_name && (oauth_section || scan.uncertain)) {
			AddEdit(edits, IniPointer(section, name), "oauth_token", key.value_start, key.value_end, {});
		} else if (section == "AdvOut" && name == "FFURL" && key.value_start != key.value_end) {
			if (!ffurl_counted) {
				++unreviewed;
				ffurl_counted = true;
			}
			AddIniUrlEdit(edits, bytes, scan, key);
		}
	}
}

void RedactUser(std::string_view bytes, const IniScanResult &scan, std::size_t &unreviewed)
{
	for (const IniKeyOccurrence &key : scan.keys) {
		if (IniSection(scan, key) == "BasicWindow" && IniKey(bytes, key) == "ExtraBrowserDocks" &&
		    key.value_start != key.value_end) {
			unreviewed = std::max<std::size_t>(unreviewed, 1U);
		}
	}
}

bool IsSpecialKind(std::string_view kind)
{
	return kind == "ws_password" || kind == "forced";
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
		return left.report.kind < right.report.kind;
	});
	std::vector<Edit> normalized;
	normalized.reserve(edits.size());
	for (Edit &edit : edits) {
		if (edit.report.start > edit.report.end || edit.report.end > bytes.size()) {
			return false;
		}
		if (normalized.empty()) {
			normalized.push_back(std::move(edit));
			continue;
		}
		Edit &last = normalized.back();
		const bool overlaps_or_touches = last.report.end >= edit.report.start;
		if (overlaps_or_touches && !IsSpecialKind(last.report.kind) && !IsSpecialKind(edit.report.kind)) {
			last.report.end = std::max(last.report.end, edit.report.end);
			last.report.replacement_size = last.replacement.size();
			continue;
		}
		normalized.push_back(std::move(edit));
	}

	result.bytes.reserve(bytes.size());
	std::size_t position = 0U;
	for (const Edit &edit : normalized) {
		if (edit.report.start < position) {
			return false;
		}
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
			const bool modules_root = role == "app.modules" && !scan.values.empty() &&
						  (scan.values.front().kind == JsonValueKind::Object ||
						   scan.values.front().kind == JsonValueKind::Array);
			if (!scan.ok() || scan.values.empty() ||
			    (scan.values.front().kind != JsonValueKind::Object && !modules_root)) {
				return InvalidResult(role, bytes);
			}
			const JsonIndex index(scan);
			if (role == "module.obs-websocket" && FindFirstValue(index, "/server_password") != nullptr &&
			    placeholder.empty() && !GeneratePlaceholder(placeholder)) {
				result.error = {ErrorCode::Io, {}};
				return result;
			}
			std::vector<Edit> edits;
			std::size_t unreviewed = 0U;
			if (role == "profile.service" || role == "profile.service.bak") {
				RedactService(edits, bytes, scan, index, unreviewed);
			} else if (role == "module.obs-websocket") {
				RedactWebsocket(edits, scan, index, placeholder, unreviewed);
			} else if (role == "collection" || role == "collection.bak") {
				RedactCollection(edits, bytes, scan, index, unreviewed);
			}
			result.report.outcome = RedactOutcome::Stored;
			result.report.unreviewed_count = unreviewed;
			result.report.fidelity = unreviewed != 0U ? "opaque-unreviewed"
								  : (edits.empty() ? "exact" : "redacted");
			if (!ApplyEdits(bytes, edits, result)) {
				result.bytes.assign(bytes);
				result.report.redactions.clear();
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
			RedactBasic(edits, bytes, scan, unreviewed);
		} else {
			RedactUser(bytes, scan, unreviewed);
		}
		result.report.outcome = RedactOutcome::Stored;
		result.report.unreviewed_count = unreviewed;
		result.report.fidelity = unreviewed != 0U ? "opaque-unreviewed"
							  : (edits.empty() ? "exact" : "redacted");
		if (!ApplyEdits(bytes, edits, result)) {
			result.bytes.assign(bytes);
			result.report.redactions.clear();
		}
		return result;
	} catch (...) {
		return InvalidResult(role, bytes);
	}
}

} // namespace Jeiobs::Workspace
