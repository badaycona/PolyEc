#pragma once

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <regex>
#include <string>

namespace polyec {

struct ModelManifest {
    std::string family;
    std::string schema;
    std::string model_contract;
    std::string sha256;
    int observation = 0;
    int actions = 0;
    int hidden = 0;
    bool trained_with_rl = false;
    bool imitation = true;
    bool declares_model_contract = false;
};

inline bool read_model_manifest(const std::filesystem::path& model_path,
                                ModelManifest& manifest, std::string& error) {
    const auto manifest_path = model_path.parent_path() / "manifest.json";
    std::ifstream input(manifest_path, std::ios::binary);
    if (!input) {
        error = "Model manifest unavailable: " + manifest_path.string();
        return false;
    }
    const std::string json((std::istreambuf_iterator<char>(input)), {});
    auto string_field = [&](const char* key, std::string& value, bool required = true) {
        const std::regex pattern(std::string("\\\"") + key + "\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");
        std::smatch match;
        if (!std::regex_search(json, match, pattern)) return !required;
        value = match[1].str();
        return true;
    };
    auto integer_field = [&](const char* key, int& value) {
        const std::regex pattern(std::string("\\\"") + key + "\\\"\\s*:\\s*(-?[0-9]+)");
        std::smatch match;
        if (!std::regex_search(json, match, pattern)) return false;
        try { value = std::stoi(match[1].str()); } catch (...) { return false; }
        return true;
    };
    auto boolean_field = [&](const char* key, bool& value) {
        const std::regex pattern(std::string("\\\"") + key + "\\\"\\s*:\\s*(true|false)");
        std::smatch match;
        if (!std::regex_search(json, match, pattern)) return false;
        value = match[1].str() == "true";
        return true;
    };
    if (!string_field("family", manifest.family) ||
        !string_field("schema", manifest.schema) ||
        !string_field("sha256", manifest.sha256) ||
        !string_field("model_contract", manifest.model_contract, false) ||
        !integer_field("observation", manifest.observation) ||
        !integer_field("actions", manifest.actions) ||
        !integer_field("hidden", manifest.hidden) ||
        !boolean_field("trained_with_rl", manifest.trained_with_rl) ||
        !boolean_field("imitation", manifest.imitation)) {
        error = "Model manifest fields invalid: " + manifest_path.string();
        return false;
    }
    const std::regex contract_key("\\\"model_contract\\\"\\s*:");
    manifest.declares_model_contract = std::regex_search(json, contract_key);
    if (manifest.declares_model_contract && manifest.model_contract.empty()) {
        error = "Model manifest contract is empty: " + manifest_path.string();
        return false;
    }
    if (manifest.sha256.size() != 64 ||
        !std::all_of(manifest.sha256.begin(), manifest.sha256.end(), [](unsigned char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        })) {
        error = "Model manifest checksum invalid: " + manifest_path.string();
        return false;
    }
    error.clear();
    return true;
}

inline bool compatible_model_manifest(const ModelManifest& manifest,
                                      const std::string& expected_family,
                                      const std::string& expected_contract,
                                      bool allow_legacy_pilot,
                                      bool& legacy_pilot,
                                      std::string& error) {
    legacy_pilot = false;
    if (manifest.family != expected_family || manifest.schema != "realm-v31" ||
        manifest.observation != 3766 || manifest.actions != 282 || manifest.hidden != 64 ||
        !manifest.trained_with_rl || manifest.imitation) {
        error = "Model manifest contract mismatch for " + expected_family +
            ": requires realm-v31 with 3766 observation values, 282 actions, hidden size 64, and an RL-trained model";
        return false;
    }
    if (manifest.declares_model_contract) {
        if (manifest.model_contract != expected_contract) {
            error = "Model contract mismatch: " + expected_contract;
            return false;
        }
        error.clear();
        return true;
    }
    if (allow_legacy_pilot) {
        legacy_pilot = true;
        error.clear();
        return true;
    }
    error = "Model contract mismatch: " + expected_contract;
    return false;
}

} // namespace polyec


