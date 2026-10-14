#include "pch.h"
#include "UI.h"
#include "SKSEMenuFramework_API.h"
#include "OutfitManager.h"
#include "Settings.h"
#include "Localization.h"
#include <vector>
#include <string>
#include <algorithm>

namespace EasyOutfit::UI
{
    static bool IsUtf8(std::string_view a_str)
    {
        size_t i = 0;
        while (i < a_str.size()) {
            unsigned char c = static_cast<unsigned char>(a_str[i]);
            if (c < 0x80) {
                i++;
            } else if ((c & 0xE0) == 0xC0) {
                if (i + 1 >= a_str.size()) return false;
                unsigned char c1 = static_cast<unsigned char>(a_str[i + 1]);
                if ((c1 & 0xC0) != 0x80) return false;
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                if (i + 2 >= a_str.size()) return false;
                unsigned char c1 = static_cast<unsigned char>(a_str[i + 1]);
                unsigned char c2 = static_cast<unsigned char>(a_str[i + 2]);
                if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return false;
                i += 3;
            } else if ((c & 0xF8) == 0xF0) {
                if (i + 3 >= a_str.size()) return false;
                unsigned char c1 = static_cast<unsigned char>(a_str[i + 1]);
                unsigned char c2 = static_cast<unsigned char>(a_str[i + 2]);
                unsigned char c3 = static_cast<unsigned char>(a_str[i + 3]);
                if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return false;
                i += 4;
            } else {
                return false;
            }
        }
        return true;
    }

    static std::string Cp1251ToUtf8(std::string_view a_str)
    {
        if (a_str.empty()) return {};
        int wlen = MultiByteToWideChar(1251, 0, a_str.data(), static_cast<int>(a_str.size()), nullptr, 0);
        if (wlen <= 0) return std::string(a_str);
        std::wstring wstr(wlen, L'\0');
        MultiByteToWideChar(1251, 0, a_str.data(), static_cast<int>(a_str.size()), wstr.data(), wlen);

        int u8len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wlen, nullptr, 0, nullptr, nullptr);
        if (u8len <= 0) return std::string(a_str);
        std::string u8str(u8len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wlen, u8str.data(), u8len, nullptr, nullptr);
        return u8str;
    }

    static std::string EnsureUtf8(std::string_view a_str)
    {
        if (IsUtf8(a_str)) {
            return std::string(a_str);
        }
        return Cp1251ToUtf8(a_str);
    }

    static std::string TruncateUtf8(std::string_view a_str, size_t a_maxChars)
    {
        size_t charCount = 0;
        size_t byteIdx = 0;
        while (byteIdx < a_str.size()) {
            if (charCount >= a_maxChars) {
                return std::string(a_str.substr(0, byteIdx)) + "..";
            }
            unsigned char c = static_cast<unsigned char>(a_str[byteIdx]);
            if (c < 0x80) {
                byteIdx += 1;
            } else if ((c & 0xE0) == 0xC0) {
                byteIdx += 2;
            } else if ((c & 0xF0) == 0xE0) {
                byteIdx += 3;
            } else if ((c & 0xF8) == 0xF0) {
                byteIdx += 4;
            } else {
                byteIdx += 1;
            }
            charCount++;
        }
        return std::string(a_str);
    }

    static std::string ToLowerUtf8(std::string_view a_str)
    {
        std::string result;
        result.reserve(a_str.size());
        for (size_t i = 0; i < a_str.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(a_str[i]);
            if (c >= 'A' && c <= 'Z') {
                result.push_back(static_cast<char>(c + 32));
            } else if (c == 0xD0 && i + 1 < a_str.size()) {
                unsigned char c2 = static_cast<unsigned char>(a_str[i + 1]);
                if (c2 >= 0x90 && c2 <= 0x9F) { // А - П -> а - п
                    result.push_back(static_cast<char>(0xD0));
                    result.push_back(static_cast<char>(c2 + 0x20));
                    i++;
                } else if (c2 >= 0xA0 && c2 <= 0xAF) { // Р - Я -> р - я
                    result.push_back(static_cast<char>(0xD1));
                    result.push_back(static_cast<char>(c2 - 0x20));
                    i++;
                } else if (c2 == 0x81) { // Ё -> ё
                    result.push_back(static_cast<char>(0xD1));
                    result.push_back(static_cast<char>(0x91));
                    i++;
                } else {
                    result.push_back(static_cast<char>(c));
                }
            } else {
                result.push_back(static_cast<char>(c));
            }
        }
        return result;
    }

    enum class SlotFilter {
        All = 0,
        Head,
        Body,
        Hands,
        Feet,
        Shield,
        Other
    };

    static bool MatchesSlotFilter(uint32_t a_mask, SlotFilter a_filter)
    {
        switch (a_filter) {
        case SlotFilter::All: return true;
        case SlotFilter::Head:
            return (a_mask & (static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHead) |
                              static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHair))) != 0;
        case SlotFilter::Body:
            return (a_mask & static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kBody)) != 0;
        case SlotFilter::Hands:
            return (a_mask & static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHands)) != 0;
        case SlotFilter::Feet:
            return (a_mask & static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet)) != 0;
        case SlotFilter::Shield:
            return (a_mask & static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kShield)) != 0;
        case SlotFilter::Other:
            return (a_mask & ~(static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHead) |
                               static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHair) |
                               static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kBody) |
                               static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kHands) |
                               static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet) |
                               static_cast<uint32_t>(RE::BGSBipedObjectForm::BipedObjectSlot::kShield))) != 0;
        }
        return true;
    }

    void RenderWardrobe()
    {
        auto& mgr = OutfitManager::GetSingleton();
        auto& cfg = Settings::GetSingleton().GetConfig();
        const auto& str = Localization::Get(cfg.language);

        // Active Outfit Banner
        const std::string& activeName = mgr.GetActiveOutfitName();
        SKSEMenuFramework::igText(str.activeOutfitLabel);
        SKSEMenuFramework::igSameLine(0.0f, 5.0f);
        if (activeName.empty()) {
            SKSEMenuFramework::igText(str.noOutfitActive);
        } else {
            char actBuf[128];
            snprintf(actBuf, sizeof(actBuf), "[%s]", activeName.c_str());
            SKSEMenuFramework::igText(actBuf);

            SKSEMenuFramework::igSameLine(0.0f, 15.0f);
            if (SKSEMenuFramework::igButton(str.btnDisableOutfit, { 0.0f, 0.0f })) {
                mgr.SetActiveOutfit("");
            }
        }

        SKSEMenuFramework::igSeparator();

        // Create New Outfit
        static char s_newOutfitBuf[64] = "";
        SKSEMenuFramework::igText(str.btnCreateOutfit);
        SKSEMenuFramework::igSameLine(0.0f, 10.0f);
        if (SKSEMenuFramework::igSetNextItemWidth) {
            SKSEMenuFramework::igSetNextItemWidth(200.0f);
        }
        if (SKSEMenuFramework::igInputTextWithHint) {
            SKSEMenuFramework::igInputTextWithHint("##NewOutfit", str.newOutfitPlaceholder, s_newOutfitBuf, sizeof(s_newOutfitBuf), 0, nullptr, nullptr);
        } else if (SKSEMenuFramework::igInputText) {
            SKSEMenuFramework::igInputText("##NewOutfit", s_newOutfitBuf, sizeof(s_newOutfitBuf), 0, nullptr, nullptr);
        }
        SKSEMenuFramework::igSameLine(0.0f, 5.0f);
        if (SKSEMenuFramework::igButton(str.btnCreateOutfit, { 0.0f, 0.0f })) {
            if (s_newOutfitBuf[0] != '\0') {
                mgr.CreateOutfit(s_newOutfitBuf);
                s_newOutfitBuf[0] = '\0';
            }
        }

        SKSEMenuFramework::igSeparator();

        // List of Outfits
        const auto& outfits = mgr.GetAllOutfits();
        char countBuf[64];
        snprintf(countBuf, sizeof(countBuf), str.outfitCountFormat, outfits.size());
        SKSEMenuFramework::igText(countBuf);
        SKSEMenuFramework::igSeparator();

        if (outfits.empty()) {
            SKSEMenuFramework::igText(str.noOutfitsYet);
            return;
        }

        auto outfitNames = mgr.GetOutfitNames();
        std::string toDelete = "";
        std::string toDuplicate = "";

        for (const auto& name : outfitNames) {
            auto* outfit = mgr.GetOutfit(name);
            if (!outfit) continue;

            bool isActive = (name == activeName);
            bool isEditing = (name == mgr.GetEditingOutfitName());

            char lineBuf[128];
            snprintf(lineBuf, sizeof(lineBuf), "%s%s (%zu)", 
                isActive ? "* " : "  ", TruncateUtf8(name, 20).c_str(), outfit->armors.size());
            SKSEMenuFramework::igText(lineBuf);

            // Action Buttons
            SKSEMenuFramework::igSameLine(300.0f, 0.0f);
            if (isActive) {
                if (SKSEMenuFramework::igButton((std::string(str.btnDisableOutfit) + "##" + name).c_str(), { 0.0f, 0.0f })) {
                    mgr.SetActiveOutfit("");
                }
            } else {
                if (SKSEMenuFramework::igButton((std::string(str.btnApplyOutfit) + "##" + name).c_str(), { 0.0f, 0.0f })) {
                    mgr.SetActiveOutfit(name);
                }
            }

            SKSEMenuFramework::igSameLine(0.0f, 5.0f);
            const char* editBtnText = isEditing ? str.btnEditingOutfit : str.btnEditOutfit;
            if (SKSEMenuFramework::igButton((std::string(editBtnText) + "##" + name).c_str(), { 0.0f, 0.0f })) {
                mgr.SetEditingOutfit(name);
            }

            SKSEMenuFramework::igSameLine(0.0f, 5.0f);
            if (SKSEMenuFramework::igButton((std::string(str.btnDuplicateOutfit) + "##" + name).c_str(), { 0.0f, 0.0f })) {
                toDuplicate = name;
            }

            SKSEMenuFramework::igSameLine(0.0f, 5.0f);
            if (SKSEMenuFramework::igButton((std::string(str.btnDeleteOutfit) + "##" + name).c_str(), { 0.0f, 0.0f })) {
                toDelete = name;
            }
        }

        if (!toDuplicate.empty()) {
            std::string copySuffix = (cfg.language == "en") ? " Copy" : " (\xd0\x9a\xd0\xbe\xd0\xbf\xd0\xb8\xd1\x8f)";
            std::string copyName = toDuplicate + copySuffix;
            int copyIdx = 2;
            while (mgr.GetOutfit(copyName) != nullptr) {
                copyName = toDuplicate + copySuffix + " " + std::to_string(copyIdx++);
            }
            mgr.DuplicateOutfit(toDuplicate, copyName);
        }

        if (!toDelete.empty()) {
            mgr.DeleteOutfit(toDelete);
        }
    }

    void RenderEditor()
    {
        auto& mgr = OutfitManager::GetSingleton();
        auto& cfg = Settings::GetSingleton().GetConfig();
        const auto& str = Localization::Get(cfg.language);

        auto outfitNames = mgr.GetOutfitNames();
        if (outfitNames.empty()) {
            std::string defName = (cfg.language == "en") ? "Outfit 1" : "\xd0\x9d\xd0\xb0\xd1\x80\xd1\x8f\xd0\xb4 1";
            mgr.CreateOutfit(defName);
            mgr.SetActiveOutfit(defName);
            outfitNames = mgr.GetOutfitNames();
        }

        auto* outfit = mgr.GetEditingOutfit();
        if (!outfit) {
            if (!outfitNames.empty()) {
                mgr.SetEditingOutfit(outfitNames.front());
                outfit = mgr.GetEditingOutfit();
            }
        }

        if (!outfit) {
            SKSEMenuFramework::igText(str.noOutfitSelectedForEdit);
            return;
        }

        // 1. Outfit Selector and Quick Equip Header
        const std::string& activeName = mgr.GetActiveOutfitName();
        bool isCurrentActive = (outfit->name == activeName);

        // Navigation between outfits right inside the editor
        SKSEMenuFramework::igText(str.editingOutfitLabel);
        SKSEMenuFramework::igSameLine(0.0f, 5.0f);

        int curIdx = 0;
        for (size_t i = 0; i < outfitNames.size(); ++i) {
            if (outfitNames[i] == outfit->name) {
                curIdx = static_cast<int>(i);
                break;
            }
        }

        if (SKSEMenuFramework::igButton("<##PrevOutfit", { 0.0f, 0.0f })) {
            int prevIdx = (curIdx > 0) ? (curIdx - 1) : static_cast<int>(outfitNames.size() - 1);
            mgr.SetEditingOutfit(outfitNames[prevIdx]);
        }
        SKSEMenuFramework::igSameLine(0.0f, 5.0f);

        char nameBadge[128];
        snprintf(nameBadge, sizeof(nameBadge), "[ %s ]", outfit->name.c_str());
        SKSEMenuFramework::igText(nameBadge);

        SKSEMenuFramework::igSameLine(0.0f, 5.0f);
        if (SKSEMenuFramework::igButton(">##NextOutfit", { 0.0f, 0.0f })) {
            int nextIdx = (curIdx + 1 < static_cast<int>(outfitNames.size())) ? (curIdx + 1) : 0;
            mgr.SetEditingOutfit(outfitNames[nextIdx]);
        }

        // Active Status & Toggle button
        SKSEMenuFramework::igSameLine(0.0f, 20.0f);
        if (isCurrentActive) {
            SKSEMenuFramework::igText(str.statusActive);
            SKSEMenuFramework::igSameLine(0.0f, 10.0f);
            if (SKSEMenuFramework::igButton(str.btnUnequipThisOutfit, { 0.0f, 0.0f })) {
                mgr.SetActiveOutfit("");
            }
        } else {
            SKSEMenuFramework::igText(str.statusInactive);
            SKSEMenuFramework::igSameLine(0.0f, 10.0f);
            if (SKSEMenuFramework::igButton(str.btnEquipThisOutfit, { 0.0f, 0.0f })) {
                mgr.SetActiveOutfit(outfit->name);
            }
        }

        SKSEMenuFramework::igSeparator();

        // 2. Current Pieces in this Outfit
        SKSEMenuFramework::igText(str.itemsInOutfitHeader);

        RE::TESObjectARMO* toRemove = nullptr;
        if (outfit->armors.empty()) {
            SKSEMenuFramework::igText(str.slotEmpty);
        } else {
            for (auto* armor : outfit->armors) {
                if (!armor) continue;

                const char* rawName = armor->GetName();
                std::string displayName = (rawName && rawName[0]) ? EnsureUtf8(rawName) : "Armor";
                displayName = TruncateUtf8(displayName, 55);

                char rowBuf[160];
                snprintf(rowBuf, sizeof(rowBuf), "%s", displayName.c_str());
                SKSEMenuFramework::igText(rowBuf);

                SKSEMenuFramework::igSameLine(520.0f, 0.0f);
                char btnBuf[64];
                snprintf(btnBuf, sizeof(btnBuf), "%s##%08X", str.btnRemoveFromSlot, armor->GetFormID());
                if (SKSEMenuFramework::igButton(btnBuf, { 0.0f, 0.0f })) {
                    toRemove = armor;
                }
            }
        }

        if (toRemove) {
            mgr.RemoveArmorFromOutfit(outfit->name, toRemove);
        }

        SKSEMenuFramework::igSeparator();

        // 3. Add from Player Inventory
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player) {
            SKSEMenuFramework::igText(str.addFromInvHeader);

            auto inv = player->GetInventory();
            std::vector<RE::TESObjectARMO*> invArmors;

            for (const auto& [item, itemData] : inv) {
                if (!item || itemData.first <= 0) continue;
                if (item->GetFormType() == RE::FormType::Armor) {
                    auto* armo = item->As<RE::TESObjectARMO>();
                    if (armo && outfit->armors.find(armo) == outfit->armors.end()) {
                        invArmors.push_back(armo);
                    }
                }
            }

            if (invArmors.empty()) {
                SKSEMenuFramework::igText(str.slotEmpty);
            } else {
                for (auto* armo : invArmors) {
                    const char* rawName = armo->GetName();
                    std::string dname = (rawName && rawName[0]) ? EnsureUtf8(rawName) : "Armor";
                    dname = TruncateUtf8(dname, 55);

                    char itemBuf[160];
                    snprintf(itemBuf, sizeof(itemBuf), "%s", dname.c_str());
                    SKSEMenuFramework::igText(itemBuf);

                    SKSEMenuFramework::igSameLine(520.0f, 0.0f);
                    char btnBuf[64];
                    snprintf(btnBuf, sizeof(btnBuf), "%s##Inv%08X", str.btnWearItem, armo->GetFormID());
                    if (SKSEMenuFramework::igButton(btnBuf, { 0.0f, 0.0f })) {
                        mgr.AddArmorToOutfit(outfit->name, armo);
                    }
                }
            }
        }

        SKSEMenuFramework::igSeparator();

        // 4. Browse / Search All Game Armors
        static SlotFilter s_slotFilter = SlotFilter::All;
        static char s_searchAllBuf[64] = "";
        static std::string s_lastSearch = "";
        static SlotFilter s_lastSlotFilter = SlotFilter::All;
        static size_t s_searchPage = 0;

        SKSEMenuFramework::igText(str.searchAllArmorHeader);

        // Slot filter buttons
        auto SlotFilterButton = [&](const char* label, SlotFilter f, bool sameLine = true) {
            bool active = (s_slotFilter == f);
            std::string btnText = active ? (std::string("[") + label + "]") : label;
            if (SKSEMenuFramework::igButton(btnText.c_str(), { 0.0f, 0.0f })) {
                s_slotFilter = f;
            }
            if (sameLine) {
                SKSEMenuFramework::igSameLine(0.0f, 4.0f);
            }
        };

        SlotFilterButton(str.filterAll, SlotFilter::All);
        SlotFilterButton(str.filterHead, SlotFilter::Head);
        SlotFilterButton(str.filterBody, SlotFilter::Body);
        SlotFilterButton(str.filterHands, SlotFilter::Hands);
        SlotFilterButton(str.filterFeet, SlotFilter::Feet);
        SlotFilterButton(str.filterShield, SlotFilter::Shield);
        SlotFilterButton(str.filterOther, SlotFilter::Other, false);

        // Search Input
        SKSEMenuFramework::igText(str.searchPlaceholder);
        SKSEMenuFramework::igSameLine(0.0f, 10.0f);
        if (SKSEMenuFramework::igSetNextItemWidth) {
            SKSEMenuFramework::igSetNextItemWidth(260.0f);
        }
        if (SKSEMenuFramework::igInputTextWithHint) {
            SKSEMenuFramework::igInputTextWithHint("##SearchAll", str.searchPlaceholder, s_searchAllBuf, sizeof(s_searchAllBuf), 0, nullptr, nullptr);
        } else if (SKSEMenuFramework::igInputText) {
            SKSEMenuFramework::igInputText("##SearchAll", s_searchAllBuf, sizeof(s_searchAllBuf), 0, nullptr, nullptr);
        }

        if (s_searchAllBuf[0] != '\0') {
            SKSEMenuFramework::igSameLine(0.0f, 6.0f);
            if (SKSEMenuFramework::igButton(str.btnClearSearch, { 0.0f, 0.0f })) {
                s_searchAllBuf[0] = '\0';
            }
        }

        if (s_lastSearch != s_searchAllBuf || s_lastSlotFilter != s_slotFilter) {
            s_lastSearch = s_searchAllBuf;
            s_lastSlotFilter = s_slotFilter;
            s_searchPage = 0;
        }

        // Collect matches
        std::string searchLower = ToLowerUtf8(s_searchAllBuf);
        const auto& allArmors = mgr.GetCachedGameArmors();
        std::vector<RE::TESObjectARMO*> matches;
        matches.reserve(allArmors.size());

        for (auto* armo : allArmors) {
            if (!armo) continue;
            if (outfit->armors.find(armo) != outfit->armors.end()) continue;

            uint32_t mask = static_cast<uint32_t>(armo->GetSlotMask());
            if (!MatchesSlotFilter(mask, s_slotFilter)) continue;

            const char* rawName = armo->GetName();
            if (!rawName || !rawName[0]) continue;

            if (!searchLower.empty()) {
                std::string u8Name = EnsureUtf8(rawName);
                if (ToLowerUtf8(u8Name).find(searchLower) == std::string::npos) {
                    continue;
                }
            }

            matches.push_back(armo);
        }

        if (matches.empty()) {
            SKSEMenuFramework::igText(str.noItemsFound);
        } else {
            constexpr size_t PAGE_SIZE = 12;
            size_t totalPages = (matches.size() + PAGE_SIZE - 1) / PAGE_SIZE;
            if (totalPages == 0) totalPages = 1;
            if (s_searchPage >= totalPages) s_searchPage = totalPages - 1;

            // Pagination Controls
            if (totalPages > 1) {
                if (s_searchPage > 0) {
                    if (SKSEMenuFramework::igButton(str.btnPrevPage, { 0.0f, 0.0f })) {
                        s_searchPage--;
                    }
                } else {
                    SKSEMenuFramework::igText(str.btnPrevPage);
                }

                SKSEMenuFramework::igSameLine(0.0f, 15.0f);
                char pageBuf[64];
                snprintf(pageBuf, sizeof(pageBuf), str.pageFormat, s_searchPage + 1, totalPages, matches.size());
                SKSEMenuFramework::igText(pageBuf);

                SKSEMenuFramework::igSameLine(0.0f, 15.0f);
                if (s_searchPage + 1 < totalPages) {
                    if (SKSEMenuFramework::igButton(str.btnNextPage, { 0.0f, 0.0f })) {
                        s_searchPage++;
                    }
                } else {
                    SKSEMenuFramework::igText(str.btnNextPage);
                }
                SKSEMenuFramework::igSeparator();
            }

            size_t startIdx = s_searchPage * PAGE_SIZE;
            size_t endIdx = std::min(matches.size(), startIdx + PAGE_SIZE);

            for (size_t i = startIdx; i < endIdx; ++i) {
                auto* match = matches[i];
                const char* rawName = match->GetName();
                std::string dname = (rawName && rawName[0]) ? EnsureUtf8(rawName) : "Armor";
                dname = TruncateUtf8(dname, 55);

                char mBuf[160];
                snprintf(mBuf, sizeof(mBuf), "%s", dname.c_str());
                SKSEMenuFramework::igText(mBuf);

                SKSEMenuFramework::igSameLine(520.0f, 0.0f);
                char btnBuf[64];
                snprintf(btnBuf, sizeof(btnBuf), "%s##Search%08X", str.btnWearItem, match->GetFormID());
                if (SKSEMenuFramework::igButton(btnBuf, { 0.0f, 0.0f })) {
                    mgr.AddArmorToOutfit(outfit->name, match);
                }
            }
        }
    }

    void RenderSettings()
    {
        auto& settings = Settings::GetSingleton();
        auto& cfg = settings.GetConfig();
        const auto& str = Localization::Get(cfg.language);
        bool changed = false;

        SKSEMenuFramework::igText(str.settingsHeader);
        SKSEMenuFramework::igSeparator();

        // Language
        SKSEMenuFramework::igText(str.langHeader);
        SKSEMenuFramework::igSameLine(0.0f, 10.0f);
        if (SKSEMenuFramework::igButton(cfg.language == "auto" ? "[Auto]" : "Auto", { 0.0f, 0.0f })) {
            cfg.language = "auto";
            changed = true;
        }
        SKSEMenuFramework::igSameLine(0.0f, 5.0f);
        if (SKSEMenuFramework::igButton(cfg.language == "ru" ? "[Русский]" : "Русский", { 0.0f, 0.0f })) {
            cfg.language = "ru";
            changed = true;
        }
        SKSEMenuFramework::igSameLine(0.0f, 5.0f);
        if (SKSEMenuFramework::igButton(cfg.language == "en" ? "[English]" : "English", { 0.0f, 0.0f })) {
            cfg.language = "en";
            changed = true;
        }

        SKSEMenuFramework::igSeparator();

        // Master Enable
        if (SKSEMenuFramework::igCheckbox(str.masterEnabled, &cfg.enabled)) {
            changed = true;
            OutfitManager::GetSingleton().RefreshPlayer();
        }

        // Beast Form Fix
        if (SKSEMenuFramework::igCheckbox(str.untransformFix, &cfg.suppressBeastForm)) {
            changed = true;
        }

        SKSEMenuFramework::igSeparator();

        char keyBuf[128];
        snprintf(keyBuf, sizeof(keyBuf), str.hotkeyInfo, cfg.toggleKeyName.c_str(), cfg.toggleKey);
        SKSEMenuFramework::igText(keyBuf);

        SKSEMenuFramework::igSeparator();

        SKSEMenuFramework::igText(str.presetsHeader);
        if (SKSEMenuFramework::igButton(str.btnSavePresets, { 0.0f, 0.0f })) {
            OutfitManager::GetSingleton().SavePresets();
        }
        SKSEMenuFramework::igSameLine(0.0f, 10.0f);
        if (SKSEMenuFramework::igButton(str.btnReloadPresets, { 0.0f, 0.0f })) {
            OutfitManager::GetSingleton().LoadPresets();
            OutfitManager::GetSingleton().RefreshPlayer();
        }

        if (changed) {
            settings.Save();
        }
    }

    void Register()
    {
        if (SKSEMenuFramework::AddSectionItem) {
            auto& cfg = Settings::GetSingleton().GetConfig();
            const auto& str = Localization::Get(cfg.language);

            std::string wTitle = std::string("EasyOutfit/") + str.menuWardrobeTitle;
            std::string eTitle = std::string("EasyOutfit/") + str.menuEditorTitle;
            std::string sTitle = std::string("EasyOutfit/") + str.menuSettingsTitle;

            SKSEMenuFramework::AddSectionItem(wTitle.c_str(), RenderWardrobe);
            SKSEMenuFramework::AddSectionItem(eTitle.c_str(), RenderEditor);
            SKSEMenuFramework::AddSectionItem(sTitle.c_str(), RenderSettings);
        }
    }
}
