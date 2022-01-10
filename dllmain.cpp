#include "pch.h"

#include <cstdio>
#include <iostream>
#include <thread>
#include <TlHelp32.h>

class ImageUVS;

template<typename T>
struct Array
{
    uint32_t size;
    T* data;
};

class Image;

class Sprite
{
public:
    const bool cant_duplicate() const
    {
        const auto v = *(int*)(this + 0x74);
        return v == 1 || v == 2;
    }

    const double get_number() const
    {
        return (double)*(int*)(this + 0x18);
    }

    const std::string get_name() const
    {
        return std::string(*(char**)(this + 0x5C));
    }

    const Image* get_image(int index) const
    {
        if (*(int*)(this + 0x74) == 1)
            return NULL;
        return *(Image**)(*(int*)(this + 0x54) + 4 * index);
    }
};


typedef Sprite* (__cdecl* pGetSpriteByIndex)(int);
typedef int (__cdecl* pSpriteDuplicate)(int);
typedef void(__cdecl* pSpriteDelete)(int);

void CreateConsole()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
}

int g_SaveSprite = 0;
int g_Index = 0;
int g_Image = 0;
const char* g_Path = 0;
__declspec(noinline) void SaveSprite()
{
    _asm
    {
        push here
        push 0
        push 0
        push 0
        push 0
        push 0
        push 0

        sub     esp, 8
        push    ebx
        push    ebp
        push    esi
        push    edi

        mov edi, g_Index
        mov ebx, g_Image
        mov eax, g_Path

        jmp g_SaveSprite

        here :
    }
}

HWND g_HWND;
WNDPROC g_OldProc;
int g_Count;
volatile bool g_Done = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (GetCurrentThreadId() == GetWindowThreadProcessId(hwnd, NULL) && !g_Done)
    {
        std::cout << "Exporting..." << std::endl;

        const auto base = (uintptr_t)GetModuleHandle(NULL);
        const auto get_sprite_by_index = (pGetSpriteByIndex)(base + 0xCF4FD0);
        const auto sprite_duplicate = (pSpriteDuplicate)(base + 0xCF6170);
        const auto sprite_delete = (pSpriteDelete)(base + 0xCF60F0);
        const auto g_image_uvs = (Array<ImageUVS>)(base + 0x255C6748);

        g_SaveSprite = base + 0xD8F837;
        for (auto s = 0; s < 10; s++ && g_Count < 4000)
        {
            const auto i = g_Count;
            g_Count++;
            std::cout << "Checking sprite " << i << std::endl;
            const auto sprite = get_sprite_by_index(i);
            if (sprite != NULL)
            {
                if (!sprite->cant_duplicate())
                {
                    const auto sprite_name = sprite->get_name();
                    try
                    {
                        std::cout << "Duplicating sprite..." << std::endl;
                        const auto ii = sprite_duplicate(i);
                        if (ii > 0)
                        {
                            for (auto j = 0; j < sprite->get_number(); j++)
                            {
                                const auto path = "assets/" + sprite_name + "/" + sprite_name + "_" + std::to_string(j) + ".png";
                                g_Index = ii;
                                g_Image = j;
                                g_Path = path.c_str();
                                std::cout << "Saving sprite image..." << std::endl;
                                SaveSprite();
                            }
                            std::cout << "Deleting sprite..." << std::endl;
                            sprite_delete(ii);
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
        }
        if (g_Count > 4000)
            g_Done = true;
    }
    return g_OldProc(hwnd, uMsg, wParam, lParam);
}

void Export(HMODULE hModule)
{
    CreateConsole();

    std::cout << "Registering hook..." << std::endl;

    g_HWND = FindWindowA("YYGameMakerYY", NULL);

    std::cout << "Hook registered..." << std::endl;

    g_OldProc = (WNDPROC)SetWindowLongPtr(g_HWND, GWL_WNDPROC, (LONG_PTR)&WndProc);

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Export, hModule, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

