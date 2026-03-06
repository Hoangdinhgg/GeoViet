#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>

using namespace geode::prelude;

#include <string>
#include <matjson.hpp>
#include <sstream>
#include <fstream>
#include <filesystem>

// ============================
// Globals
// ============================

static matjson::Value g_translations = matjson::Value::object();

// ============================
// Utils
// ============================

static std::string trim_copy(const std::string& s) {
    const char* ws = " \t\n\r\f\v";
    auto start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

static bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), s.begin());
}

// ============================
// Load translations from resources/vi.json
// ============================

void loadTranslations() {
    try {
        auto path = Mod::get()->getResourcesDir() / "vi.json";

        if (!std::filesystem::exists(path)) {
            log::warn("GeoViet: resources/vi.json not found at {}", path.string());
            return;
        }

        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (!ifs.is_open()) {
            log::warn("GeoViet: cannot open vi.json");
            return;
        }

        std::ostringstream ss;
        ss << ifs.rdbuf();
        std::string content = ss.str();
        if (content.empty()) {
            log::warn("GeoViet: vi.json is empty");
            return;
        }

        auto parsed = matjson::parse(content);
        if (!parsed) {
            log::error("GeoViet: invalid JSON in vi.json");
            return;
        }

        g_translations = parsed.unwrap();

        // Avoid calling asObject() — just confirm it's an object
        if (!g_translations.isObject()) {
            log::error("GeoViet: vi.json root is not an object");
            g_translations = matjson::Value::object();
            return;
        }

        log::info("GeoViet: translations loaded (object present)");
    }
    catch (const std::exception& e) {
        log::error("GeoViet: exception loading vi.json: {}", e.what());
    }
    catch (...) {
        log::error("GeoViet: unknown exception loading vi.json");
    }
}

// ============================
// Translate logic
// - exact match
// - trimmed match
// - simple prefix rules (Attempt X -> Lần thử X)
// ============================

static std::string translate_string(const std::string& key) {
    if (!g_translations.isObject()) return key;

    // exact
    auto res = g_translations.get(key);
    if (res) {
        auto& v = res.unwrap();
        if (v.isString()) return v.asString().unwrapOr(key);
    }

    // trimmed
    auto t = trim_copy(key);
    if (t != key) {
        auto res2 = g_translations.get(t);
        if (res2) {
            auto& v = res2.unwrap();
            if (v.isString()) return v.asString().unwrapOr(key);
        }
    }

    // Simple pattern rules (common in GD)
    if (starts_with(key, "Attempt")) {
        std::string rest = key.substr(7); // after "Attempt"
        rest = trim_copy(rest);
        auto resPrefix = g_translations.get("Attempt");
        if (resPrefix) {
            auto& v = resPrefix.unwrap();
            if (v.isString()) {
                std::string prefix = v.asString().unwrapOr("Attempt");
                if (!rest.empty()) return prefix + " " + rest;
                return prefix;
            }
        }
    }

    return key;
}

// ============================
// Hook Label
// ============================

class $modify(GeoVietLabel, CCLabelBMFont) {
    void setString(const char* text, bool needUpdateLabel = true) {
        std::string str = text ? text : "";

        if (!str.empty()) {
            auto translated = translate_string(str);
            if (translated != str) {
                CCLabelBMFont::setString(translated.c_str(), needUpdateLabel);
                return;
            }
        }

        CCLabelBMFont::setString(str.c_str(), needUpdateLabel);
    }
};

// ============================
// Init
// ============================

$execute {
    loadTranslations();
}
