#include "pch.h"
#include "Settings.h"
#include "OutfitManager.h"
#include "SkinningHooks.h"
#include "UI.h"
#include "Localization.h"
#include "SKSEMenuFramework_API.h"
#include <fstream>

namespace
{
    static void Dbg(std::string_view a_msg)
    {
        EasyOutfit::Log(a_msg);
    }

    class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* a_event,
            RE::BSTEventSource<RE::InputEvent*>*) override
        {
            if (!a_event) {
                return RE::BSEventNotifyControl::kContinue;
            }

            for (auto* event = *a_event; event; event = event->next) {
                auto* button = event->AsButtonEvent();
                if (!button || !button->IsDown()) continue;

                uint32_t code = button->GetIDCode();
                auto& settings = EasyOutfit::Settings::GetSingleton();
                auto& cfg = settings.GetConfig();

                if (code == cfg.toggleKey) {
                    auto& mgr = EasyOutfit::OutfitManager::GetSingleton();
                    mgr.ToggleActiveOutfit();

                    const auto& str = EasyOutfit::Localization::Get(cfg.language);
                    if (!mgr.GetActiveOutfitName().empty()) {
                        char buf[128];
                        snprintf(buf, sizeof(buf), str.notifyEquipped, mgr.GetActiveOutfitName().c_str());
                        RE::DebugNotification(buf);
                    } else {
                        RE::DebugNotification(str.notifyUnequipped);
                    }
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    static InputEventHandler g_inputHandler;

    void Setup()
    {
        Dbg("=== EasyOutfit Setup() Started ===");

        // 1. Settings
        auto& settings = EasyOutfit::Settings::GetSingleton();
        settings.Load();
        Dbg("Settings loaded successfully");

        // 1.1 Localization
        EasyOutfit::Localization::GetSingleton().Load(settings.GetConfig().language);
        Dbg("Localization loaded successfully");

        // 2. Outfit Manager
        auto& mgr = EasyOutfit::OutfitManager::GetSingleton();
        mgr.Init();
        Dbg("OutfitManager initialized");

        // 3. Engine Hooks
        try {
            EasyOutfit::Hooks::Install();
            Dbg("SkinningHooks installed successfully");
        } catch (const std::exception& e) {
            Dbg(std::string("FATAL: Failed to install SkinningHooks: ") + e.what());
        } catch (...) {
            Dbg("FATAL: Failed to install SkinningHooks (unknown exception)");
        }

        // 4. Input Handler
        auto* inputDevice = RE::BSInputDeviceManager::GetSingleton();
        if (inputDevice) {
            inputDevice->AddEventSink(&g_inputHandler);
            Dbg("Input event sink registered");
        }

        // 5. SKSE Menu Framework (ImGui)
        bool smfOk = SKSEMenuFramework::Init();
        Dbg(std::string("SKSEMenuFramework::Init() = ") + (smfOk ? "TRUE" : "FALSE"));

        if (smfOk) {
            EasyOutfit::UI::Register();
            Dbg("EasyOutfit UI registered in SKSE Menu Framework");
        } else {
            Dbg("SKSEMenuFramework not detected or failed to load");
        }

        Dbg("=== EasyOutfit Setup() Finished ===");
    }
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
    switch (message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        Dbg("Received SKSE Message: kDataLoaded");
        Setup();
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
    case SKSE::MessagingInterface::kNewGame:
        EasyOutfit::OutfitManager::GetSingleton().RefreshPlayer();
        break;
    default:
        break;
    }
}

SKSEPluginInfo(
    .Version = REL::Version{ 1, 0, 0, 0 },
    .Name = "EasyOutfit",
    .Author = "Alik"
)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    Dbg("=== EasyOutfit PluginLoad ===");
    auto* messaging = static_cast<SKSE::MessagingInterface*>(
        a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));

    if (!messaging) {
        Dbg("ERROR: Messaging interface is null!");
        return false;
    }

    SKSE::Init(a_skse);
    messaging->RegisterListener("SKSE", SKSEMessageHandler);
    Dbg("Registered SKSE message listener");

    return true;
}
