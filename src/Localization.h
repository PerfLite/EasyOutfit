#pragma once
#include <string>
#include <string_view>
#include <filesystem>

namespace EasyOutfit
{
    struct Strings
    {
        // Menu Titles
        const char* menuWardrobeTitle;
        const char* menuEditorTitle;
        const char* menuSettingsTitle;

        // Wardrobe Tab
        const char* activeOutfitLabel;
        const char* noOutfitActive;
        const char* btnDisableOutfit;
        const char* btnApplyOutfit;
        const char* btnCreateOutfit;
        const char* btnDeleteOutfit;
        const char* btnRenameOutfit;
        const char* btnDuplicateOutfit;
        const char* btnEditOutfit;
        const char* btnEditingOutfit;
        const char* newOutfitPlaceholder;
        const char* outfitCountFormat;
        const char* noOutfitsYet;

        // Editor Tab
        const char* editingOutfitLabel;
        const char* itemsInOutfitHeader;
        const char* noOutfitSelectedForEdit;
        const char* slotEmpty;
        const char* btnRemoveFromSlot;
        const char* addFromInvHeader;
        const char* searchAllArmorHeader;
        const char* searchPlaceholder;
        const char* btnAddItem;
        const char* btnClearSearch;
        const char* noItemsFound;

        // Settings Tab
        const char* settingsHeader;
        const char* langHeader;
        const char* masterEnabled;
        const char* hotkeyInfo;
        const char* untransformFix;
        const char* presetsHeader;
        const char* btnSavePresets;
        const char* btnReloadPresets;

        // Notifications
        const char* notifyEquipped;
        const char* notifyUnequipped;
        const char* notifyToggleDisabled;
        const char* notifyToggleEnabled;

        // UI Controls & Navigation
        const char* statusActive;
        const char* statusInactive;
        const char* btnEquipThisOutfit;
        const char* btnUnequipThisOutfit;
        const char* filterAll;
        const char* filterHead;
        const char* filterBody;
        const char* filterHands;
        const char* filterFeet;
        const char* filterShield;
        const char* filterOther;
        const char* pageFormat;
        const char* btnPrevPage;
        const char* btnNextPage;
        const char* btnWearItem;
    };

    class Localization
    {
    public:
        static Localization& GetSingleton();

        void Load(std::string_view a_override = "");
        const Strings& GetStrings() const { return m_ptrs; }
        const std::string& GetCurrentSuffix() const { return m_currentSuffix; }

        static const Strings& Get(const std::string& a_lang = "");

    private:
        Localization() = default;
        ~Localization() = default;

        std::string DetectLanguageSuffix();
        void LoadFromFile(const std::filesystem::path& a_path);
        void SetRussianDefaults();
        void SetEnglishDefaults();
        void UpdatePointers();

        struct StringStorage
        {
            std::string menuWardrobeTitle;
            std::string menuEditorTitle;
            std::string menuSettingsTitle;

            std::string activeOutfitLabel;
            std::string noOutfitActive;
            std::string btnDisableOutfit;
            std::string btnApplyOutfit;
            std::string btnCreateOutfit;
            std::string btnDeleteOutfit;
            std::string btnRenameOutfit;
            std::string btnDuplicateOutfit;
            std::string btnEditOutfit;
            std::string btnEditingOutfit;
            std::string newOutfitPlaceholder;
            std::string outfitCountFormat;
            std::string noOutfitsYet;

            std::string editingOutfitLabel;
            std::string itemsInOutfitHeader;
            std::string noOutfitSelectedForEdit;
            std::string slotEmpty;
            std::string btnRemoveFromSlot;
            std::string addFromInvHeader;
            std::string searchAllArmorHeader;
            std::string searchPlaceholder;
            std::string btnAddItem;
            std::string btnClearSearch;
            std::string noItemsFound;

            std::string settingsHeader;
            std::string langHeader;
            std::string masterEnabled;
            std::string hotkeyInfo;
            std::string untransformFix;
            std::string presetsHeader;
            std::string btnSavePresets;
            std::string btnReloadPresets;

            std::string notifyEquipped;
            std::string notifyUnequipped;
            std::string notifyToggleDisabled;
            std::string notifyToggleEnabled;

            std::string statusActive;
            std::string statusInactive;
            std::string btnEquipThisOutfit;
            std::string btnUnequipThisOutfit;
            std::string filterAll;
            std::string filterHead;
            std::string filterBody;
            std::string filterHands;
            std::string filterFeet;
            std::string filterShield;
            std::string filterOther;
            std::string pageFormat;
            std::string btnPrevPage;
            std::string btnNextPage;
            std::string btnWearItem;
        };

        StringStorage m_storage;
        Strings m_ptrs{};
        std::string m_currentSuffix{ "EN" };
        std::string m_currentLangSetting{ "auto" };
        bool m_loaded{ false };
    };

    using I18n = Localization;
    inline const Strings& Tr() { return Localization::GetSingleton().GetStrings(); }
}
