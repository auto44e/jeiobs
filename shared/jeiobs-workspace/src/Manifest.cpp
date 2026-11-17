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
#include "Utf8Path.hpp"
#include "jeiobs/workspace/Manifest.hpp"

#include "jeiobs/workspace/Limits.hpp"
#include "jeiobs/workspace/PathRules.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Jeiobs::Workspace {
namespace {

using JsonSax = nlohmann::json_sax<Json>;

Error MakeError(ErrorCode code, std::string detail = {})
{
	return {code, std::move(detail)};
}

bool IsValidUtf8(std::string_view bytes)
{
	for (std::size_t index = 0U; index < bytes.size();) {
		const unsigned char first = static_cast<unsigned char>(bytes[index]);
		std::size_t width = 0U;
		std::uint32_t value = 0U;
		if (first <= 0x7fU) {
			width = 1U;
			value = first;
		} else if (first >= 0xc2U && first <= 0xdfU) {
			width = 2U;
			value = first & 0x1fU;
		} else if (first >= 0xe0U && first <= 0xefU) {
			width = 3U;
			value = first & 0x0fU;
		} else if (first >= 0xf0U && first <= 0xf4U) {
			width = 4U;
			value = first & 0x07U;
		} else {
			return false;
		}

		if (index + width > bytes.size()) {
			return false;
		}
		for (std::size_t offset = 1U; offset < width; ++offset) {
			const unsigned char continuation = static_cast<unsigned char>(bytes[index + offset]);
			if ((continuation & 0xc0U) != 0x80U) {
				return false;
			}
			value = (value << 6U) | (continuation & 0x3fU);
		}
		if ((width == 2U && value < 0x80U) || (width == 3U && value < 0x800U) ||
		    (width == 4U && value < 0x10000U) || (value >= 0xd800U && value <= 0xdfffU) || value > 0x10ffffU) {
			return false;
		}
		index += width;
	}
	return true;
}

bool ContainsNul(std::string_view value)
{
	return value.find('\0') != std::string_view::npos;
}

bool StringsWithinLimit(std::string_view bytes, std::size_t limit)
{
	for (std::size_t index = 0U; index < bytes.size(); ++index) {
		if (bytes[index] != '"') {
			continue;
		}
		std::size_t decoded = 0U;
		++index;
		while (index < bytes.size()) {
			const unsigned char character = static_cast<unsigned char>(bytes[index]);
			if (character == '"') {
				break;
			}
			std::size_t width = 1U;
			if (character == '\\') {
				if (index + 1U >= bytes.size()) {
					break;
				}
				if (bytes[index + 1U] == 'u' && index + 6U <= bytes.size()) {
					unsigned int code_unit = 0U;
					for (std::size_t offset = 0U; offset < 4U; ++offset) {
						const char digit = bytes[index + 2U + offset];
						if (!((digit >= '0' && digit <= '9') ||
						      (digit >= 'a' && digit <= 'f') ||
						      (digit >= 'A' && digit <= 'F'))) {
							code_unit = 0U;
							break;
						}
						code_unit = (code_unit << 4U) |
							    static_cast<unsigned int>(
								    digit >= '0' && digit <= '9'
									    ? digit - '0'
									    : (digit >= 'a' && digit <= 'f'
										       ? digit - 'a' + 10
										       : digit - 'A' + 10));
					}
					width = code_unit <= 0x7fU ? 1U : (code_unit <= 0x7ffU ? 2U : 3U);
					if (code_unit >= 0xd800U && code_unit <= 0xdbffU &&
					    index + 12U <= bytes.size() && bytes[index + 6U] == '\\' &&
					    bytes[index + 7U] == 'u') {
						width = 4U;
						index += 6U;
					}
					index += 6U;
				} else {
					width = 1U;
					index += 2U;
				}
			} else if (character >= 0x80U) {
				if (character >= 0xc2U && character <= 0xdfU) {
					width = 2U;
				} else if (character >= 0xe0U && character <= 0xefU) {
					width = 3U;
				} else if (character >= 0xf0U && character <= 0xf4U) {
					width = 4U;
				}
				index += width;
			} else {
				++index;
			}
			if (width > limit - decoded) {
				return false;
			}
			decoded += width;
		}
	}
	return true;
}

class BoundedSax final : public JsonSax {
	struct Frame {
		enum class Kind {
			Object,
			Array,
		};

		Kind kind = Kind::Object;
		Json value;
		std::optional<std::string> pendingKey;
		std::unordered_set<std::string> keys;
		bool entriesArray = false;
		std::size_t entries = 0U;
	};

	Json root_;
	bool hasRoot_ = false;
	std::vector<Frame> stack_;
	std::size_t valueCount_ = 0U;
	Error error_;

	bool Fail(Error error)
	{
		if (error_.ok()) {
			error_ = std::move(error);
		}
		return false;
	}

	bool BeforeValue()
	{
		++valueCount_;
		if (valueCount_ > Limits::kValues) {
			return Fail(MakeError(ErrorCode::Limit, "values"));
		}

		if (!stack_.empty() && stack_.back().kind == Frame::Kind::Array && stack_.back().entriesArray) {
			++stack_.back().entries;
			if (stack_.back().entries > Limits::kEntries) {
				return Fail(MakeError(ErrorCode::Limit, "entries"));
			}
		}
		return true;
	}

	bool Attach(Json value)
	{
		if (stack_.empty()) {
			if (hasRoot_) {
				return Fail(MakeError(ErrorCode::Json));
			}
			root_ = std::move(value);
			hasRoot_ = true;
			return true;
		}

		Frame &frame = stack_.back();
		if (frame.kind == Frame::Kind::Array) {
			frame.value.push_back(std::move(value));
			return true;
		}
		if (!frame.pendingKey.has_value()) {
			return Fail(MakeError(ErrorCode::Json));
		}
		frame.value[frame.pendingKey.value()] = std::move(value);
		frame.pendingKey.reset();
		return true;
	}

	bool CheckString(const std::string &value)
	{
		if (value.size() > Limits::kStringBytes) {
			return Fail(MakeError(ErrorCode::Limit, "string"));
		}
		if (ContainsNul(value)) {
			return Fail(MakeError(ErrorCode::Nul));
		}
		return true;
	}

public:
	BoundedSax() = default;

	[[nodiscard]] const Json &root() const noexcept { return root_; }

	[[nodiscard]] const Error &error() const noexcept { return error_; }

	bool null() override { return BeforeValue() && Attach(nullptr); }

	bool boolean(bool value) override { return BeforeValue() && Attach(value); }

	bool number_integer(number_integer_t value) override { return BeforeValue() && Attach(value); }

	bool number_unsigned(number_unsigned_t value) override { return BeforeValue() && Attach(value); }

	bool number_float(number_float_t value, const string_t &) override
	{
		if (!std::isfinite(value)) {
			return Fail(MakeError(ErrorCode::Nonfinite));
		}
		return BeforeValue() && Attach(value);
	}

	bool string(string_t &value) override
	{
		if (!CheckString(value) || !BeforeValue()) {
			return false;
		}
		return Attach(std::move(value));
	}

	bool binary(binary_t &) override { return Fail(MakeError(ErrorCode::Json)); }

	bool start_object(std::size_t) override
	{
		if (!BeforeValue()) {
			return false;
		}
		if (stack_.size() + 1U > Limits::kDepth) {
			return Fail(MakeError(ErrorCode::Limit, "depth"));
		}
		Frame frame;
		frame.kind = Frame::Kind::Object;
		frame.value = Json::object();
		stack_.push_back(std::move(frame));
		return true;
	}

	bool key(string_t &value) override
	{
		if (stack_.empty() || stack_.back().kind != Frame::Kind::Object) {
			return Fail(MakeError(ErrorCode::Json));
		}
		if (!CheckString(value)) {
			return false;
		}
		Frame &frame = stack_.back();
		if (!frame.keys.insert(value).second) {
			return Fail(MakeError(ErrorCode::DuplicateKey));
		}
		frame.pendingKey = std::move(value);
		return true;
	}

	bool end_object() override
	{
		if (stack_.empty() || stack_.back().kind != Frame::Kind::Object ||
		    stack_.back().pendingKey.has_value()) {
			return Fail(MakeError(ErrorCode::Json));
		}
		Json value = std::move(stack_.back().value);
		stack_.pop_back();
		return Attach(std::move(value));
	}

	bool start_array(std::size_t elements) override
	{
		if (!BeforeValue()) {
			return false;
		}
		if (stack_.size() + 1U > Limits::kDepth) {
			return Fail(MakeError(ErrorCode::Limit, "depth"));
		}
		if (!stack_.empty() && stack_.back().kind == Frame::Kind::Object &&
		    stack_.back().pendingKey.has_value() && stack_.back().pendingKey.value() == "entries" &&
		    elements != std::numeric_limits<std::size_t>::max() && elements > Limits::kEntries) {
			return Fail(MakeError(ErrorCode::Limit, "entries"));
		}
		Frame frame;
		frame.kind = Frame::Kind::Array;
		frame.value = Json::array();
		frame.entriesArray = !stack_.empty() && stack_.back().kind == Frame::Kind::Object &&
				     stack_.back().pendingKey.has_value() &&
				     stack_.back().pendingKey.value() == "entries";
		stack_.push_back(std::move(frame));
		return true;
	}

	bool end_array() override
	{
		if (stack_.empty() || stack_.back().kind != Frame::Kind::Array) {
			return Fail(MakeError(ErrorCode::Json));
		}
		Json value = std::move(stack_.back().value);
		stack_.pop_back();
		return Attach(std::move(value));
	}

	bool parse_error(std::size_t, const std::string &, const nlohmann::detail::exception &error) override
	{
		if (error_.ok()) {
			// nlohmann reports a number that overflows double (1e400) as out_of_range 406.
			error_ = MakeError(error.id == 406 ? ErrorCode::Nonfinite : ErrorCode::Json);
		}
		return false;
	}
};

JsonResult ParseJsonInternal(std::string_view bytes)
{
	if (bytes.size() >= 3U && static_cast<unsigned char>(bytes[0]) == 0xefU &&
	    static_cast<unsigned char>(bytes[1]) == 0xbbU && static_cast<unsigned char>(bytes[2]) == 0xbfU) {
		return {std::nullopt, MakeError(ErrorCode::Bom)};
	}
	if (bytes.find('\0') != std::string_view::npos) {
		return {std::nullopt, MakeError(ErrorCode::Nul)};
	}
	if (!IsValidUtf8(bytes)) {
		return {std::nullopt, MakeError(ErrorCode::Utf8)};
	}
	if (!StringsWithinLimit(bytes, Limits::kStringBytes)) {
		return {std::nullopt, MakeError(ErrorCode::Limit, "string")};
	}

	BoundedSax sax;
	bool parsed = false;
	try {
		parsed = Json::sax_parse(bytes.begin(), bytes.end(), &sax, Json::input_format_t::json, true);
	} catch (...) {
		return {std::nullopt, MakeError(ErrorCode::Json)};
	}
	if (!parsed || !sax.error().ok()) {
		if (!sax.error().ok()) {
			return {std::nullopt, sax.error()};
		}
		return {std::nullopt, MakeError(ErrorCode::Json)};
	}
	return {sax.root(), {}};
}

BytesResult ReadFileInternal(const std::string &path)
{
	try {
		const std::filesystem::path filesystem_path = std::filesystem::u8path(path);
		std::error_code status_error;
		const std::filesystem::file_status status = std::filesystem::status(filesystem_path, status_error);
		if (status_error || !std::filesystem::is_regular_file(status)) {
			return {{}, MakeError(ErrorCode::Io)};
		}

		std::error_code size_error;
		const std::uintmax_t file_size = std::filesystem::file_size(filesystem_path, size_error);
		if (size_error) {
			return {{}, MakeError(ErrorCode::Io)};
		}
		if (file_size > Limits::kManifestBytes) {
			return {{}, MakeError(ErrorCode::Limit, "manifest")};
		}

		std::ifstream input;
		Jeiobs::Workspace::Detail::OpenUtf8(input, path);
		if (!input.is_open()) {
			return {{}, MakeError(ErrorCode::Io)};
		}
		std::string bytes(static_cast<std::size_t>(file_size), '\0');
		if (!bytes.empty()) {
			input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
			if (input.gcount() != static_cast<std::streamsize>(bytes.size())) {
				return {{}, MakeError(ErrorCode::Io)};
			}
		}
		return {std::move(bytes), {}};
	} catch (...) {
		return {{}, MakeError(ErrorCode::Io)};
	}
}

bool IsInteger(const Json &value)
{
	return value.is_number_integer() || value.is_number_unsigned();
}

bool IsNonNegativeInteger(const Json &value)
{
	if (value.is_number_unsigned()) {
		return true;
	}
	return value.is_number_integer() && value.get<std::int64_t>() >= 0;
}

bool IsPositiveInteger(const Json &value)
{
	if (value.is_number_unsigned()) {
		return value.get<std::uint64_t>() >= 1U;
	}
	return value.is_number_integer() && value.get<std::int64_t>() >= 1;
}

bool GetUnsigned(const Json &value, std::uint64_t &result)
{
	if (value.is_number_unsigned()) {
		result = value.get<std::uint64_t>();
		return true;
	}
	if (value.is_number_integer()) {
		const std::int64_t signed_value = value.get<std::int64_t>();
		if (signed_value < 0) {
			return false;
		}
		result = static_cast<std::uint64_t>(signed_value);
		return true;
	}
	return false;
}

bool IsHash(const Json &value)
{
	if (!value.is_string() || value.get_ref<const std::string &>().size() != 64U) {
		return false;
	}
	for (const char character : value.get_ref<const std::string &>()) {
		if (!((character >= '0' && character <= '9') || (character >= 'a' && character <= 'f'))) {
			return false;
		}
	}
	return true;
}

Error Schema(std::string field)
{
	return MakeError(ErrorCode::Schema, std::move(field));
}

bool IsObjectWithString(const Json &object, const char *field)
{
	return object.contains(field) && object.at(field).is_string();
}

Error ValidateKnownStringFields(const Json &object, std::string_view detail,
				const std::initializer_list<const char *> &known_fields)
{
	if (!object.is_object()) {
		return Schema(std::string(detail));
	}
	for (const char *field : known_fields) {
		if (object.contains(field) && !object.at(field).is_string()) {
			return Schema(std::string(detail));
		}
	}
	return {};
}

Error ValidateAppKeys(const Json &document)
{
	for (const Json &app_key : document.at("app_keys")) {
		if (!app_key.is_object()) {
			return Schema("app_keys");
		}
		for (const char *field : {"root", "file", "section", "key", "value", "restore"}) {
			if (!IsObjectWithString(app_key, field)) {
				return Schema("app_keys");
			}
		}
		const std::string &restore = app_key.at("restore").get_ref<const std::string &>();
		if (restore != "same-os" && restore != "never") {
			return Schema("app_keys");
		}
	}
	return {};
}

Error ValidateDroppedExtensions(const Json &document)
{
	for (const Json &extension : document.at("dropped_ext")) {
		if (!extension.is_object() || !extension.contains("entry") || !extension.contains("fields")) {
			return Schema("dropped_ext");
		}
		if (!extension.at("entry").is_null() && !extension.at("entry").is_string()) {
			return Schema("dropped_ext");
		}
		if (!extension.at("fields").is_array()) {
			return Schema("dropped_ext");
		}
		for (const Json &field : extension.at("fields")) {
			if (!field.is_string()) {
				return Schema("dropped_ext");
			}
		}
	}
	return {};
}

Error ValidateTargetAudio(const Json &document, ManifestMode mode)
{
	const Json &target = document.at("target");
	if (!target.contains("audio")) {
		return {};
	}
	const Json &audio = target.at("audio");
	if (!audio.is_object() || !audio.contains("sample_rate") || !IsPositiveInteger(audio.at("sample_rate")) ||
	    !audio.contains("channels") || !audio.at("channels").is_string() ||
	    audio.at("channels").get_ref<const std::string &>().empty()) {
		return Schema("target.audio");
	}
	if (audio.contains("explicit")) {
		if (mode == ManifestMode::Export || !audio.at("explicit").is_boolean()) {
			return Schema("target.audio");
		}
	}
	if (audio.contains("src")) {
		if (mode == ManifestMode::Export || !audio.at("src").is_object() ||
		    !IsObjectWithString(audio.at("src"), "entry")) {
			return Schema("target.audio");
		}
	}
	return {};
}

Error ValidateCompat(const Json &document, bool &ro_compat)
{
	if (!document.contains("compat")) {
		return Schema("compat");
	}
	const Json &compat = document.at("compat");
	if (!compat.is_object()) {
		return Schema("compat");
	}
	for (const char *field : {"incompat", "ro_compat"}) {
		if (!compat.contains(field) || !compat.at(field).is_array()) {
			return Schema(field);
		}
		for (const Json &feature : compat.at(field)) {
			if (!feature.is_string()) {
				return Schema(field);
			}
			if (std::string_view(field) == "incompat") {
				return MakeError(ErrorCode::Incompat, feature.get<std::string>());
			}
			ro_compat = true;
		}
	}
	return {};
}

Error ValidateRedactions(const Json &entry, ManifestMode mode, std::vector<std::string> &refs)
{
	if (!entry.contains("redactions")) {
		return {};
	}
	const Json &redactions = entry.at("redactions");
	if (!redactions.is_array()) {
		return Schema("redactions");
	}
	for (const Json &redaction : redactions) {
		if (!redaction.is_object()) {
			return Schema("redactions");
		}
		if (!IsObjectWithString(redaction, "pointer")) {
			return Schema("pointer");
		}
		if (!IsObjectWithString(redaction, "kind")) {
			return Schema("kind");
		}
		if (mode == ManifestMode::Export) {
			if (redaction.contains("ref")) {
				return Schema("ref");
			}
			if (redaction.contains("was")) {
				return Schema("was");
			}
		} else if (redaction.contains("ref")) {
			if (!redaction.at("ref").is_string()) {
				return Schema("ref");
			}
			refs.push_back(redaction.at("ref").get<std::string>());
		}
	}
	return {};
}

bool IsPathComponent(std::string_view value)
{
	return value.find('/') == std::string_view::npos && !value.empty() && ValidatePath(value).ok();
}

bool MatchesPathShape(std::string_view path, std::string_view prefix, std::string_view suffix)
{
	if (path.size() <= prefix.size() + suffix.size() || path.substr(0U, prefix.size()) != prefix ||
	    path.substr(path.size() - suffix.size()) != suffix) {
		return false;
	}
	return IsPathComponent(path.substr(prefix.size(), path.size() - prefix.size() - suffix.size()));
}

bool IsStoredEntryAllowed(std::string_view root, std::string_view role, std::string_view path)
{
	if (root == "user") {
		return role == "app.user" && path == "user.ini";
	}
	if (root == "scenes") {
		if (role == "collection") {
			return MatchesPathShape(path, "basic/scenes/", ".json");
		}
		if (role == "collection.bak") {
			return MatchesPathShape(path, "basic/scenes/", ".json.bak");
		}
		return false;
	}
	if (root == "profiles") {
		if (role == "profile.basic") {
			return MatchesPathShape(path, "basic/profiles/", "/basic.ini");
		}
		if (role == "profile.service") {
			return MatchesPathShape(path, "basic/profiles/", "/service.json");
		}
		if (role == "profile.service.bak") {
			return MatchesPathShape(path, "basic/profiles/", "/service.json.bak");
		}
		if (role == "profile.encoder.stream") {
			return MatchesPathShape(path, "basic/profiles/", "/streamEncoder.json");
		}
		if (role == "profile.encoder.stream.bak") {
			return MatchesPathShape(path, "basic/profiles/", "/streamEncoder.json.bak");
		}
		if (role == "profile.encoder.record") {
			return MatchesPathShape(path, "basic/profiles/", "/recordEncoder.json");
		}
		if (role == "profile.encoder.record.bak") {
			return MatchesPathShape(path, "basic/profiles/", "/recordEncoder.json.bak");
		}
		return false;
	}
	if (root == "plugin_manager") {
		return role == "app.modules" && path == "plugin_manager/modules.json";
	}
	if (root == "config") {
		if (role == "module.obs-websocket") {
			return path == "plugin_config/obs-websocket/config.json";
		}
		if (role == "module.decklink-output-ui") {
			return path == "plugin_config/decklink-output-ui/decklinkOutputProps.json" ||
			       path == "plugin_config/decklink-output-ui/decklinkPreviewOutputProps.json";
		}
		if (role == "module.aja-output-ui") {
			return path == "plugin_config/aja-output-ui/ajaOutputProps.json" ||
			       path == "plugin_config/aja-output-ui/ajaPreviewOutputProps.json" ||
			       path == "plugin_config/aja-output-ui/ajaMiscProps.json";
		}
	}
	return false;
}

Error ValidateEntries(const Json &document, ManifestMode mode, bool &unchecked_paths,
		      std::unordered_set<std::string> &entry_ids, std::vector<std::string> &redaction_refs,
		      std::uint64_t &stored_bytes)
{
	if (!document.contains("entries") || !document.at("entries").is_array()) {
		return Schema("entries");
	}
	const Json &entries = document.at("entries");
	if (entries.size() > Limits::kEntries) {
		return MakeError(ErrorCode::Limit, "entries");
	}

	std::unordered_set<std::string> collision_keys;
	for (const Json &entry : entries) {
		if (!entry.is_object()) {
			return Schema("entry");
		}
		if (!IsObjectWithString(entry, "id")) {
			return Schema("id");
		}
		const std::string id = entry.at("id").get<std::string>();
		if (!entry_ids.insert(id).second) {
			return Schema("id");
		}
		if (!IsObjectWithString(entry, "status")) {
			return Schema("status");
		}
		if (!IsObjectWithString(entry, "root")) {
			return Schema("root");
		}
		const std::string root = entry.at("root").get<std::string>();
		if (root != "config" && root != "user" && root != "scenes" && root != "profiles" &&
		    root != "plugin_manager") {
			return Schema("root");
		}
		if (!IsObjectWithString(entry, "path")) {
			return Schema("path");
		}
		const std::string path = entry.at("path").get<std::string>();
		const PathResult path_result = ValidatePath(path);
		if (!path_result.ok()) {
			return MakeError(IsPathLimitRule(path_result.rule) ? ErrorCode::Limit : ErrorCode::Path,
					 path_result.rule);
		}
		if (path_result.status == PathStatus::Unchecked) {
			unchecked_paths = true;
		}
		const std::string collision_key = root + '\0' + AsciiFold(path);
		if (!collision_keys.insert(collision_key).second) {
			return MakeError(ErrorCode::Collision);
		}

		const std::string status = entry.at("status").get<std::string>();
		if (status != "stored" && status != "omitted") {
			return Schema("status");
		}
		if (!IsObjectWithString(entry, status == "stored" ? "role" : "reason")) {
			return Schema(status == "stored" ? "role" : "reason");
		}
		if (status == "stored" && !IsStoredEntryAllowed(root, entry.at("role").get<std::string>(), path)) {
			return Schema("role");
		}
		if (!entry.contains("size") || !IsNonNegativeInteger(entry.at("size"))) {
			return Schema("size");
		}
		std::uint64_t size = 0U;
		if (!GetUnsigned(entry.at("size"), size)) {
			return Schema("size");
		}
		if (status == "stored") {
			if (!entry.contains("hash")) {
				return Schema("hash");
			}
			if (!IsHash(entry.at("hash"))) {
				return Schema("hash");
			}
			if (!IsObjectWithString(entry, "fidelity")) {
				return Schema("fidelity");
			}
			const std::string fidelity = entry.at("fidelity").get<std::string>();
			if (fidelity != "exact" && fidelity != "redacted" && fidelity != "opaque-unreviewed") {
				return Schema("fidelity");
			}
			if (size > Limits::kBlobBytes) {
				return MakeError(ErrorCode::Limit, "blob");
			}
			if (size > Limits::kRevisionBytes - stored_bytes) {
				return MakeError(ErrorCode::Limit, "revision");
			}
			stored_bytes += size;
			if (mode == ManifestMode::Export && fidelity == "opaque-unreviewed") {
				return Schema("fidelity");
			}
		} else {
			for (const char *field : {"fidelity", "hash", "redactions", "unreviewed", "parse"}) {
				if (entry.contains(field)) {
					return Schema(field);
				}
			}
		}

		for (const char *field : {"profile", "collection", "kind", "load_rule"}) {
			if (entry.contains(field) && !entry.at(field).is_string()) {
				return Schema(field);
			}
		}
		if (entry.contains("bak") && !entry.at("bak").is_string()) {
			return Schema("bak");
		}
		const bool opaque = status == "stored" &&
				    entry.at("fidelity").get<std::string>() == "opaque-unreviewed";
		if (entry.contains("unreviewed")) {
			const Json &unreviewed = entry.at("unreviewed");
			if (!opaque || !unreviewed.is_object() || !IsObjectWithString(unreviewed, "rules") ||
			    unreviewed.at("rules").get_ref<const std::string &>().empty() ||
			    !unreviewed.contains("count") || !IsPositiveInteger(unreviewed.at("count"))) {
				return Schema("unreviewed");
			}
		} else if (opaque) {
			return Schema("unreviewed");
		}
		if (entry.contains("parse") &&
		    (!opaque || !entry.at("parse").is_string() || entry.at("parse").get<std::string>() != "invalid")) {
			return Schema("parse");
		}
		const Error redaction_error = ValidateRedactions(entry, mode, redaction_refs);
		if (!redaction_error.ok()) {
			return redaction_error;
		}
	}
	return {};
}

Error ValidateReferences(const Json &document, ManifestMode mode, const std::unordered_set<std::string> &entry_ids,
			 const std::vector<std::string> &redaction_refs)
{
	std::unordered_set<std::string> secret_ids;
	if (mode == ManifestMode::Local) {
		if (!document.contains("secret_refs") || !document.at("secret_refs").is_array()) {
			return Schema("secret_refs");
		}
		for (const Json &secret : document.at("secret_refs")) {
			if (!secret.is_object() || !IsObjectWithString(secret, "id")) {
				return Schema("id");
			}
			const std::string id = secret.at("id").get<std::string>();
			if (!secret_ids.insert(id).second) {
				return Schema("id");
			}
			if (!IsObjectWithString(secret, "entry")) {
				return Schema("entry");
			}
			if (!entry_ids.count(secret.at("entry").get<std::string>())) {
				return Schema("entry");
			}
		}

		for (const std::string &ref : redaction_refs) {
			if (!secret_ids.count(ref)) {
				return Schema("ref");
			}
		}
	}

	if (!document.contains("entries") || !document.at("entries").is_array()) {
		return Schema("entries");
	}
	for (const Json &entry : document.at("entries")) {
		if (entry.contains("bak") &&
		    (!entry_ids.count(entry.at("bak").get<std::string>()) || entry.at("bak") == entry.at("id"))) {
			return Schema("bak");
		}
	}
	return {};
}

Error ValidateExportObjects(const Json &document, ManifestMode mode)
{
	if (mode != ManifestMode::Export) {
		return {};
	}

	const Json &target = document.at("target");
	for (const auto &[key, ignored] : target.items()) {
		if (key != "os" && key != "audio") {
			return Schema("target");
		}
	}
	if (target.contains("audio")) {
		if (!target.at("audio").is_object()) {
			return Schema("target");
		}
		for (const auto &[key, ignored] : target.at("audio").items()) {
			if (key != "sample_rate" && key != "channels") {
				return Schema("target");
			}
		}
	}
	for (const Json &app_key : document.at("app_keys")) {
		if (app_key.is_object() && app_key.contains("restore") && app_key.at("restore").is_string() &&
		    app_key.at("restore").get<std::string>() == "never") {
			return Schema("app_keys");
		}
	}
	return {};
}

ManifestResult ValidateDocument(Json document, ManifestMode mode)
{
	if (!document.is_object()) {
		return {std::nullopt, Schema("manifest")};
	}
	for (const char *field : {"format", "major", "minor", "compat", "created_utc", "creator", "active", "target",
				  "app_keys", "entries", "dropped_ext"}) {
		if (!document.contains(field)) {
			return {std::nullopt, Schema(field)};
		}
	}
	if (!document.at("format").is_string() || document.at("format").get<std::string>() != "jeiobs-workspace") {
		return {std::nullopt, Schema("format")};
	}
	std::uint64_t major = 0U;
	if (!IsInteger(document.at("major")) || !GetUnsigned(document.at("major"), major) || major < 1U) {
		return {std::nullopt, Schema("major")};
	}
	std::uint64_t minor = 0U;
	if (!IsInteger(document.at("minor")) || !GetUnsigned(document.at("minor"), minor)) {
		return {std::nullopt, Schema("minor")};
	}
	if (!document.at("created_utc").is_string()) {
		return {std::nullopt, Schema("created_utc")};
	}
	for (const char *field : {"creator", "active"}) {
		if (!document.at(field).is_object()) {
			return {std::nullopt, Schema(field)};
		}
		const Error string_error =
			std::string_view(field) == "creator"
				? ValidateKnownStringFields(document.at(field), field,
							    {"app", "version", "commit", "obs_base"})
				: ValidateKnownStringFields(document.at(field), field, {"profile", "collection"});
		if (!string_error.ok()) {
			return {std::nullopt, string_error};
		}
	}
	if (!document.at("target").is_object()) {
		return {std::nullopt, Schema("target")};
	}
	if (!IsObjectWithString(document.at("target"), "os")) {
		return {std::nullopt, Schema("target.os")};
	}
	if (!document.at("app_keys").is_array()) {
		return {std::nullopt, Schema("app_keys")};
	}
	if (!document.at("dropped_ext").is_array()) {
		return {std::nullopt, Schema("dropped_ext")};
	}
	const Error audio_error = ValidateTargetAudio(document, mode);
	if (!audio_error.ok()) {
		return {std::nullopt, audio_error};
	}
	const Error app_keys_error = ValidateAppKeys(document);
	if (!app_keys_error.ok()) {
		return {std::nullopt, app_keys_error};
	}
	const Error dropped_ext_error = ValidateDroppedExtensions(document);
	if (!dropped_ext_error.ok()) {
		return {std::nullopt, dropped_ext_error};
	}
	const Error export_object_error = ValidateExportObjects(document, mode);
	if (!export_object_error.ok()) {
		return {std::nullopt, export_object_error};
	}
	if (document.contains("label") && !document.at("label").is_string()) {
		return {std::nullopt, Schema("label")};
	}

	bool ro_compat = false;
	const Error compat_error = ValidateCompat(document, ro_compat);
	if (!compat_error.ok()) {
		return {std::nullopt, compat_error};
	}

	if (mode == ManifestMode::Local) {
		for (const char *field : {"workspace_id", "seq", "parent", "secret_refs"}) {
			if (!document.contains(field)) {
				return {std::nullopt, Schema(field)};
			}
		}
		if (!document.at("workspace_id").is_string()) {
			return {std::nullopt, Schema("workspace_id")};
		}
		std::uint64_t sequence = 0U;
		if (!IsInteger(document.at("seq")) || !GetUnsigned(document.at("seq"), sequence) || sequence < 1U) {
			return {std::nullopt, Schema("seq")};
		}
		if (!document.at("parent").is_null() && !IsHash(document.at("parent"))) {
			return {std::nullopt, Schema("parent")};
		}
		for (const char *field : {"origin", "exported_utc"}) {
			if (document.contains(field)) {
				return {std::nullopt, Schema(field)};
			}
		}
	} else {
		if (!document.contains("origin") || !document.at("origin").is_object()) {
			return {std::nullopt, Schema("origin")};
		}
		const Json &origin = document.at("origin");
		if (!IsObjectWithString(origin, "workspace_id")) {
			return {std::nullopt, Schema("workspace_id")};
		}
		if (!origin.contains("revision_id") || !IsHash(origin.at("revision_id"))) {
			return {std::nullopt, Schema("revision_id")};
		}
		if (!document.contains("exported_utc") || !document.at("exported_utc").is_string()) {
			return {std::nullopt, Schema("exported_utc")};
		}
		for (const char *field : {"workspace_id", "seq", "parent", "secret_refs"}) {
			if (document.contains(field)) {
				return {std::nullopt, Schema(field)};
			}
		}
	}

	bool unchecked_paths = false;
	std::unordered_set<std::string> entry_ids;
	std::vector<std::string> redaction_refs;
	std::uint64_t stored_bytes = 0U;
	const Error entries_error =
		ValidateEntries(document, mode, unchecked_paths, entry_ids, redaction_refs, stored_bytes);
	if (!entries_error.ok()) {
		return {std::nullopt, entries_error};
	}
	const Error reference_error = ValidateReferences(document, mode, entry_ids, redaction_refs);
	if (!reference_error.ok()) {
		return {std::nullopt, reference_error};
	}

	Manifest manifest;
	manifest.document = std::move(document);
	manifest.mode = mode;
	manifest.readonly = major != 1U;
	manifest.roCompat = ro_compat;
	manifest.uncheckedPaths = unchecked_paths;
	return {std::move(manifest), {}};
}

bool IsKnownTopLevel(std::string_view key)
{
	static constexpr std::array<std::string_view, 18U> fields = {
		"format",   "major",       "minor",       "compat",      "workspace_id", "seq",
		"parent",   "created_utc", "label",       "creator",     "active",       "target",
		"app_keys", "entries",     "secret_refs", "dropped_ext", "origin",       "exported_utc",
	};
	return std::find(fields.begin(), fields.end(), key) != fields.end();
}

bool IsKnownEntry(std::string_view key)
{
	static constexpr std::array<std::string_view, 17U> fields = {
		"id",       "status",     "root",       "path",  "role", "profile",   "collection", "size",   "hash",
		"fidelity", "redactions", "unreviewed", "parse", "bak",  "load_rule", "kind",       "reason",
	};
	return std::find(fields.begin(), fields.end(), key) != fields.end();
}

std::vector<std::string> UnknownFields(const Json &value, bool entry)
{
	std::vector<std::string> fields;
	if (!value.is_object()) {
		return fields;
	}
	for (const auto &[key, ignored] : value.items()) {
		if ((entry && !IsKnownEntry(key)) || (!entry && !IsKnownTopLevel(key))) {
			fields.push_back(key);
		}
	}
	std::sort(fields.begin(), fields.end());
	return fields;
}

void AppendUnknownObjectFields(const Json &object, std::string_view prefix,
			       const std::initializer_list<std::string_view> &known_fields,
			       std::vector<std::string> &fields)
{
	if (!object.is_object()) {
		return;
	}
	for (const auto &[key, ignored] : object.items()) {
		if (std::find(known_fields.begin(), known_fields.end(), key) == known_fields.end()) {
			std::string field_name(prefix);
			field_name += '.';
			field_name += key;
			fields.push_back(std::move(field_name));
		}
	}
}

std::vector<std::string> ExportUnknownFields(const Json &document)
{
	std::vector<std::string> fields;
	for (const auto &[key, ignored] : document.items()) {
		if (!IsKnownTopLevel(key)) {
			fields.push_back(key);
		}
	}
	AppendUnknownObjectFields(document.at("creator"), "creator", {"app", "version", "commit", "obs_base"}, fields);
	AppendUnknownObjectFields(document.at("active"), "active", {"profile", "collection"}, fields);
	AppendUnknownObjectFields(document.at("compat"), "compat", {"incompat", "ro_compat"}, fields);

	const Json &target = document.at("target");
	// Known local fields that the projection drops on purpose are not "unknown" (dropped_ext is
	// only for fields this version does not know).
	AppendUnknownObjectFields(target, "target",
				  {"os", "arch", "machine_id", "app_dir", "roots", "audio", "devices"}, fields);
	if (target.contains("audio")) {
		if (target.at("audio").is_object()) {
			AppendUnknownObjectFields(target.at("audio"), "target.audio",
						  {"sample_rate", "channels", "explicit", "src"}, fields);
		} else {
			fields.emplace_back("target.audio");
		}
	}

	for (const Json &app_key : document.at("app_keys")) {
		if (!app_key.is_object()) {
			fields.emplace_back("app_keys");
			continue;
		}
		AppendUnknownObjectFields(app_key, "app_keys", {"root", "file", "section", "key", "value", "restore"},
					  fields);
	}

	std::sort(fields.begin(), fields.end());
	fields.erase(std::unique(fields.begin(), fields.end()), fields.end());
	return fields;
}

void AppendDropped(Json &dropped, const Json &entry, std::vector<std::string> fields)
{
	if (fields.empty()) {
		return;
	}
	Json record = Json::object();
	if (entry.is_null()) {
		record["entry"] = nullptr;
	} else {
		record["entry"] = entry;
	}
	record["fields"] = std::move(fields);
	dropped.push_back(std::move(record));
}

Json CopyDroppedExtensions(const Json &extensions)
{
	Json copied = Json::array();
	for (const Json &extension : extensions) {
		Json item = Json::object();
		item["entry"] = extension.at("entry");
		item["fields"] = extension.at("fields");
		copied.push_back(std::move(item));
	}
	return copied;
}

std::string EntryKey(const Json &entry)
{
	return entry.at("root").get<std::string>() + '\0' + entry.at("path").get<std::string>();
}

Json CopyExportRedactions(const Json &entry)
{
	Json redactions = Json::array();
	if (!entry.contains("redactions")) {
		return redactions;
	}
	for (const Json &redaction : entry.at("redactions")) {
		Json copy = Json::object();
		copy["pointer"] = redaction.at("pointer");
		copy["kind"] = redaction.at("kind");
		redactions.push_back(std::move(copy));
	}
	return redactions;
}

Json CopyKnownEntry(const Json &entry)
{
	Json result = Json::object();
	for (const char *field : {"id", "status", "root", "path", "role", "profile", "collection", "size", "hash",
				  "fidelity", "parse", "bak", "load_rule", "kind", "reason"}) {
		if (entry.contains(field)) {
			result[field] = entry.at(field);
		}
	}
	if (entry.contains("redactions")) {
		result["redactions"] = CopyExportRedactions(entry);
	}
	return result;
}

Error ValidateDecisionObject(const Json &decision, std::string &action, std::string &fidelity)
{
	if (!decision.is_object() || !decision.contains("decision") || !decision.at("decision").is_string()) {
		return Schema("decisions");
	}
	action = decision.at("decision").get<std::string>();
	if (action != "include" && action != "omit") {
		return Schema("decisions");
	}
	for (const auto &[key, ignored] : decision.items()) {
		if (key != "decision" && !(action == "include" && key == "fidelity")) {
			return Schema("decisions");
		}
	}
	if (action == "include") {
		if (decision.contains("fidelity")) {
			if (!decision.at("fidelity").is_string()) {
				return Schema("decisions");
			}
			fidelity = decision.at("fidelity").get<std::string>();
			if (fidelity != "exact" && fidelity != "redacted") {
				return Schema("decisions");
			}
		}
	}
	return {};
}

bool IsLeapYear(int year)
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

bool IsValidNow(std::string_view now)
{
	if (now.size() != 20U || now[4] != '-' || now[7] != '-' || now[10] != 'T' || now[13] != ':' || now[16] != ':' ||
	    now[19] != 'Z') {
		return false;
	}
	for (std::size_t index = 0U; index < now.size(); ++index) {
		if (index == 4U || index == 7U || index == 10U || index == 13U || index == 16U || index == 19U) {
			continue;
		}
		if (now[index] < '0' || now[index] > '9') {
			return false;
		}
	}

	const int year = (now[0] - '0') * 1000 + (now[1] - '0') * 100 + (now[2] - '0') * 10 + (now[3] - '0');
	const int month = (now[5] - '0') * 10 + (now[6] - '0');
	const int day = (now[8] - '0') * 10 + (now[9] - '0');
	const int hour = (now[11] - '0') * 10 + (now[12] - '0');
	const int minute = (now[14] - '0') * 10 + (now[15] - '0');
	const int second = (now[17] - '0') * 10 + (now[18] - '0');
	if (year == 0 || month < 1 || month > 12 || hour > 23 || minute > 59 || second > 59) {
		return false;
	}

	const int days_in_month[] = {31, IsLeapYear(year) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return day >= 1 && day <= days_in_month[month - 1];
}

} // namespace

JsonResult ParseJson(std::string_view bytes)
{
	try {
		return ParseJsonInternal(bytes);
	} catch (...) {
		return {std::nullopt, MakeError(ErrorCode::Json)};
	}
}

BytesResult ReadFile(const std::string &path)
{
	return ReadFileInternal(path);
}

ManifestResult ParseManifest(std::string_view bytes, ManifestMode mode)
{
	if (bytes.size() > Limits::kManifestBytes) {
		return {std::nullopt, MakeError(ErrorCode::Limit, "manifest")};
	}
	const JsonResult parsed = ParseJson(bytes);
	if (!parsed.ok()) {
		return {std::nullopt, parsed.error};
	}
	return ValidateDocument(*parsed.value, mode);
}

ManifestResult LoadManifest(const std::string &path, ManifestMode mode)
{
	const BytesResult bytes = ReadFile(path);
	if (!bytes.ok()) {
		return {std::nullopt, bytes.error};
	}
	return ParseManifest(bytes.bytes, mode);
}

JsonResult CarryOver(const Manifest &parent, const Manifest &draft)
{
	if (parent.mode != ManifestMode::Local || draft.mode != ManifestMode::Local) {
		return {std::nullopt, Schema("manifest")};
	}
	if (parent.readonly) {
		return {std::nullopt, MakeError(ErrorCode::Major)};
	}
	if (parent.roCompat) {
		const Json &features = parent.document.at("compat").at("ro_compat");
		return {std::nullopt, MakeError(ErrorCode::RoCompat, features.front().get<std::string>())};
	}

	Json result = draft.document;
	for (const auto &[key, value] : parent.document.items()) {
		if (!IsKnownTopLevel(key) && !result.contains(key)) {
			result[key] = value;
		}
	}

	std::unordered_map<std::string, Json> parent_entries;
	for (const Json &entry : parent.document.at("entries")) {
		parent_entries.emplace(EntryKey(entry), entry);
	}
	Json &draft_entries = result["entries"];
	for (Json &entry : draft_entries) {
		const auto parent_iterator = parent_entries.find(EntryKey(entry));
		if (parent_iterator == parent_entries.end()) {
			continue;
		}
		const Json &parent_entry = parent_iterator->second;
		const std::vector<std::string> unknown_fields = UnknownFields(parent_entry, true);
		const bool same_hash = entry.contains("hash") && parent_entry.contains("hash") &&
				       entry.at("hash") == parent_entry.at("hash");
		if (same_hash) {
			for (const std::string &field : unknown_fields) {
				if (!entry.contains(field)) {
					entry[field] = parent_entry.at(field);
				}
			}
		} else {
			AppendDropped(result["dropped_ext"], entry.at("id"), unknown_fields);
		}
	}
	return {std::move(result), {}};
}

JsonResult ProjectForExport(const Manifest &manifest, std::string_view bytesHash, const Json &decisions,
			    std::string_view now)
{
	try {
		if (manifest.mode != ManifestMode::Local) {
			return {std::nullopt, Schema("manifest")};
		}
		if (manifest.readonly) {
			return {std::nullopt, MakeError(ErrorCode::Major)};
		}
		if (manifest.roCompat) {
			const Json &features = manifest.document.at("compat").at("ro_compat");
			return {std::nullopt, MakeError(ErrorCode::RoCompat, features.front().get<std::string>())};
		}
		if (bytesHash.size() != 64U || std::any_of(bytesHash.begin(), bytesHash.end(), [](char character) {
			    return !((character >= '0' && character <= '9') || (character >= 'a' && character <= 'f'));
		    })) {
			return {std::nullopt, MakeError(ErrorCode::Revid)};
		}
		if (!IsValidNow(now)) {
			return {std::nullopt, Schema("now")};
		}
		if (!decisions.is_object()) {
			return {std::nullopt, Schema("decisions")};
		}

		std::unordered_map<std::string, const Json *> entries_by_id;
		for (const Json &entry : manifest.document.at("entries")) {
			entries_by_id.emplace(entry.at("id").get<std::string>(), &entry);
		}
		for (const auto &[id, decision] : decisions.items()) {
			const auto iterator = entries_by_id.find(id);
			if (iterator == entries_by_id.end() || !iterator->second->contains("fidelity") ||
			    iterator->second->at("fidelity") != "opaque-unreviewed") {
				return {std::nullopt, Schema("decisions")};
			}
			std::string action;
			std::string fidelity;
			const Error decision_error = ValidateDecisionObject(decision, action, fidelity);
			if (!decision_error.ok()) {
				return {std::nullopt, decision_error};
			}
			if (action == "include") {
				const Json &entry = *iterator->second;
				const bool is_redacted = entry.contains("redactions") &&
							 !entry.at("redactions").empty();
				const std::string derived_fidelity = is_redacted ? "redacted" : "exact";
				if (!fidelity.empty() && fidelity != derived_fidelity) {
					return {std::nullopt, Schema("decisions")};
				}
			}
			if (iterator->second->contains("parse") && action == "include") {
				return {std::nullopt, Schema("decisions")};
			}
		}

		for (const Json &entry : manifest.document.at("entries")) {
			if (!entry.contains("fidelity") || entry.at("fidelity") != "opaque-unreviewed" ||
			    (entry.contains("parse") && entry.at("parse") == "invalid")) {
				continue;
			}
			if (!decisions.contains(entry.at("id").get<std::string>())) {
				return {std::nullopt,
					MakeError(ErrorCode::Undecided, entry.at("id").get<std::string>())};
			}
		}

		Json result = Json::object();
		for (const char *field : {"format", "major", "minor", "created_utc", "label"}) {
			if (manifest.document.contains(field)) {
				result[field] = manifest.document.at(field);
			}
		}
		result["compat"] = Json::object();
		for (const char *field : {"incompat", "ro_compat"}) {
			result["compat"][field] = manifest.document.at("compat").at(field);
		}
		result["creator"] = Json::object();
		for (const char *field : {"app", "version", "commit", "obs_base"}) {
			if (manifest.document.at("creator").contains(field)) {
				result["creator"][field] = manifest.document.at("creator").at(field);
			}
		}
		result["active"] = Json::object();
		for (const char *field : {"profile", "collection"}) {
			if (manifest.document.at("active").contains(field)) {
				result["active"][field] = manifest.document.at("active").at(field);
			}
		}
		result["origin"] = {
			{"workspace_id", manifest.document.at("workspace_id")},
			{"revision_id", std::string(bytesHash)},
		};
		result["exported_utc"] = std::string(now);

		Json target = Json::object();
		target["os"] = manifest.document.at("target").at("os");
		if (manifest.document.at("target").contains("audio") &&
		    manifest.document.at("target").at("audio").is_object()) {
			const Json &audio = manifest.document.at("target").at("audio");
			Json audio_copy = Json::object();
			for (const char *field : {"sample_rate", "channels"}) {
				if (audio.contains(field)) {
					audio_copy[field] = audio.at(field);
				}
			}
			target["audio"] = std::move(audio_copy);
		}
		result["target"] = std::move(target);

		Json app_keys = Json::array();
		for (const Json &app_key : manifest.document.at("app_keys")) {
			if (!app_key.is_object() || (app_key.contains("restore") && app_key.at("restore").is_string() &&
						     app_key.at("restore").get<std::string>() == "never")) {
				continue;
			}
			Json app_key_copy = Json::object();
			for (const char *field : {"root", "file", "section", "key", "value", "restore"}) {
				if (app_key.contains(field)) {
					app_key_copy[field] = app_key.at(field);
				}
			}
			app_keys.push_back(std::move(app_key_copy));
		}
		result["app_keys"] = std::move(app_keys);

		Json dropped_ext = CopyDroppedExtensions(manifest.document.at("dropped_ext"));
		AppendDropped(dropped_ext, nullptr, ExportUnknownFields(manifest.document));

		Json projected_entries = Json::array();
		for (const Json &entry : manifest.document.at("entries")) {
			const std::string id = entry.at("id").get<std::string>();
			AppendDropped(dropped_ext, entry.at("id"), UnknownFields(entry, true));
			if (entry.contains("parse") && entry.at("parse") == "invalid") {
				Json omitted = Json::object();
				for (const char *field :
				     {"id", "root", "path", "role", "profile", "collection", "size"}) {
					if (entry.contains(field)) {
						omitted[field] = entry.at(field);
					}
				}
				omitted["status"] = "omitted";
				omitted["reason"] = "unparseable";
				projected_entries.push_back(std::move(omitted));
				continue;
			}

			const bool opaque = entry.contains("fidelity") && entry.at("fidelity") == "opaque-unreviewed";
			if (opaque) {
				const Json &decision = decisions.at(id);
				std::string action;
				std::string fidelity;
				const Error decision_error = ValidateDecisionObject(decision, action, fidelity);
				if (!decision_error.ok()) {
					return {std::nullopt, decision_error};
				}
				if (action == "omit") {
					Json omitted = Json::object();
					for (const char *field :
					     {"id", "root", "path", "role", "profile", "collection", "size"}) {
						if (entry.contains(field)) {
							omitted[field] = entry.at(field);
						}
					}
					omitted["status"] = "omitted";
					omitted["reason"] = "unreviewed";
					projected_entries.push_back(std::move(omitted));
					continue;
				}

				Json included = CopyKnownEntry(entry);
				const bool is_redacted = entry.contains("redactions") &&
							 !entry.at("redactions").empty();
				included["fidelity"] = is_redacted ? "redacted" : "exact";
				included.erase("unreviewed");
				included["review"] = {
					{"rules", entry.at("unreviewed").at("rules")},
					{"decision", "include"},
				};
				projected_entries.push_back(std::move(included));
				continue;
			}

			Json projected = CopyKnownEntry(entry);
			projected_entries.push_back(std::move(projected));
		}
		result["entries"] = std::move(projected_entries);
		result["dropped_ext"] = std::move(dropped_ext);
		return {std::move(result), {}};
	} catch (...) {
		return {std::nullopt, MakeError(ErrorCode::Json)};
	}
}

} // namespace Jeiobs::Workspace
