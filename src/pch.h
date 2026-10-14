#pragma once

#define NOMINMAX

#pragma warning(push)
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <xbyak/xbyak.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)

using namespace std::literals;

namespace logger = SKSE::log;

namespace RE
{
    inline InventoryChanges::IItemChangeVisitor::~IItemChangeVisitor() {}
}

namespace EasyOutfit
{
    inline void Log(std::string_view a_msg)
    {
        static std::filesystem::path logPath = []() {
            auto dir = SKSE::log::log_directory();
            if (dir) {
                return *dir / "EasyOutfit.log";
            }
            return std::filesystem::current_path() / "EasyOutfit.log";
        }();

        try {
            std::ofstream f(logPath, std::ios::app);
            if (f.is_open()) {
                f << a_msg << "\n";
            }
        } catch (...) {}
    }
}

#define DLLEXPORT __declspec(dllexport)
