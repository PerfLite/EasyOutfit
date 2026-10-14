#include "pch.h"
#include "Settings.h"
#include <sstream>

namespace EasyOutfit
{
    static std::string ExtractBool(const std::string& a_json, const std::string& a_key, bool a_default)
    {
        auto pos = a_json.find("\"" + a_key + "\"");
        if (pos == std::string::npos) return a_default ? "true" : "false";
        pos = a_json.find(':', pos);
        if (pos == std::string::npos) return a_default ? "true" : "false";

        auto end = a_json.find_first_of(",}\n", pos);
        std::string val = a_json.substr(pos + 1, end - pos - 1);
        return val.find("true") != std::string::npos ? "true" : "false";
    }

    static uint32_t ExtractUInt(const std::string& a_json, const std::string& a_key, uint32_t a_default)
    {
        auto pos = a_json.find("\"" + a_key + "\"");
        if (pos == std::string::npos) return a_default;
        pos = a_json.find(':', pos);
        if (pos == std::string::npos) return a_default;

        auto end = a_json.find_first_of(",}\n", pos);
        try {
            return static_cast<uint32_t>(std::stoul(a_json.substr(pos + 1, end - pos - 1)));
        } catch (...) {
            return a_default;
        }
    }

    static std::string ExtractString(const std::string& a_json, const std::string& a_key, const std::string& a_default)
    {
        auto pos = a_json.find("\"" + a_key + "\"");
        if (pos == std::string::npos) return a_default;
        pos = a_json.find(':', pos);
        if (pos == std::string::npos) return a_default;

        auto firstQuote = a_json.find('"', pos);
        if (firstQuote == std::string::npos) return a_default;
        auto secondQuote = a_json.find('"', firstQuote + 1);
        if (secondQuote == std::string::npos) return a_default;

        return a_json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    }

    void Settings::Load()
    {
        configPath_ = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "EasyOutfit.json";

        if (!std::filesystem::exists(configPath_)) {
            Save();
            return;
        }

        try {
            std::ifstream file(configPath_);
            if (!file.is_open()) return;

            std::stringstream buffer;
            buffer << file.rdbuf();
            FromJson(buffer.str());
        } catch (...) {
        }
    }

    void Settings::Save()
    {
        try {
            if (configPath_.empty()) {
                configPath_ = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "EasyOutfit.json";
            }
            std::filesystem::create_directories(configPath_.parent_path());

            std::ofstream file(configPath_);
            if (file.is_open()) {
                file << ToJson();
            }
        } catch (...) {
        }
    }

    void Settings::FromJson(const std::string& a_json)
    {
        config_.enabled = ExtractBool(a_json, "enabled", true) == "true";
        config_.suppressBeastForm = ExtractBool(a_json, "suppressBeastForm", true) == "true";
        config_.toggleKey = ExtractUInt(a_json, "toggleKey", 24);
        config_.toggleKeyName = ExtractString(a_json, "toggleKeyName", "O");
        config_.language = ExtractString(a_json, "language", "auto");
        config_.selectedOutfit = ExtractString(a_json, "selectedOutfit", "");
    }

    std::string Settings::ToJson() const
    {
        std::stringstream ss;
        ss << "{\n";
        ss << "  \"enabled\": " << (config_.enabled ? "true" : "false") << ",\n";
        ss << "  \"suppressBeastForm\": " << (config_.suppressBeastForm ? "true" : "false") << ",\n";
        ss << "  \"toggleKey\": " << config_.toggleKey << ",\n";
        ss << "  \"toggleKeyName\": \"" << config_.toggleKeyName << "\",\n";
        ss << "  \"language\": \"" << config_.language << "\",\n";
        ss << "  \"selectedOutfit\": \"" << config_.selectedOutfit << "\"\n";
        ss << "}\n";
        return ss.str();
    }
}
