#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>

using namespace geode::prelude;

#include <fstream>
#include <unordered_map>
#include <nlohmann/json.hpp>

std::unordered_map<std::string, std::string> g_translations;

void loadTranslations() {
    try {
        auto path = Mod::get()->getConfigDir() / "vi.json";

        std::ifstream file(path);
        if (!file.is_open()) {
            log::warn("GeoViet: vi.json not found");
            return;
        }

        nlohmann::json j;
        file >> j;

        for (auto& [key, value] : j.items()) {
            g_translations[key] = value.get<std::string>();
        }

        log::info("GeoViet: Loaded {} translations", g_translations.size());
    }
    catch (...) {
        log::error("GeoViet: Failed to load vi.json");
    }
}

class $modify(GeoVietLabel, CCLabelBMFont) {

    void setString(const char* text, bool needUpdateLabel = true) {
        std::string str = text ? text : "";

        auto it = g_translations.find(str);
        if (it != g_translations.end()) {
            str = it->second;
        }

        CCLabelBMFont::setString(str.c_str(), needUpdateLabel);
    }

};

$execute {
    loadTranslations();
}
