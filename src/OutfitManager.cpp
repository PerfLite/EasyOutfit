#include "pch.h"
#include "OutfitManager.h"
#include "Settings.h"
#include <sstream>

namespace EasyOutfit
{
    bool Outfit::ConflictsWith(RE::TESObjectARMO* a_armor) const
    {
        if (!a_armor) return false;
        uint32_t mask = static_cast<uint32_t>(a_armor->GetSlotMask());
        for (auto* armo : armors) {
            if (armo && armo != a_armor) {
                uint32_t existingMask = static_cast<uint32_t>(armo->GetSlotMask());
                if ((mask & existingMask) != 0) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Outfit::HasShield() const
    {
        for (auto* armo : armors) {
            if (armo && armo->IsShield()) {
                return true;
            }
        }
        return false;
    }

    uint32_t Outfit::GetSlotMask() const
    {
        uint32_t mask = 0;
        for (auto* armo : armors) {
            if (armo) {
                mask |= static_cast<uint32_t>(armo->GetSlotMask());
            }
        }
        return mask;
    }

    void Outfit::RemoveConflicting(RE::TESObjectARMO* a_armor)
    {
        if (!a_armor) return;
        uint32_t mask = static_cast<uint32_t>(a_armor->GetSlotMask());
        std::vector<RE::TESObjectARMO*> toRemove;
        for (auto* armo : armors) {
            if (armo && armo != a_armor) {
                uint32_t existingMask = static_cast<uint32_t>(armo->GetSlotMask());
                if ((mask & existingMask) != 0) {
                    toRemove.push_back(armo);
                }
            }
        }
        for (auto* r : toRemove) {
            armors.erase(r);
        }
    }

    void OutfitManager::Init()
    {
        presetsPath_ = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "EasyOutfit_Presets.json";
        LoadPresets();

        // Restore active outfit from Settings
        const auto& savedActive = Settings::GetSingleton().GetConfig().selectedOutfit;
        if (!savedActive.empty() && outfits_.find(savedActive) != outfits_.end()) {
            activeOutfitName_ = savedActive;
        }

        // If no outfits exist yet, auto-create and activate default outfit
        if (outfits_.empty()) {
            std::string defaultName = (Settings::GetSingleton().GetConfig().language == "en") ? "Outfit 1" : "\xd0\x9d\xd0\xb0\xd1\x80\xd1\x8f\xd0\xb4 1";
            outfits_[defaultName] = Outfit{ defaultName, {} };
            activeOutfitName_ = defaultName;
            editingOutfitName_ = defaultName;
        }

        if (editingOutfitName_.empty() && !activeOutfitName_.empty()) {
            editingOutfitName_ = activeOutfitName_;
        } else if (editingOutfitName_.empty() && !outfits_.empty()) {
            editingOutfitName_ = outfits_.begin()->first;
        }

        CacheGameArmors();
    }

    bool OutfitManager::CreateOutfit(const std::string& a_name)
    {
        if (a_name.empty()) return false;
        if (outfits_.find(a_name) != outfits_.end()) return false;

        outfits_[a_name] = Outfit{ a_name, {} };
        editingOutfitName_ = a_name;
        SavePresets();
        return true;
    }

    bool OutfitManager::DeleteOutfit(const std::string& a_name)
    {
        auto it = outfits_.find(a_name);
        if (it == outfits_.end()) return false;

        if (activeOutfitName_ == a_name) {
            activeOutfitName_.clear();
            Settings::GetSingleton().GetConfig().selectedOutfit.clear();
            Settings::GetSingleton().Save();
            RefreshPlayer();
        }

        outfits_.erase(it);

        if (editingOutfitName_ == a_name) {
            editingOutfitName_ = outfits_.empty() ? "" : outfits_.begin()->first;
        }

        SavePresets();
        return true;
    }

    bool OutfitManager::RenameOutfit(const std::string& a_oldName, const std::string& a_newName)
    {
        if (a_newName.empty() || a_oldName == a_newName) return false;
        if (outfits_.find(a_newName) != outfits_.end()) return false;

        auto it = outfits_.find(a_oldName);
        if (it == outfits_.end()) return false;

        Outfit outfit = std::move(it->second);
        outfit.name = a_newName;
        outfits_.erase(it);
        outfits_[a_newName] = std::move(outfit);

        if (activeOutfitName_ == a_oldName) {
            activeOutfitName_ = a_newName;
            Settings::GetSingleton().GetConfig().selectedOutfit = a_newName;
            Settings::GetSingleton().Save();
        }
        if (editingOutfitName_ == a_oldName) {
            editingOutfitName_ = a_newName;
        }

        SavePresets();
        return true;
    }

    bool OutfitManager::DuplicateOutfit(const std::string& a_srcName, const std::string& a_newName)
    {
        if (a_newName.empty()) return false;
        if (outfits_.find(a_newName) != outfits_.end()) return false;

        auto* src = GetOutfit(a_srcName);
        if (!src) return false;

        Outfit copy;
        copy.name = a_newName;
        copy.armors = src->armors;
        outfits_[a_newName] = std::move(copy);
        editingOutfitName_ = a_newName;

        SavePresets();
        return true;
    }

    Outfit* OutfitManager::GetOutfit(const std::string& a_name)
    {
        auto it = outfits_.find(a_name);
        return it != outfits_.end() ? &it->second : nullptr;
    }

    const Outfit* OutfitManager::GetOutfit(const std::string& a_name) const
    {
        auto it = outfits_.find(a_name);
        return it != outfits_.end() ? &it->second : nullptr;
    }

    Outfit* OutfitManager::GetActiveOutfit()
    {
        return GetOutfit(activeOutfitName_);
    }

    const Outfit* OutfitManager::GetActiveOutfit() const
    {
        return GetOutfit(activeOutfitName_);
    }

    Outfit* OutfitManager::GetEditingOutfit()
    {
        return GetOutfit(editingOutfitName_);
    }

    void OutfitManager::SetActiveOutfit(const std::string& a_name)
    {
        if (a_name.empty()) {
            activeOutfitName_.clear();
        } else {
            if (outfits_.find(a_name) != outfits_.end()) {
                activeOutfitName_ = a_name;
            }
        }

        Settings::GetSingleton().GetConfig().selectedOutfit = activeOutfitName_;
        Settings::GetSingleton().Save();

        RefreshPlayer();
    }

    void OutfitManager::ToggleActiveOutfit()
    {
        if (activeOutfitName_.empty()) {
            if (!lastActiveBeforeToggle_.empty() && outfits_.find(lastActiveBeforeToggle_) != outfits_.end()) {
                SetActiveOutfit(lastActiveBeforeToggle_);
            } else if (!outfits_.empty()) {
                SetActiveOutfit(outfits_.begin()->first);
            }
        } else {
            lastActiveBeforeToggle_ = activeOutfitName_;
            SetActiveOutfit("");
        }
    }

    bool OutfitManager::AddArmorToOutfit(const std::string& a_outfitName, RE::TESObjectARMO* a_armor)
    {
        if (!a_armor) return false;
        auto* outfit = GetOutfit(a_outfitName);
        if (!outfit) return false;

        outfit->RemoveConflicting(a_armor);
        outfit->armors.insert(a_armor);

        SavePresets();
        if (activeOutfitName_.empty()) {
            SetActiveOutfit(a_outfitName);
        } else if (activeOutfitName_ == a_outfitName) {
            RefreshPlayer();
        }
        return true;
    }

    bool OutfitManager::RemoveArmorFromOutfit(const std::string& a_outfitName, RE::TESObjectARMO* a_armor)
    {
        if (!a_armor) return false;
        auto* outfit = GetOutfit(a_outfitName);
        if (!outfit) return false;

        outfit->armors.erase(a_armor);

        SavePresets();
        if (activeOutfitName_ == a_outfitName) {
            RefreshPlayer();
        }
        return true;
    }

    std::vector<std::string> OutfitManager::GetOutfitNames() const
    {
        std::vector<std::string> names;
        names.reserve(outfits_.size());
        for (const auto& [name, _] : outfits_) {
            names.push_back(name);
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    void OutfitManager::RefreshActor(RE::Actor* a_actor)
    {
        if (!a_actor) return;
        auto* pm = a_actor->GetActorRuntimeData().currentProcess;
        if (pm) {
            pm->Set3DUpdateFlag(static_cast<RE::RESET_3D_FLAGS>(
                std::to_underlying(RE::RESET_3D_FLAGS::kModel) | 
                std::to_underlying(RE::RESET_3D_FLAGS::kSkin) |
                std::to_underlying(RE::RESET_3D_FLAGS::kHead)
            ));
            pm->Update3DModel(a_actor);
        } else {
            a_actor->Update3DModel();
        }
    }

    void OutfitManager::RefreshPlayer()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player) {
            RefreshActor(player);
        }
    }

    void OutfitManager::CacheGameArmors()
    {
        cachedArmors_.clear();
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) return;

        const auto& armors = dataHandler->GetFormArray<RE::TESObjectARMO>();
        cachedArmors_.reserve(armors.size());

        for (auto* armor : armors) {
            if (!armor) continue;
            if (armor->templateArmor) continue; // skip pre-enchanted duplicates
            if (armor->formFlags & RE::TESObjectARMO::RecordFlags::kNonPlayable) continue;

            const char* name = armor->GetName();
            if (!name || !name[0]) continue;

            cachedArmors_.push_back(armor);
        }

        std::sort(cachedArmors_.begin(), cachedArmors_.end(), [](RE::TESObjectARMO* a, RE::TESObjectARMO* b) {
            const char* na = a ? a->GetName() : "";
            const char* nb = b ? b->GetName() : "";
            return std::string_view(na) < std::string_view(nb);
        });
    }

    namespace
    {
        static void LogDbg(const std::string& a_msg)
        {
            EasyOutfit::Log("[OutfitManager] " + a_msg);
        }

        static void SkipWs(const std::string& s, size_t& pos)
        {
            while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r' || s[pos] == '\n')) {
                ++pos;
            }
        }

        static std::string ParseJsonString(const std::string& s, size_t& pos)
        {
            SkipWs(s, pos);
            if (pos >= s.size() || s[pos] != '"') return "";
            ++pos; // skip '"'
            std::string result;
            while (pos < s.size() && s[pos] != '"') {
                if (s[pos] == '\\' && pos + 1 < s.size()) {
                    ++pos;
                    if (s[pos] == '"') result += '"';
                    else if (s[pos] == '\\') result += '\\';
                    else if (s[pos] == 'n') result += '\n';
                    else if (s[pos] == 'r') result += '\r';
                    else if (s[pos] == 't') result += '\t';
                    else result += s[pos];
                } else {
                    result += s[pos];
                }
                ++pos;
            }
            if (pos < s.size() && s[pos] == '"') {
                ++pos; // skip closing '"'
            }
            return result;
        }

        static uint32_t ParseJsonNumber(const std::string& s, size_t& pos)
        {
            SkipWs(s, pos);
            size_t start = pos;
            while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) {
                ++pos;
            }
            if (pos > start) {
                try {
                    return static_cast<uint32_t>(std::stoul(s.substr(start, pos - start)));
                } catch (...) {
                    return 0;
                }
            }
            return 0;
        }

        static std::string EscapeJson(const std::string& s)
        {
            std::string res;
            res.reserve(s.size() + 8);
            for (char c : s) {
                if (c == '"') res += "\\\"";
                else if (c == '\\') res += "\\\\";
                else if (c == '\n') res += "\\n";
                else if (c == '\r') res += "\\r";
                else if (c == '\t') res += "\\t";
                else res += c;
            }
            return res;
        }

        static RE::TESObjectARMO* LookupArmorForm(RE::TESDataHandler* a_handler, uint32_t a_localID, const std::string& a_plugin)
        {
            if (!a_handler || a_plugin.empty() || a_localID == 0) return nullptr;

            // 1. Try standard CommonLib LookupForm
            try {
                auto* form = a_handler->LookupForm<RE::TESObjectARMO>(a_localID, a_plugin);
                if (form) return form;
            } catch (...) {}

            // 2. Try looking up through loaded regular mod collection
            try {
                auto regularIndex = a_handler->GetLoadedModIndex(a_plugin);
                if (regularIndex.has_value()) {
                    uint32_t fullID = (static_cast<uint32_t>(*regularIndex) << 24) | (a_localID & 0x00FFFFFF);
                    auto* f = RE::TESForm::LookupByID<RE::TESObjectARMO>(fullID);
                    if (f) return f;
                }
            } catch (...) {}

            // 3. Try looking up through loaded light mod collection
            try {
                auto lightIndex = a_handler->GetLoadedLightModIndex(a_plugin);
                if (lightIndex.has_value()) {
                    uint32_t fullID = 0xFE000000 | (static_cast<uint32_t>(*lightIndex) << 12) | (a_localID & 0x00000FFF);
                    auto* f = RE::TESForm::LookupByID<RE::TESObjectARMO>(fullID);
                    if (f) return f;
                }
            } catch (...) {}

            // 4. Fallback: try raw ID directly if it's already a full FormID
            try {
                return RE::TESForm::LookupByID<RE::TESObjectARMO>(a_localID);
            } catch (...) {}

            return nullptr;
        }
    }

    void OutfitManager::SavePresets()
    {
        try {
            if (presetsPath_.empty()) {
                presetsPath_ = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "EasyOutfit_Presets.json";
            }
            std::filesystem::create_directories(presetsPath_.parent_path());

            std::ofstream file(presetsPath_);
            if (!file.is_open()) {
                LogDbg("ERROR: SavePresets failed to open file: " + presetsPath_.string());
                return;
            }

            file << "{\n  \"outfits\": [\n";
            size_t outfitIdx = 0;
            for (const auto& [name, outfit] : outfits_) {
                file << "    {\n";
                file << "      \"name\": \"" << EscapeJson(name) << "\",\n";
                file << "      \"items\": [\n";

                size_t itemIdx = 0;
                for (auto* armor : outfit.armors) {
                    if (!armor) continue;

                    auto* fileOwner = armor->GetFile(0);
                    std::string modName = fileOwner ? std::string(fileOwner->GetFilename()) : "";
                    uint32_t localID = armor->GetFormID() & 0x00FFFFFF;
                    if (fileOwner && fileOwner->IsLight()) {
                        localID = armor->GetFormID() & 0x00000FFF;
                    }

                    const char* armoName = armor->GetName();
                    std::string safeName = (armoName && armoName[0]) ? armoName : "Unnamed";

                    file << "        { \"plugin\": \"" << EscapeJson(modName) 
                         << "\", \"id\": " << localID
                         << ", \"name\": \"" << EscapeJson(safeName) << "\" }";
                    if (++itemIdx < outfit.armors.size()) file << ",";
                    file << "\n";
                }

                file << "      ]\n";
                file << "    }";
                if (++outfitIdx < outfits_.size()) file << ",";
                file << "\n";
            }
            file << "  ]\n}\n";
            LogDbg("Saved " + std::to_string(outfits_.size()) + " outfits to " + presetsPath_.string());
        } catch (const std::exception& e) {
            LogDbg(std::string("ERROR: SavePresets exception: ") + e.what());
        } catch (...) {
            LogDbg("ERROR: SavePresets unknown exception");
        }
    }

    void OutfitManager::LoadPresets()
    {
        if (presetsPath_.empty()) {
            presetsPath_ = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "EasyOutfit_Presets.json";
        }

        if (!std::filesystem::exists(presetsPath_)) {
            LogDbg("Presets file does not exist: " + presetsPath_.string());
            std::string defaultName = (Settings::GetSingleton().GetConfig().language == "en") ? "Outfit 1" : "\xd0\x9d\xd0\xb0\xd1\x80\xd1\x8f\xd0\xb4 1";
            outfits_[defaultName] = Outfit{ defaultName, {} };
            editingOutfitName_ = defaultName;
            return;
        }

        try {
            std::ifstream file(presetsPath_);
            if (!file.is_open()) {
                LogDbg("ERROR: Failed to open presets file for reading: " + presetsPath_.string());
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string json = buffer.str();
            LogDbg("Loaded presets file (" + std::to_string(json.size()) + " bytes)");

            auto* dataHandler = RE::TESDataHandler::GetSingleton();
            std::unordered_map<std::string, Outfit> loadedOutfits;

            size_t outfitsPos = json.find("\"outfits\"");
            size_t pos = (outfitsPos != std::string::npos) ? json.find('[', outfitsPos) : json.find('[');

            if (pos != std::string::npos) {
                ++pos; // skip '['
                while (pos < json.size()) {
                    SkipWs(json, pos);
                    if (pos >= json.size() || json[pos] == ']') break;
                    if (json[pos] == ',') { ++pos; continue; }
                    if (json[pos] != '{') { ++pos; continue; }
                    ++pos; // skip '{' (start of outfit object)

                    std::string outfitName;
                    std::vector<std::pair<std::string, uint32_t>> items;

                    while (pos < json.size()) {
                        SkipWs(json, pos);
                        if (pos >= json.size() || json[pos] == '}') {
                            if (pos < json.size()) ++pos;
                            break;
                        }
                        if (json[pos] == ',') { ++pos; continue; }

                        std::string key = ParseJsonString(json, pos);
                        SkipWs(json, pos);
                        if (pos < json.size() && json[pos] == ':') ++pos;
                        SkipWs(json, pos);

                        if (key == "name") {
                            outfitName = ParseJsonString(json, pos);
                        } else if (key == "items") {
                            if (pos < json.size() && json[pos] == '[') {
                                ++pos; // skip '['
                                while (pos < json.size()) {
                                    SkipWs(json, pos);
                                    if (pos >= json.size() || json[pos] == ']') {
                                        if (pos < json.size()) ++pos;
                                        break;
                                    }
                                    if (json[pos] == ',') { ++pos; continue; }
                                    if (json[pos] != '{') { ++pos; continue; }
                                    ++pos; // skip '{' (start of item object)

                                    std::string plugin;
                                    uint32_t localID = 0;

                                    while (pos < json.size()) {
                                        SkipWs(json, pos);
                                        if (pos >= json.size() || json[pos] == '}') {
                                            if (pos < json.size()) ++pos;
                                            break;
                                        }
                                        if (json[pos] == ',') { ++pos; continue; }

                                        std::string itemKey = ParseJsonString(json, pos);
                                        SkipWs(json, pos);
                                        if (pos < json.size() && json[pos] == ':') ++pos;
                                        SkipWs(json, pos);

                                        if (itemKey == "plugin") {
                                            plugin = ParseJsonString(json, pos);
                                        } else if (itemKey == "id") {
                                            localID = ParseJsonNumber(json, pos);
                                        } else if (itemKey == "name") {
                                            ParseJsonString(json, pos);
                                        } else {
                                            if (pos < json.size() && json[pos] == '"') {
                                                ParseJsonString(json, pos);
                                            } else {
                                                while (pos < json.size() && json[pos] != ',' && json[pos] != '}') ++pos;
                                            }
                                        }
                                    }

                                    if (!plugin.empty() && localID != 0) {
                                        items.push_back({ plugin, localID });
                                    }
                                }
                            }
                        } else {
                            if (pos < json.size() && json[pos] == '"') {
                                ParseJsonString(json, pos);
                            } else {
                                while (pos < json.size() && json[pos] != ',' && json[pos] != '}') ++pos;
                            }
                        }
                    }

                    if (!outfitName.empty()) {
                        Outfit outfit{ outfitName, {} };
                        for (const auto& [plugin, localID] : items) {
                            auto* armor = LookupArmorForm(dataHandler, localID, plugin);
                            if (armor) {
                                outfit.armors.insert(armor);
                            } else {
                                LogDbg("Warning: Could not resolve armor plugin=" + plugin + " id=" + std::to_string(localID));
                            }
                        }
                        LogDbg("Loaded outfit '" + outfitName + "' with " + std::to_string(outfit.armors.size()) + " items");
                        loadedOutfits[outfitName] = std::move(outfit);
                    }
                }
            }

            if (!loadedOutfits.empty()) {
                outfits_ = std::move(loadedOutfits);
                LogDbg("Successfully loaded " + std::to_string(outfits_.size()) + " outfits from file");
            } else {
                LogDbg("No valid outfits found in file or file was empty");
                if (outfits_.empty()) {
                    std::string defaultName = (Settings::GetSingleton().GetConfig().language == "en") ? "Outfit 1" : "\xd0\x9d\xd0\xb0\xd1\x80\xd1\x8f\xd0\xb4 1";
                    outfits_[defaultName] = Outfit{ defaultName, {} };
                    editingOutfitName_ = defaultName;
                }
            }
        } catch (const std::exception& e) {
            LogDbg(std::string("ERROR: LoadPresets exception: ") + e.what());
        } catch (...) {
            LogDbg("ERROR: LoadPresets unknown exception");
        }
    }
}
