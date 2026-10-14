#pragma once
#include <string>
#include <filesystem>

namespace EasyOutfit
{
    struct Config
    {
        bool enabled{ true };
        bool suppressBeastForm{ true };
        uint32_t toggleKey{ 24 }; // 'O' key
        std::string toggleKeyName{ "O" };
        std::string language{ "auto" };
        std::string selectedOutfit{ "" };
    };

    class Settings
    {
    public:
        static Settings& GetSingleton()
        {
            static Settings instance;
            return instance;
        }

        void Load();
        void Save();

        Config& GetConfig() { return config_; }
        const Config& GetConfig() const { return config_; }

    private:
        Settings() = default;
        Config config_;
        std::filesystem::path configPath_;

        void FromJson(const std::string& a_json);
        std::string ToJson() const;
    };
}
