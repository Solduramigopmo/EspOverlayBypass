#pragma once
#include <windows.h>

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

class Overlay {
public:
    HWND hwnd;
    HDC hdc;
    HDC memDc;
    HBITMAP memBitmap;
    int screenWidth, screenHeight;

    Overlay() {
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);

        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "SystemOverlay", NULL };
        RegisterClassEx(&wc);

        hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, "SystemOverlay", "System Utility", WS_POPUP, 0, 0, screenWidth, screenHeight, NULL, NULL, GetModuleHandle(NULL), NULL);

        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

        ShowWindow(hwnd, SW_SHOW);
        
        hdc = GetDC(hwnd);
        memDc = CreateCompatibleDC(hdc);
        memBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
        SelectObject(memDc, memBitmap);
    }

    ~Overlay() {
        DeleteObject(memBitmap);
        DeleteDC(memDc);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
    }

    void StartDrawing() {
        RECT rect = { 0, 0, screenWidth, screenHeight };
        FillRect(memDc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    }

    void FinishDrawing() {
        BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDc, 0, 0, SRCCOPY);
    }

    void DrawRect(int x, int y, int w, int h, COLORREF color) {
        HPEN hPen = CreatePen(PS_SOLID, 1, color);
        HPEN hOldPen = (HPEN)SelectObject(memDc, hPen);
        
        MoveToEx(memDc, x, y, NULL);
        LineTo(memDc, x + w, y);
        LineTo(memDc, x + w, y + h);
        LineTo(memDc, x, y + h);
        LineTo(memDc, x, y);

        SelectObject(memDc, hOldPen);
        DeleteObject(hPen);
    }

    void DrawHealthBar(int x, int y, int w, int h, int health) {
        // Background
        HBRUSH hBg = CreateSolidBrush(RGB(50, 50, 50));
        RECT rBg = { x, y, x + w, y + h };
        FillRect(memDc, &rBg, hBg);
        DeleteObject(hBg);

        // Health
        COLORREF hpColor = RGB((255 * (100 - health)) / 100, (255 * health) / 100, 0);
        HBRUSH hHp = CreateSolidBrush(hpColor);
        int hpHeight = (int)(h * (health / 100.0f));
        RECT rHp = { x, y + (h - hpHeight), x + w, y + h };
        FillRect(memDc, &rHp, hHp);
        DeleteObject(hHp);
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_DESTROY: PostQuitMessage(0); return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};
