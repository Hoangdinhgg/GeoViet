#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>

using namespace geode::prelude;

#include <string>
#include <matjson.hpp>

// ============================
// Include built-in translation
// ============================

static const char* GEO_VI_JSON = R"JSON(
#include "vi.json"
)JSON";

// parsed json
matjson::Value g_translations = matjson::Value::object();

// ============================
// Utility
// ============================

static inline std::string trim_copy(std::string s) {
    const char* ws = " \t\n\r\f\v";
    auto start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// ============================
// Load translation
// ============================

void loadTranslations() {
    auto parsed = matjson::parse(GEO_VI_JSON);

    if (!parsed) {
        log::error("GeoViet: failed to parse built-in vi.json");
        return;
    }

    g_translations = parsed.unwrap();

    size_t count = 0;
    if (g_translations.isObject()) {
        count = g_translations.asObject()->size();
    }

    log::info("GeoViet: Loaded {} translations (built-in)", count);
}

// ============================
// Translate function
// ============================

static std::string translate_string(const std::string& key) {
    if (!g_translations.isObject()) return key;

    // exact match
    auto it = g_translations.get(key);
    if (it && it->isString()) {
        return it->asString().unwrapOr(key);
    }

    // trimmed match
    auto trimmed = trim_copy(key);
    if (trimmed != key) {
        auto it2 = g_translations.get(trimmed);
        if (it2 && it2->isString()) {
            return it2->asString().unwrapOr(key);
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
