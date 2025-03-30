// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <cstdint>
#include <stdio.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <mutex>

#include "patterns.hpp"
#include "scanner.hpp"
#include "minhook/MinHook.h"
#include "classFileStream.hpp"

static inline void java_print(std::string msg) {
    std::printf("[dumper] %s\n", msg.c_str());
    std::fflush(stdout);
}


using fnParseStream = void(__stdcall*)(void*, void*, void*);
static fnParseStream oParseStream;
static std::uint64_t cnt = 0;
static std::mutex mtx;
static std::string path;

static void hkParseStream(void* instance, void* cfs, void* jthread) {
    //mtx.lock();
    const ClassFileStream* classFileStream = reinterpret_cast<ClassFileStream*>(cfs);
    std::string file = std::string(path).append("class-").append(std::to_string(cnt)).append(".class");
    std::ofstream* out = new std::ofstream(file, std::ios::binary | std::ios::out);
    std::uint64_t len = ((uintptr_t)classFileStream->_buffer_end) - ((uintptr_t)classFileStream->_buffer_start) / sizeof std::uint8_t;
    out->write(reinterpret_cast<char*>(classFileStream->_buffer_start), len);
    out->flush();
    out->close();
    java_print("Save: " + file);
    ++cnt;
    delete out;
    //mtx.unlock();
    return oParseStream(instance, cfs, jthread);
}


static void __stdcall run() {

    path = std::filesystem::current_path().string().append("/dump/");

    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }

    auto jvm = GetModuleHandle("jvm.dll");
    if (!jvm) {
        java_print("jvm.dll not found!");
        return;
    }

    std::uint8_t* cfp_parseStream = scanner::scan(jvm, Patterns::ClassFileParser_parse_stream);

    if (!cfp_parseStream) {
        java_print("cfp_parseStream pattern no found!");
        return;
    }

    if (MH_Initialize() != MH_OK) {
        java_print("MinHook failed to initialize!");
        return;
    }

    if (MH_CreateHook(cfp_parseStream, &hkParseStream, reinterpret_cast<void**>(&oParseStream)) != MH_OK || !oParseStream) {
        java_print("Failed to set hook!");
        return;
    }


    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        java_print("Failed to enable hook!");
        return;
    }

    java_print("Ready to dump!");
    java_print("Saving to " + path);
}

bool __stdcall DllMain( void* handle, std::uint64_t  reason, void* resrved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&run), 0, 0, 0);
    }
    return TRUE;
}

