#include "pch.h"
#include "Localization.h"
#include "RE/I/INISettingCollection.h"
#include "RE/I/INIPrefSettingCollection.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <shlobj.h>

namespace EasyOutfit
{
    static std::string ExtractJsonString(const std::string& a_json, const std::string& a_key, const std::string& a_default)
    {
        auto pos = a_json.find("\"" + a_key + "\"");
        if (pos == std::string::npos) return a_default;
        pos = a_json.find(':', pos);
        if (pos == std::string::npos) return a_default;
        auto q1 = a_json.find('"', pos);
        if (q1 == std::string::npos) return a_default;
        auto q2 = a_json.find('"', q1 + 1);
        while (q2 != std::string::npos && a_json[q2 - 1] == '\\') {
            q2 = a_json.find('"', q2 + 1);
        }
        if (q2 == std::string::npos) return a_default;

        std::string val = a_json.substr(q1 + 1, q2 - q1 - 1);
        std::string res;
        for (size_t i = 0; i < val.size(); ++i) {
            if (val[i] == '\\' && i + 1 < val.size()) {
                char next = val[i + 1];
                if (next == '"') { res += '"'; ++i; continue; }
                if (next == '\\') { res += '\\'; ++i; continue; }
                if (next == 'n') { res += '\n'; ++i; continue; }
            }
            res += val[i];
        }
        return res;
    }

    Localization& Localization::GetSingleton()
    {
        static Localization s_instance;
        return s_instance;
    }

    const Strings& Localization::Get(const std::string& a_lang)
    {
        auto& inst = GetSingleton();
        if (!inst.m_loaded || (!a_lang.empty() && a_lang != inst.m_currentLangSetting)) {
            inst.Load(a_lang);
        }
        return inst.GetStrings();
    }

    std::string Localization::DetectLanguageSuffix()
    {
        std::string rawLang;

        try {
            if (auto* ini = RE::INISettingCollection::GetSingleton()) {
                if (auto* s = ini->GetSetting("sLanguage:General")) {
                    const char* v = s->GetString();
                    if (v && v[0]) rawLang = v;
                }
            }

            if (rawLang.empty()) {
                if (auto* iniPref = RE::INIPrefSettingCollection::GetSingleton()) {
                    if (auto* s = iniPref->GetSetting("sLanguage:General")) {
                        const char* v = s->GetString();
                        if (v && v[0]) rawLang = v;
                    }
                }
            }
        } catch (...) {}

        if (rawLang.empty()) {
            char docPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, docPath))) {
                auto p = std::filesystem::path(docPath) / "My Games" / "Skyrim Special Edition" / "Skyrim.ini";
                char buf[64] = { 0 };
                GetPrivateProfileStringA("General", "sLanguage", "", buf, sizeof(buf), p.string().c_str());
                if (buf[0]) rawLang = buf;
            }
        }

        std::string upperLang = rawLang;
        std::transform(upperLang.begin(), upperLang.end(), upperLang.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });

        if (upperLang.find("RUS") != std::string::npos || upperLang == "RU") return "RU";
        if (upperLang.find("ENG") != std::string::npos || upperLang == "EN") return "EN";
        if (upperLang.find("GER") != std::string::npos || upperLang == "DE") return "DE";
        if (upperLang.find("FRE") != std::string::npos || upperLang == "FR") return "FR";
        if (upperLang.find("SPA") != std::string::npos || upperLang == "ES") return "ES";
        if (upperLang.find("POR") != std::string::npos || upperLang == "PT") return "PT";
        if (upperLang.find("POL") != std::string::npos || upperLang == "PL") return "PL";
        if (upperLang.find("CHI") != std::string::npos || upperLang == "ZH") return "ZH";
        if (upperLang.find("JAP") != std::string::npos || upperLang == "JA") return "JA";

        return upperLang.empty() ? "EN" : upperLang;
    }

    void Localization::SetEnglishDefaults()
    {
        m_storage.menuWardrobeTitle = "Wardrobe";
        m_storage.menuEditorTitle = "Outfit Editor";
        m_storage.menuSettingsTitle = "Settings";

        m_storage.activeOutfitLabel = "Current Outfit: ";
        m_storage.noOutfitActive = "[None - Normal Equipment]";
        m_storage.btnDisableOutfit = "Unequip Outfit";
        m_storage.btnApplyOutfit = "Equip";
        m_storage.btnCreateOutfit = "Create Outfit";
        m_storage.btnDeleteOutfit = "Delete";
        m_storage.btnRenameOutfit = "Rename";
        m_storage.btnDuplicateOutfit = "Duplicate";
        m_storage.btnEditOutfit = "Edit";
        m_storage.btnEditingOutfit = "[Editing]";
        m_storage.newOutfitPlaceholder = "New outfit name...";
        m_storage.outfitCountFormat = "Total Outfits: %zu";
        m_storage.noOutfitsYet = "- No outfits created yet -";

        m_storage.editingOutfitLabel = "Editing Outfit: ";
        m_storage.itemsInOutfitHeader = "Items in this Outfit:";
        m_storage.noOutfitSelectedForEdit = "Please select or create an outfit first.";
        m_storage.slotEmpty = "- Empty -";
        m_storage.btnRemoveFromSlot = "Remove";
        m_storage.addFromInvHeader = "Add from Player Inventory";
        m_storage.searchAllArmorHeader = "Browse / Search All Game Armors";
        m_storage.searchPlaceholder = "Search by name...";
        m_storage.btnAddItem = "Add";
        m_storage.btnClearSearch = "Clear";
        m_storage.noItemsFound = "- No matching items found -";

        m_storage.settingsHeader = "General Settings";
        m_storage.langHeader = "Interface Language:";
        m_storage.masterEnabled = "Enable EasyOutfit System";
        m_storage.hotkeyInfo = "Toggle Hotkey: [%s] (Code: %u)";
        m_storage.untransformFix = "Suppress outfit in Werewolf / Vampire Lord form";
        m_storage.presetsHeader = "Presets Management";
        m_storage.btnSavePresets = "Save All to Presets";
        m_storage.btnReloadPresets = "Reload from Presets";

        m_storage.notifyEquipped = "EasyOutfit: Equipped '%s'";
        m_storage.notifyUnequipped = "EasyOutfit: Outfit unequipped";
        m_storage.notifyToggleDisabled = "EasyOutfit: Visuals disabled";
        m_storage.notifyToggleEnabled = "EasyOutfit: Visuals enabled";

        m_storage.statusActive = "[EQUIPPED / ACTIVE]";
        m_storage.statusInactive = "[NOT EQUIPPED]";
        m_storage.btnEquipThisOutfit = "Equip This Outfit";
        m_storage.btnUnequipThisOutfit = "Unequip Outfit";
        m_storage.filterAll = "All";
        m_storage.filterHead = "Head";
        m_storage.filterBody = "Body";
        m_storage.filterHands = "Hands";
        m_storage.filterFeet = "Feet";
        m_storage.filterShield = "Shield";
        m_storage.filterOther = "Other";
        m_storage.pageFormat = "Page %zu / %zu (%zu items)";
        m_storage.btnPrevPage = "< Prev";
        m_storage.btnNextPage = "Next >";
        m_storage.btnWearItem = "Equip";
    }

    void Localization::SetRussianDefaults()
    {
        m_storage.menuWardrobeTitle = "Гардероб";
        m_storage.menuEditorTitle = "Конструктор наряда";
        m_storage.menuSettingsTitle = "Настройки";

        m_storage.activeOutfitLabel = "Текущий наряд: ";
        m_storage.noOutfitActive = "[Без наряда - Обычная экипировка]";
        m_storage.btnDisableOutfit = "Снять наряд";
        m_storage.btnApplyOutfit = "Надеть";
        m_storage.btnCreateOutfit = "Создать наряд";
        m_storage.btnDeleteOutfit = "Удалить";
        m_storage.btnRenameOutfit = "Переименовать";
        m_storage.btnDuplicateOutfit = "Дублировать";
        m_storage.btnEditOutfit = "Редактировать";
        m_storage.btnEditingOutfit = "[В редакторе]";
        m_storage.newOutfitPlaceholder = "Название нового наряда...";
        m_storage.outfitCountFormat = "Всего нарядов: %zu";
        m_storage.noOutfitsYet = "- Нет созданных нарядов -";

        m_storage.editingOutfitLabel = "Редактирование наряда: ";
        m_storage.itemsInOutfitHeader = "Предметы в этом наряде:";
        m_storage.noOutfitSelectedForEdit = "Сначала выберите или создайте наряд в Гардеробе.";
        m_storage.slotEmpty = "- Пусто -";
        m_storage.btnRemoveFromSlot = "Снять";
        m_storage.addFromInvHeader = "Добавить из инвентаря игрока";
        m_storage.searchAllArmorHeader = "Каталог и поиск по всей броне игры";
        m_storage.searchPlaceholder = "Поиск по названию...";
        m_storage.btnAddItem = "Добавить";
        m_storage.btnClearSearch = "Сброс";
        m_storage.noItemsFound = "- Ничего не найдено -";

        m_storage.settingsHeader = "Основные настройки";
        m_storage.langHeader = "Язык интерфейса:";
        m_storage.masterEnabled = "Включить систему EasyOutfit";
        m_storage.hotkeyInfo = "Горячая клавиша: [%s] (Код: %u)";
        m_storage.untransformFix = "Отключать наряд в форме Зверя / Лорда-вампира";
        m_storage.presetsHeader = "Управление пресетами";
        m_storage.btnSavePresets = "Сохранить все пресеты";
        m_storage.btnReloadPresets = "Перезагрузить пресеты";

        m_storage.notifyEquipped = "EasyOutfit: Надет наряд '%s'";
        m_storage.notifyUnequipped = "EasyOutfit: Наряд снят";
        m_storage.notifyToggleDisabled = "EasyOutfit: Визуал отключен";
        m_storage.notifyToggleEnabled = "EasyOutfit: Визуал включен";

        m_storage.statusActive = "[ НАДЕТ / АКТИВЕН ]";
        m_storage.statusInactive = "[ НЕ НАДЕТ ]";
        m_storage.btnEquipThisOutfit = "Надеть этот наряд";
        m_storage.btnUnequipThisOutfit = "Снять наряд";
        m_storage.filterAll = "Все";
        m_storage.filterHead = "Шлем";
        m_storage.filterBody = "Кираса";
        m_storage.filterHands = "Перчатки";
        m_storage.filterFeet = "Сапоги";
        m_storage.filterShield = "Щит";
        m_storage.filterOther = "Прочее";
        m_storage.pageFormat = "Стр. %zu / %zu (%zu предм.)";
        m_storage.btnPrevPage = "< Начало";
        m_storage.btnNextPage = "Дальше >";
        m_storage.btnWearItem = "Надеть";
    }

    void Localization::UpdatePointers()
    {
        m_ptrs.menuWardrobeTitle = m_storage.menuWardrobeTitle.c_str();
        m_ptrs.menuEditorTitle = m_storage.menuEditorTitle.c_str();
        m_ptrs.menuSettingsTitle = m_storage.menuSettingsTitle.c_str();

        m_ptrs.activeOutfitLabel = m_storage.activeOutfitLabel.c_str();
        m_ptrs.noOutfitActive = m_storage.noOutfitActive.c_str();
        m_ptrs.btnDisableOutfit = m_storage.btnDisableOutfit.c_str();
        m_ptrs.btnApplyOutfit = m_storage.btnApplyOutfit.c_str();
        m_ptrs.btnCreateOutfit = m_storage.btnCreateOutfit.c_str();
        m_ptrs.btnDeleteOutfit = m_storage.btnDeleteOutfit.c_str();
        m_ptrs.btnRenameOutfit = m_storage.btnRenameOutfit.c_str();
        m_ptrs.btnDuplicateOutfit = m_storage.btnDuplicateOutfit.c_str();
        m_ptrs.btnEditOutfit = m_storage.btnEditOutfit.c_str();
        m_ptrs.btnEditingOutfit = m_storage.btnEditingOutfit.c_str();
        m_ptrs.newOutfitPlaceholder = m_storage.newOutfitPlaceholder.c_str();
        m_ptrs.outfitCountFormat = m_storage.outfitCountFormat.c_str();
        m_ptrs.noOutfitsYet = m_storage.noOutfitsYet.c_str();

        m_ptrs.editingOutfitLabel = m_storage.editingOutfitLabel.c_str();
        m_ptrs.itemsInOutfitHeader = m_storage.itemsInOutfitHeader.c_str();
        m_ptrs.noOutfitSelectedForEdit = m_storage.noOutfitSelectedForEdit.c_str();
        m_ptrs.slotEmpty = m_storage.slotEmpty.c_str();
        m_ptrs.btnRemoveFromSlot = m_storage.btnRemoveFromSlot.c_str();
        m_ptrs.addFromInvHeader = m_storage.addFromInvHeader.c_str();
        m_ptrs.searchAllArmorHeader = m_storage.searchAllArmorHeader.c_str();
        m_ptrs.searchPlaceholder = m_storage.searchPlaceholder.c_str();
        m_ptrs.btnAddItem = m_storage.btnAddItem.c_str();
        m_ptrs.btnClearSearch = m_storage.btnClearSearch.c_str();
        m_ptrs.noItemsFound = m_storage.noItemsFound.c_str();

        m_ptrs.settingsHeader = m_storage.settingsHeader.c_str();
        m_ptrs.langHeader = m_storage.langHeader.c_str();
        m_ptrs.masterEnabled = m_storage.masterEnabled.c_str();
        m_ptrs.hotkeyInfo = m_storage.hotkeyInfo.c_str();
        m_ptrs.untransformFix = m_storage.untransformFix.c_str();
        m_ptrs.presetsHeader = m_storage.presetsHeader.c_str();
        m_ptrs.btnSavePresets = m_storage.btnSavePresets.c_str();
        m_ptrs.btnReloadPresets = m_storage.btnReloadPresets.c_str();

        m_ptrs.notifyEquipped = m_storage.notifyEquipped.c_str();
        m_ptrs.notifyUnequipped = m_storage.notifyUnequipped.c_str();
        m_ptrs.notifyToggleDisabled = m_storage.notifyToggleDisabled.c_str();
        m_ptrs.notifyToggleEnabled = m_storage.notifyToggleEnabled.c_str();

        m_ptrs.statusActive = m_storage.statusActive.c_str();
        m_ptrs.statusInactive = m_storage.statusInactive.c_str();
        m_ptrs.btnEquipThisOutfit = m_storage.btnEquipThisOutfit.c_str();
        m_ptrs.btnUnequipThisOutfit = m_storage.btnUnequipThisOutfit.c_str();
        m_ptrs.filterAll = m_storage.filterAll.c_str();
        m_ptrs.filterHead = m_storage.filterHead.c_str();
        m_ptrs.filterBody = m_storage.filterBody.c_str();
        m_ptrs.filterHands = m_storage.filterHands.c_str();
        m_ptrs.filterFeet = m_storage.filterFeet.c_str();
        m_ptrs.filterShield = m_storage.filterShield.c_str();
        m_ptrs.filterOther = m_storage.filterOther.c_str();
        m_ptrs.pageFormat = m_storage.pageFormat.c_str();
        m_ptrs.btnPrevPage = m_storage.btnPrevPage.c_str();
        m_ptrs.btnNextPage = m_storage.btnNextPage.c_str();
        m_ptrs.btnWearItem = m_storage.btnWearItem.c_str();
    }

    void Localization::LoadFromFile(const std::filesystem::path& a_path)
    {
        std::ifstream file(a_path);
        if (!file.is_open()) return;

        std::stringstream buf;
        buf << file.rdbuf();
        std::string json = buf.str();
        file.close();

        m_storage.menuWardrobeTitle = ExtractJsonString(json, "Menu.WardrobeTitle", m_storage.menuWardrobeTitle);
        m_storage.menuEditorTitle = ExtractJsonString(json, "Menu.EditorTitle", m_storage.menuEditorTitle);
        m_storage.menuSettingsTitle = ExtractJsonString(json, "Menu.SettingsTitle", m_storage.menuSettingsTitle);

        m_storage.activeOutfitLabel = ExtractJsonString(json, "Wardrobe.ActiveOutfitLabel", m_storage.activeOutfitLabel);
        m_storage.noOutfitActive = ExtractJsonString(json, "Wardrobe.NoOutfitActive", m_storage.noOutfitActive);
        m_storage.btnDisableOutfit = ExtractJsonString(json, "Wardrobe.BtnDisableOutfit", m_storage.btnDisableOutfit);
        m_storage.btnApplyOutfit = ExtractJsonString(json, "Wardrobe.BtnApplyOutfit", m_storage.btnApplyOutfit);
        m_storage.btnCreateOutfit = ExtractJsonString(json, "Wardrobe.BtnCreateOutfit", m_storage.btnCreateOutfit);
        m_storage.btnDeleteOutfit = ExtractJsonString(json, "Wardrobe.BtnDeleteOutfit", m_storage.btnDeleteOutfit);
        m_storage.btnRenameOutfit = ExtractJsonString(json, "Wardrobe.BtnRenameOutfit", m_storage.btnRenameOutfit);
        m_storage.btnDuplicateOutfit = ExtractJsonString(json, "Wardrobe.BtnDuplicateOutfit", m_storage.btnDuplicateOutfit);
        m_storage.btnEditOutfit = ExtractJsonString(json, "Wardrobe.BtnEditOutfit", m_storage.btnEditOutfit);
        m_storage.btnEditingOutfit = ExtractJsonString(json, "Wardrobe.BtnEditingOutfit", m_storage.btnEditingOutfit);
        m_storage.newOutfitPlaceholder = ExtractJsonString(json, "Wardrobe.NewOutfitPlaceholder", m_storage.newOutfitPlaceholder);
        m_storage.outfitCountFormat = ExtractJsonString(json, "Wardrobe.OutfitCountFormat", m_storage.outfitCountFormat);
        m_storage.noOutfitsYet = ExtractJsonString(json, "Wardrobe.NoOutfitsYet", m_storage.noOutfitsYet);

        m_storage.editingOutfitLabel = ExtractJsonString(json, "Editor.EditingOutfitLabel", m_storage.editingOutfitLabel);
        m_storage.itemsInOutfitHeader = ExtractJsonString(json, "Editor.ItemsInOutfitHeader", m_storage.itemsInOutfitHeader);
        m_storage.noOutfitSelectedForEdit = ExtractJsonString(json, "Editor.NoOutfitSelected", m_storage.noOutfitSelectedForEdit);
        m_storage.slotEmpty = ExtractJsonString(json, "Editor.SlotEmpty", m_storage.slotEmpty);
        m_storage.btnRemoveFromSlot = ExtractJsonString(json, "Editor.BtnRemoveFromSlot", m_storage.btnRemoveFromSlot);
        m_storage.addFromInvHeader = ExtractJsonString(json, "Editor.AddFromInvHeader", m_storage.addFromInvHeader);
        m_storage.searchAllArmorHeader = ExtractJsonString(json, "Editor.SearchAllArmorHeader", m_storage.searchAllArmorHeader);
        m_storage.searchPlaceholder = ExtractJsonString(json, "Editor.SearchPlaceholder", m_storage.searchPlaceholder);
        m_storage.btnAddItem = ExtractJsonString(json, "Editor.BtnAddItem", m_storage.btnAddItem);
        m_storage.btnClearSearch = ExtractJsonString(json, "Editor.BtnClearSearch", m_storage.btnClearSearch);
        m_storage.noItemsFound = ExtractJsonString(json, "Editor.NoItemsFound", m_storage.noItemsFound);

        m_storage.settingsHeader = ExtractJsonString(json, "Settings.Header", m_storage.settingsHeader);
        m_storage.langHeader = ExtractJsonString(json, "Settings.LangHeader", m_storage.langHeader);
        m_storage.masterEnabled = ExtractJsonString(json, "Settings.MasterEnabled", m_storage.masterEnabled);
        m_storage.hotkeyInfo = ExtractJsonString(json, "Settings.HotkeyInfo", m_storage.hotkeyInfo);
        m_storage.untransformFix = ExtractJsonString(json, "Settings.UntransformFix", m_storage.untransformFix);
        m_storage.presetsHeader = ExtractJsonString(json, "Settings.PresetsHeader", m_storage.presetsHeader);
        m_storage.btnSavePresets = ExtractJsonString(json, "Settings.BtnSavePresets", m_storage.btnSavePresets);
        m_storage.btnReloadPresets = ExtractJsonString(json, "Settings.BtnReloadPresets", m_storage.btnReloadPresets);

        m_storage.notifyEquipped = ExtractJsonString(json, "Notify.Equipped", m_storage.notifyEquipped);
        m_storage.notifyUnequipped = ExtractJsonString(json, "Notify.Unequipped", m_storage.notifyUnequipped);
        m_storage.notifyToggleDisabled = ExtractJsonString(json, "Notify.ToggleDisabled", m_storage.notifyToggleDisabled);
        m_storage.notifyToggleEnabled = ExtractJsonString(json, "Notify.ToggleEnabled", m_storage.notifyToggleEnabled);

        m_storage.statusActive = ExtractJsonString(json, "Control.StatusActive", m_storage.statusActive);
        m_storage.statusInactive = ExtractJsonString(json, "Control.StatusInactive", m_storage.statusInactive);
        m_storage.btnEquipThisOutfit = ExtractJsonString(json, "Control.BtnEquipThisOutfit", m_storage.btnEquipThisOutfit);
        m_storage.btnUnequipThisOutfit = ExtractJsonString(json, "Control.BtnUnequipThisOutfit", m_storage.btnUnequipThisOutfit);
        m_storage.filterAll = ExtractJsonString(json, "Filter.All", m_storage.filterAll);
        m_storage.filterHead = ExtractJsonString(json, "Filter.Head", m_storage.filterHead);
        m_storage.filterBody = ExtractJsonString(json, "Filter.Body", m_storage.filterBody);
        m_storage.filterHands = ExtractJsonString(json, "Filter.Hands", m_storage.filterHands);
        m_storage.filterFeet = ExtractJsonString(json, "Filter.Feet", m_storage.filterFeet);
        m_storage.filterShield = ExtractJsonString(json, "Filter.Shield", m_storage.filterShield);
        m_storage.filterOther = ExtractJsonString(json, "Filter.Other", m_storage.filterOther);
        m_storage.pageFormat = ExtractJsonString(json, "Page.Format", m_storage.pageFormat);
        m_storage.btnPrevPage = ExtractJsonString(json, "Page.BtnPrev", m_storage.btnPrevPage);
        m_storage.btnNextPage = ExtractJsonString(json, "Page.BtnNext", m_storage.btnNextPage);
        m_storage.btnWearItem = ExtractJsonString(json, "Btn.WearItem", m_storage.btnWearItem);

        UpdatePointers();
        Log("Loaded translation from: " + a_path.filename().string());
    }

    void Localization::Load(std::string_view a_override)
    {
        std::string suffix;
        if (!a_override.empty() && a_override != "auto") {
            suffix = a_override;
            std::transform(suffix.begin(), suffix.end(), suffix.begin(), [](unsigned char c) {
                return static_cast<char>(std::toupper(c));
            });
        } else {
            suffix = DetectLanguageSuffix();
        }

        m_currentSuffix = suffix;
        m_currentLangSetting = a_override.empty() ? "auto" : std::string(a_override);
        m_loaded = true;

        Log("Using language: " + suffix);

        if (suffix == "RU") {
            SetRussianDefaults();
        } else {
            SetEnglishDefaults();
        }
        UpdatePointers();

        auto pluginsDir = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins";
        auto langFile = pluginsDir / ("EasyOutfitStrings_" + suffix + ".json");

        if (std::filesystem::exists(langFile)) {
            LoadFromFile(langFile);
            return;
        }

        auto defaultFile = pluginsDir / "EasyOutfitStrings.json";
        if (std::filesystem::exists(defaultFile)) {
            LoadFromFile(defaultFile);
            return;
        }

        auto enFile = pluginsDir / "EasyOutfitStrings_EN.json";
        if (std::filesystem::exists(enFile)) {
            LoadFromFile(enFile);
            return;
        }

        Log("Using built-in defaults (" + std::string(suffix == "RU" ? "Russian" : "English") + ")");
    }
}
