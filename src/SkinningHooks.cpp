#include "pch.h"
#include "SkinningHooks.h"
#include "OutfitManager.h"
#include "Settings.h"

namespace EasyOutfit::Hooks
{
    static bool ShouldOverrideSkinning(RE::TESObjectREFR* a_target)
    {
        if (!a_target) return false;

        // Fast check: ONLY override the player character!
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (a_target != player) return false;

        const auto& cfg = Settings::GetSingleton().GetConfig();
        if (!cfg.enabled) return false;

        if (cfg.suppressBeastForm) {
            auto* actor = a_target->As<RE::Actor>();
            if (actor) {
                auto* race = actor->GetRace();
                if (race) {
                    uint32_t formID = race->GetFormID();
                    if (formID == 0x000CDD84 || (formID & 0x00FFFFFF) == 0x0000283A) {
                        return false;
                    }
                }
            }
        }

        auto* outfit = OutfitManager::GetSingleton().GetActiveOutfit();
        if (!outfit || outfit->armors.empty()) return false;

        return true;
    }

    class ShieldVisitor : public RE::InventoryChanges::IItemChangeVisitor
    {
    public:
        RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_data) override
        {
            if (a_data && a_data->object && a_data->object->GetFormType() == RE::FormType::Armor) {
                auto* armor = a_data->object->As<RE::TESObjectARMO>();
                if (armor && armor->IsShield()) {
                    mask |= static_cast<uint32_t>(armor->GetSlotMask());
                    hasShield = true;
                }
            }
            return RE::BSContainer::ForEachResult::kContinue;
        }

        uint32_t mask{ 0 };
        bool hasShield{ false };
    };

    static void* AllocateCode(Xbyak::CodeGenerator& a_code)
    {
        auto& trampoline = SKSE::GetTrampoline();
        auto size = a_code.getSize();
        void* result = trampoline.allocate(size);
        std::memcpy(result, a_code.getCode(), size);
        return result;
    }

    namespace DontVanillaSkinPlayer
    {
        static bool ShouldOverride(RE::TESObjectARMO* a_armor, RE::TESObjectREFR* a_target)
        {
            if (!ShouldOverrideSkinning(a_target)) return false;
            if (!a_armor) return false;

            if (a_armor->IsShield()) {
                auto* outfit = OutfitManager::GetSingleton().GetActiveOutfit();
                if (!outfit || !outfit->HasShield()) {
                    return false; // let real shield be skinned
                }
            }
            return true;
        }

        void Apply(bool a_isAE)
        {
            auto hookAddr = a_isAE ?
                REL::ID(24736).address() + 0x302 :
                REL::ID(24232).address() + 0x302;

            auto applyArmorAddonAddr = a_isAE ?
                REL::ID(17792).address() :
                REL::ID(17392).address();

            struct Code : Xbyak::CodeGenerator
            {
                Code(std::uintptr_t a_hook, std::uintptr_t a_apply)
                {
                    Xbyak::Label j_Out;
                    Xbyak::Label f_ApplyArmorAddon;
                    Xbyak::Label f_ShouldOverride;

                    // armor in rcx, target in r13
                    push(rcx); // 8
                    push(rdx); // 16
                    push(r9);  // 24
                    push(r8);  // 32 bytes (multiple of 16)
                    mov(rdx, r13);
                    sub(rsp, 0x20); // 32 bytes shadow space (rsp % 16 == 0)
                    call(ptr[rip + f_ShouldOverride]);
                    add(rsp, 0x20);
                    pop(r8);
                    pop(r9);
                    pop(rdx);
                    pop(rcx);
                    test(al, al);
                    jnz(j_Out);

                    sub(rsp, 0x20); // shadow space
                    call(ptr[rip + f_ApplyArmorAddon]);
                    add(rsp, 0x20);

                    L(j_Out);
                    jmp(ptr[rip]);
                    dq(a_hook + 0x5);

                    L(f_ApplyArmorAddon);
                    dq(a_apply);

                    L(f_ShouldOverride);
                    dq(reinterpret_cast<std::uintptr_t>(ShouldOverride));
                }
            };

            Code gen(hookAddr, applyArmorAddonAddr);
            auto& trampoline = SKSE::GetTrampoline();
            void* codeBuf = AllocateCode(gen);
            trampoline.write_branch<5>(hookAddr, codeBuf);
        }
    }

    namespace ShimWornFlags
    {
        static uint32_t OverrideWornFlags(RE::InventoryChanges* a_inventory)
        {
            uint32_t mask = 0;
            auto* activeOutfit = OutfitManager::GetSingleton().GetActiveOutfit();
            if (!activeOutfit) return mask;

            bool shieldEquipped = false;
            if (!activeOutfit->HasShield() && a_inventory) {
                ShieldVisitor visitor;
                a_inventory->VisitWornItems(visitor);
                mask |= visitor.mask;
                shieldEquipped = visitor.hasShield;
            }

            for (auto* armor : activeOutfit->armors) {
                if (armor) {
                    if (armor->IsShield()) {
                        if (!shieldEquipped) continue;
                    }
                    mask |= static_cast<uint32_t>(armor->GetSlotMask());
                }
            }
            return mask;
        }

        void Apply(bool a_isAE)
        {
            auto hookAddr = a_isAE ?
                REL::ID(24724).address() + 0x80 :
                REL::ID(24220).address() + 0x7C;

            auto getWornMaskAddr = a_isAE ?
                REL::ID(16044).address() :
                REL::ID(15806).address();

            struct Code : Xbyak::CodeGenerator
            {
                Code(std::uintptr_t a_hook, std::uintptr_t a_getMask, bool a_ae)
                {
                    Xbyak::Label j_SuppressVanilla;
                    Xbyak::Label j_Out;
                    Xbyak::Label f_ShouldOverrideSkinning;
                    Xbyak::Label f_GetWornMask;
                    Xbyak::Label f_OverrideWornFlags;

                    // target in rbx (AE) or rsi (SE)
                    // rcx holds inventory pointer
                    push(rcx); // 8 bytes
                    if (a_ae) {
                        mov(rcx, rbx);
                    } else {
                        mov(rcx, rsi);
                    }
                    sub(rsp, 0x28); // 8 + 40 = 48 bytes (rsp % 16 == 0)
                    call(ptr[rip + f_ShouldOverrideSkinning]);
                    add(rsp, 0x28);
                    pop(rcx); // restore rcx = inventory pointer (rsp % 16 == 0)

                    test(al, al);
                    jnz(j_SuppressVanilla);

                    sub(rsp, 0x20); // shadow space
                    call(ptr[rip + f_GetWornMask]);
                    add(rsp, 0x20);
                    jmp(j_Out);

                    L(j_SuppressVanilla);
                    sub(rsp, 0x20); // shadow space
                    call(ptr[rip + f_OverrideWornFlags]);
                    add(rsp, 0x20);

                    L(j_Out);
                    jmp(ptr[rip]);
                    dq(a_hook + 0x5);

                    L(f_ShouldOverrideSkinning);
                    dq(reinterpret_cast<std::uintptr_t>(ShouldOverrideSkinning));

                    L(f_GetWornMask);
                    dq(a_getMask);

                    L(f_OverrideWornFlags);
                    dq(reinterpret_cast<std::uintptr_t>(OverrideWornFlags));
                }
            };

            Code gen(hookAddr, getWornMaskAddr, a_isAE);
            auto& trampoline = SKSE::GetTrampoline();
            void* codeBuf = AllocateCode(gen);
            trampoline.write_branch<5>(hookAddr, codeBuf);
        }
    }

    namespace CustomSkinPlayer
    {
        static void Custom(RE::Actor* a_target, void* a_actorWeightModel)
        {
            if (!a_target || !a_actorWeightModel) return;

            auto* base = a_target->GetActorBase();
            if (!base) return;

            auto* race = base->GetRace();
            if (!race) return;
            bool isFemale = base->IsFemale();

            auto* activeOutfit = OutfitManager::GetSingleton().GetActiveOutfit();
            if (!activeOutfit) return;

            bool playerHasShieldEquipped = false;
            auto* inv = a_target->GetInventoryChanges();
            if (inv) {
                ShieldVisitor visitor;
                inv->VisitWornItems(visitor);
                playerHasShieldEquipped = visitor.hasShield;
            }

            using ApplyArmorAddon_t = bool (*)(RE::TESObjectARMO*, RE::TESRace*, void*, bool);
            static REL::Relocation<ApplyArmorAddon_t> applyArmorAddonFunc{ RELOCATION_ID(17392, 17792) };

            for (auto* armor : activeOutfit->armors) {
                if (!armor) continue;
                if (armor->IsShield()) {
                    if (!playerHasShieldEquipped) {
                        continue;
                    }
                }
                applyArmorAddonFunc(armor, race, a_actorWeightModel, isFemale);
            }
        }

        void Apply(bool a_isAE)
        {
            auto hookAddr = a_isAE ?
                REL::ID(24725).address() + 0x1EF :
                REL::ID(24231).address() + 0x81;

            auto execVisitorAddr = a_isAE ?
                REL::ID(16096).address() :
                REL::ID(15856).address();

            struct Code : Xbyak::CodeGenerator
            {
                Code(std::uintptr_t a_hook, std::uintptr_t a_execVisitor, bool a_ae)
                {
                    Xbyak::Label j_Out;
                    Xbyak::Label f_Custom;
                    Xbyak::Label f_ExecuteVisitorOnWorn;
                    Xbyak::Label f_ShouldOverrideSkinning;

                    sub(rsp, 0x20); // shadow space for ExecuteVisitorOnWorn
                    call(ptr[rip + f_ExecuteVisitorOnWorn]);
                    add(rsp, 0x20);

                    push(rcx); // 8 bytes
                    mov(rcx, rbx);
                    sub(rsp, 0x28); // 8 + 40 = 48 bytes (rsp % 16 == 0)
                    call(ptr[rip + f_ShouldOverrideSkinning]);
                    add(rsp, 0x28);
                    pop(rcx); // rsp % 16 == 0

                    test(al, al);
                    jz(j_Out);

                    push(rdx); // 8
                    push(rcx); // 16 bytes
                    mov(rcx, rbx);
                    if (a_ae) {
                        mov(rdx, r15);
                    } else {
                        mov(rdx, rdi);
                    }
                    sub(rsp, 0x20); // 16 + 32 = 48 bytes (rsp % 16 == 0)
                    call(ptr[rip + f_Custom]);
                    add(rsp, 0x20);
                    pop(rcx);
                    pop(rdx);

                    L(j_Out);
                    jmp(ptr[rip]);
                    dq(a_hook + 0x5);

                    L(f_Custom);
                    dq(reinterpret_cast<std::uintptr_t>(Custom));

                    L(f_ExecuteVisitorOnWorn);
                    dq(a_execVisitor);

                    L(f_ShouldOverrideSkinning);
                    dq(reinterpret_cast<std::uintptr_t>(ShouldOverrideSkinning));
                }
            };

            Code gen(hookAddr, execVisitorAddr, a_isAE);
            auto& trampoline = SKSE::GetTrampoline();
            void* codeBuf = AllocateCode(gen);
            trampoline.write_branch<5>(hookAddr, codeBuf);
        }
    }

    namespace FixEquipConflictCheck
    {
        class ConflictVisitor : public RE::InventoryChanges::IItemChangeVisitor
        {
        public:
            ConflictVisitor(RE::Actor* a_actor, uint32_t a_slotIndex) : actor(a_actor), slotIndex(a_slotIndex) {}

            RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_data) override
            {
                if (a_data && a_data->object && a_data->object->GetFormType() == RE::FormType::Armor) {
                    auto* armor = a_data->object->As<RE::TESObjectARMO>();
                    if (armor) {
                        using TestBodyPartByIndex_t = bool (*)(RE::TESObjectARMO*, uint32_t);
                        static REL::Relocation<TestBodyPartByIndex_t> testBodyPart{ RELOCATION_ID(17395, 17795) };

                        if (testBodyPart(armor, slotIndex)) {
                            auto* em = RE::ActorEquipManager::GetSingleton();
                            if (em) {
                                em->UnequipObject(actor, armor, nullptr, 1, nullptr, false, false, true, false, nullptr);
                            }
                        }
                    }
                }
                return RE::BSContainer::ForEachResult::kContinue;
            }

            RE::Actor* actor{ nullptr };
            uint32_t slotIndex{ 0 };
        };

        static void Inner(uint32_t a_slotIndex, RE::Actor* a_target)
        {
            if (!a_target) return;
            auto* inv = a_target->GetInventoryChanges();
            if (inv) {
                ConflictVisitor visitor(a_target, a_slotIndex);
                inv->VisitWornItems(visitor);
            }
        }

        static bool ShouldOverride(RE::TESForm* a_item)
        {
            return a_item && a_item->GetFormType() == RE::FormType::Armor;
        }

        void Apply(bool a_isAE)
        {
            auto hookAddr = a_isAE ?
                REL::ID(38004).address() + 0x97 :
                REL::ID(36979).address() + 0x97;

            auto testBodyPartAddr = a_isAE ?
                REL::ID(14119).address() :
                REL::ID(14026).address();

            struct Code : Xbyak::CodeGenerator
            {
                Code(std::uintptr_t a_hook, std::uintptr_t a_testBodyPart)
                {
                    Xbyak::Label j_Out;
                    Xbyak::Label j_Exit;
                    Xbyak::Label f_Inner;
                    Xbyak::Label f_TestBodyPartByIndex;
                    Xbyak::Label f_ShouldOverride;

                    sub(rsp, 0x20); // shadow space for TestBodyPartByIndex
                    call(ptr[rip + f_TestBodyPartByIndex]);
                    add(rsp, 0x20);
                    test(al, al);
                    jz(j_Exit);

                    // rsp+0x10: item, rdi: Actor, rbx: Body Slot
                    push(rcx); // 8 bytes
                    mov(rcx, ptr[rsp + 0x18]); // item
                    sub(rsp, 0x28); // 8 + 40 = 48 bytes (rsp % 16 == 0)
                    call(ptr[rip + f_ShouldOverride]);
                    add(rsp, 0x28);
                    pop(rcx); // rsp % 16 == 0
                    test(al, al);
                    mov(rax, 1);
                    jz(j_Out);

                    push(rcx); // 8
                    push(rdx); // 16 bytes
                    mov(rcx, rbx);
                    mov(rdx, rdi);
                    sub(rsp, 0x20); // 16 + 32 = 48 bytes (rsp % 16 == 0)
                    call(ptr[rip + f_Inner]);
                    add(rsp, 0x20);
                    pop(rdx);
                    pop(rcx);

                    L(j_Exit);
                    xor_(al, al);

                    L(j_Out);
                    jmp(ptr[rip]);
                    dq(a_hook + 0x5);

                    L(f_TestBodyPartByIndex);
                    dq(a_testBodyPart);

                    L(f_ShouldOverride);
                    dq(reinterpret_cast<std::uintptr_t>(ShouldOverride));

                    L(f_Inner);
                    dq(reinterpret_cast<std::uintptr_t>(Inner));
                }
            };

            Code gen(hookAddr, testBodyPartAddr);
            auto& trampoline = SKSE::GetTrampoline();
            void* codeBuf = AllocateCode(gen);
            trampoline.write_branch<5>(hookAddr, codeBuf);
        }
    }

    void Install()
    {
        bool isAE = (REL::Module::get().version().minor() >= 6);

        SKSE::AllocTrampoline(2048);

        DontVanillaSkinPlayer::Apply(isAE);
        ShimWornFlags::Apply(isAE);
        CustomSkinPlayer::Apply(isAE);
        FixEquipConflictCheck::Apply(isAE);
    }
}
