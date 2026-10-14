#pragma once
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <filesystem>

namespace EasyOutfit
{
    struct SavedItem
    {
        std::string plugin;
        uint32_t localFormID{ 0 };
        std::string name;
    };

    struct Outfit
    {
        std::string name;
        std::set<RE::TESObjectARMO*> armors;

        bool ConflictsWith(RE::TESObjectARMO* a_armor) const;
        bool HasShield() const;
        uint32_t GetSlotMask() const;
        void RemoveConflicting(RE::TESObjectARMO* a_armor);
    };

    class OutfitManager
    {
    public:
        static OutfitManager& GetSingleton()
        {
            static OutfitManager instance;
            return instance;
        }

        void Init();

        // Outfit CRUD
        bool CreateOutfit(const std::string& a_name);
        bool DeleteOutfit(const std::string& a_name);
        bool RenameOutfit(const std::string& a_oldName, const std::string& a_newName);
        bool DuplicateOutfit(const std::string& a_srcName, const std::string& a_newName);

        Outfit* GetOutfit(const std::string& a_name);
        const Outfit* GetOutfit(const std::string& a_name) const;

        Outfit* GetActiveOutfit();
        const Outfit* GetActiveOutfit() const;

        void SetActiveOutfit(const std::string& a_name);
        void ToggleActiveOutfit();
        const std::string& GetActiveOutfitName() const { return activeOutfitName_; }

        // Editor
        void SetEditingOutfit(const std::string& a_name) { editingOutfitName_ = a_name; }
        Outfit* GetEditingOutfit();
        const std::string& GetEditingOutfitName() const { return editingOutfitName_; }

        bool AddArmorToOutfit(const std::string& a_outfitName, RE::TESObjectARMO* a_armor);
        bool RemoveArmorFromOutfit(const std::string& a_outfitName, RE::TESObjectARMO* a_armor);

        const std::unordered_map<std::string, Outfit>& GetAllOutfits() const { return outfits_; }
        std::vector<std::string> GetOutfitNames() const;

        // Player Refresh
        void RefreshPlayer();
        void RefreshActor(RE::Actor* a_actor);

        // Presets JSON
        void SavePresets();
        void LoadPresets();

        // Search cache for Game Armors
        void CacheGameArmors();
        const std::vector<RE::TESObjectARMO*>& GetCachedGameArmors() const { return cachedArmors_; }

    private:
        OutfitManager() = default;

        std::unordered_map<std::string, Outfit> outfits_;
        std::string activeOutfitName_{ "" };
        std::string editingOutfitName_{ "" };
        std::string lastActiveBeforeToggle_{ "" };

        std::vector<RE::TESObjectARMO*> cachedArmors_;
        std::filesystem::path presetsPath_;

        void ResolveArmorsForOutfit(Outfit& a_outfit, const std::vector<SavedItem>& a_savedItems);
    };
}
