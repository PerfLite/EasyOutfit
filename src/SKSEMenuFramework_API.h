#pragma once

namespace SKSEMenuFramework
{
    using RenderCallback = void (*)();
    using AddSectionItem_t = void (*)(const char*, RenderCallback);
    using AddWindow_t = void (*)(RenderCallback);
    using RegisterHudElement_t = void (*)(RenderCallback);

    struct ImVec2
    {
        float x{ 0.0f };
        float y{ 0.0f };
    };

    struct ImVec4
    {
        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };
        float w{ 0.0f };
    };

    using igText_t = void (*)(const char*, ...);
    using igCheckbox_t = bool (*)(const char*, bool*);
    using igSliderFloat_t = bool (*)(const char*, float*, float, float, const char*, int);
    using igButton_t = bool (*)(const char*, ImVec2);
    using igSeparator_t = void (*)();
    using igSameLine_t = void (*)(float, float);
    using igSpacing_t = void (*)();
    using igInputText_t = bool (*)(const char*, char*, size_t, int, void*, void*);
    using igInputTextWithHint_t = bool (*)(const char*, const char*, char*, size_t, int, void*, void*);
    using igSetNextItemWidth_t = void (*)(float);

    using igBeginCombo_t = bool (*)(const char*, const char*, int);
    using igEndCombo_t = void (*)();
    using igSelectable_Bool_t = bool (*)(const char*, bool, int, ImVec2);
    using igBeginChild_Str_t = bool (*)(const char*, ImVec2, bool, int);
    using igEndChild_t = void (*)();

    using igBeginTabBar_t = bool (*)(const char*, int);
    using igEndTabBar_t = void (*)();
    using igBeginTabItem_t = bool (*)(const char*, bool*, int);
    using igEndTabItem_t = void (*)();

    using igBeginTable_t = bool (*)(const char*, int, int, ImVec2, float);
    using igEndTable_t = void (*)();
    using igTableNextRow_t = void (*)(int, float);
    using igTableNextColumn_t = bool (*)();
    using igTableSetupColumn_t = void (*)(const char*, int, float, unsigned int);
    using igTableHeadersRow_t = void (*)();

    inline AddSectionItem_t AddSectionItem = nullptr;
    inline AddWindow_t AddWindow = nullptr;
    inline RegisterHudElement_t RegisterHudElement = nullptr;
    inline igText_t igText = nullptr;
    inline igCheckbox_t igCheckbox = nullptr;
    inline igSliderFloat_t igSliderFloat = nullptr;
    inline igButton_t igButton = nullptr;
    inline igSeparator_t igSeparator = nullptr;
    inline igSameLine_t igSameLine = nullptr;
    inline igSpacing_t igSpacing = nullptr;
    inline igInputText_t igInputText = nullptr;
    inline igInputTextWithHint_t igInputTextWithHint = nullptr;
    inline igSetNextItemWidth_t igSetNextItemWidth = nullptr;

    inline igBeginCombo_t igBeginCombo = nullptr;
    inline igEndCombo_t igEndCombo = nullptr;
    inline igSelectable_Bool_t igSelectable_Bool = nullptr;
    inline igBeginChild_Str_t igBeginChild_Str = nullptr;
    inline igEndChild_t igEndChild = nullptr;

    inline igBeginTabBar_t igBeginTabBar = nullptr;
    inline igEndTabBar_t igEndTabBar = nullptr;
    inline igBeginTabItem_t igBeginTabItem = nullptr;
    inline igEndTabItem_t igEndTabItem = nullptr;

    inline igBeginTable_t igBeginTable = nullptr;
    inline igEndTable_t igEndTable = nullptr;
    inline igTableNextRow_t igTableNextRow = nullptr;
    inline igTableNextColumn_t igTableNextColumn = nullptr;
    inline igTableSetupColumn_t igTableSetupColumn = nullptr;
    inline igTableHeadersRow_t igTableHeadersRow = nullptr;

    inline bool Init()
    {
        auto hMod = GetModuleHandleA("SKSEMenuFramework.dll");
        if (!hMod) {
            hMod = LoadLibraryA("SKSEMenuFramework.dll");
        }
        if (!hMod) {
            return false;
        }

        AddSectionItem = reinterpret_cast<AddSectionItem_t>(GetProcAddress(hMod, "AddSectionItem"));
        AddWindow = reinterpret_cast<AddWindow_t>(GetProcAddress(hMod, "AddWindow"));
        RegisterHudElement = reinterpret_cast<RegisterHudElement_t>(GetProcAddress(hMod, "RegisterHudElement"));
        igText = reinterpret_cast<igText_t>(GetProcAddress(hMod, "igText"));
        igCheckbox = reinterpret_cast<igCheckbox_t>(GetProcAddress(hMod, "igCheckbox"));
        igSliderFloat = reinterpret_cast<igSliderFloat_t>(GetProcAddress(hMod, "igSliderFloat"));
        igButton = reinterpret_cast<igButton_t>(GetProcAddress(hMod, "igButton"));
        igSeparator = reinterpret_cast<igSeparator_t>(GetProcAddress(hMod, "igSeparator"));
        igSameLine = reinterpret_cast<igSameLine_t>(GetProcAddress(hMod, "igSameLine"));
        igSpacing = reinterpret_cast<igSpacing_t>(GetProcAddress(hMod, "igSpacing"));
        igInputText = reinterpret_cast<igInputText_t>(GetProcAddress(hMod, "igInputText"));
        igInputTextWithHint = reinterpret_cast<igInputTextWithHint_t>(GetProcAddress(hMod, "igInputTextWithHint"));
        igSetNextItemWidth = reinterpret_cast<igSetNextItemWidth_t>(GetProcAddress(hMod, "igSetNextItemWidth"));

        igBeginCombo = reinterpret_cast<igBeginCombo_t>(GetProcAddress(hMod, "igBeginCombo"));
        igEndCombo = reinterpret_cast<igEndCombo_t>(GetProcAddress(hMod, "igEndCombo"));
        igSelectable_Bool = reinterpret_cast<igSelectable_Bool_t>(GetProcAddress(hMod, "igSelectable_Bool"));
        igBeginChild_Str = reinterpret_cast<igBeginChild_Str_t>(GetProcAddress(hMod, "igBeginChild_Str"));
        igEndChild = reinterpret_cast<igEndChild_t>(GetProcAddress(hMod, "igEndChild"));

        igBeginTabBar = reinterpret_cast<igBeginTabBar_t>(GetProcAddress(hMod, "igBeginTabBar"));
        igEndTabBar = reinterpret_cast<igEndTabBar_t>(GetProcAddress(hMod, "igEndTabBar"));
        igBeginTabItem = reinterpret_cast<igBeginTabItem_t>(GetProcAddress(hMod, "igBeginTabItem"));
        igEndTabItem = reinterpret_cast<igEndTabItem_t>(GetProcAddress(hMod, "igEndTabItem"));

        igBeginTable = reinterpret_cast<igBeginTable_t>(GetProcAddress(hMod, "igBeginTable"));
        igEndTable = reinterpret_cast<igEndTable_t>(GetProcAddress(hMod, "igEndTable"));
        igTableNextRow = reinterpret_cast<igTableNextRow_t>(GetProcAddress(hMod, "igTableNextRow"));
        igTableNextColumn = reinterpret_cast<igTableNextColumn_t>(GetProcAddress(hMod, "igTableNextColumn"));
        igTableSetupColumn = reinterpret_cast<igTableSetupColumn_t>(GetProcAddress(hMod, "igTableSetupColumn"));
        igTableHeadersRow = reinterpret_cast<igTableHeadersRow_t>(GetProcAddress(hMod, "igTableHeadersRow"));

        return AddSectionItem != nullptr;
    }
}
