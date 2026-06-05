#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "Memory.h"
#include "offsets.h"
#include "Overlay.h"

using Pointer = DWORD;

struct Vector3 {
    float x, y, z;
};

struct ViewMatrix {
    float matrix[16];
};

bool WorldToScreen(Vector3 pos, Vector3& screen, float matrix[16], int width, int height) {
    float clip_w = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];
    if (clip_w < 0.01f) return false;

    float clip_x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
    float clip_y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];

    float ndc_x = clip_x / clip_w;
    float ndc_y = clip_y / clip_w;

    screen.x = (width / 2.0f) + (width / 2.0f * ndc_x);
    screen.y = (height / 2.0f) - (height / 2.0f * ndc_y);
    return true;
}

// Optimization: Faster box drawing using FillRect
void DrawBox(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    RECT r1 = { x, y, x + w, y + 1 };           // Top
    RECT r2 = { x, y + h - 1, x + w, y + h };   // Bottom
    RECT r3 = { x, y, x + 1, y + h };           // Left
    RECT r4 = { x + w - 1, y, x + w, y + h };   // Right
    FillRect(hdc, &r1, brush);
    FillRect(hdc, &r2, brush);
    FillRect(hdc, &r3, brush);
    FillRect(hdc, &r4, brush);
    DeleteObject(brush);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Set priority
    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

    Memory mem("csgo.exe");
    while (!mem.GetProcessId()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        mem = Memory("csgo.exe");
    }

    uintptr_t client = mem.GetModuleAddress("client.dll");
    if (!client) return 1;

    Overlay overlay;
    HWND gameHwnd = FindWindowA("Valve001", NULL);

    const int targetFps = 120;
    const std::chrono::milliseconds frameDuration(1000 / targetFps);

    Pointer entityList[64];
    MSG msg;

    while (true) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Optimization: Only run if game is active
        if (GetForegroundWindow() != gameHwnd && GetForegroundWindow() != overlay.hwnd) {
            overlay.StartDrawing();
            overlay.FinishDrawing();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        Pointer localPlayer = mem.Read<Pointer>(client + offsets::dwLocalPlayer);
        if (!localPlayer) {
            overlay.StartDrawing();
            overlay.FinishDrawing();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        int localTeam = mem.Read<int>(localPlayer + offsets::m_iTeamNum);
        ViewMatrix vm = mem.Read<ViewMatrix>(client + offsets::dwViewMatrix);
        
        ReadProcessMemory(mem.GetProcessHandle(), (LPCVOID)(client + offsets::dwEntityList), &entityList, sizeof(entityList), NULL);

        overlay.StartDrawing();

        for (int i = 0; i < 64; i++) {
            Pointer entity = mem.Read<Pointer>(client + offsets::dwEntityList + i * 0x10);
            if (!entity || entity == localPlayer) continue;

            BYTE buffer[0x300];
            if (!ReadProcessMemory(mem.GetProcessHandle(), (LPCVOID)entity, &buffer, sizeof(buffer), NULL)) continue;

            int health = *(int*)(buffer + offsets::m_iHealth);
            int team = *(int*)(buffer + offsets::m_iTeamNum);
            int lifeState = *(int*)(buffer + offsets::m_lifeState);
            bool dormant = *(bool*)(buffer + offsets::m_bDormant);
            Vector3 origin = *(Vector3*)(buffer + offsets::m_vecOrigin);

            if (health <= 0 || health > 100 || lifeState != 0 || dormant) continue;

            Vector3 screenPos, screenHead;
            Vector3 entityHead = origin;
            entityHead.z += 75.f; 

            if (WorldToScreen(origin, screenPos, vm.matrix, overlay.screenWidth, overlay.screenHeight) &&
                WorldToScreen(entityHead, screenHead, vm.matrix, overlay.screenWidth, overlay.screenHeight)) {
                
                float h = abs(screenPos.y - screenHead.y);
                float w = h / 2.0f;

                COLORREF color = (team == localTeam) ? RGB(0, 150, 255) : RGB(255, 0, 0);
                
                DrawBox(overlay.memDc, (int)(screenHead.x - w / 2), (int)screenHead.y, (int)w, (int)h, color);
                overlay.DrawHealthBar((int)(screenHead.x - w / 2 - 6), (int)screenHead.y, 3, (int)h, health);
            }
        }
        
        overlay.FinishDrawing();

        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        if (elapsed < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }
    return 0;
}
