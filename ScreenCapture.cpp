#include <iostream>
#include <windows.h>

const COLORREF BACKGROUND_TRANSPARENT_COLOR = RGB(123, 123, 123);
const byte TICK_DECREASE_STEP = 20;
const int HOTKEY_ID = 123456;
const UINT_PTR TIMER_ID = 6543;

byte DRAWING_TICKS = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void set_window_transparency(HWND hwnd) {
    byte new_transparency = DRAWING_TICKS;
    if (DRAWING_TICKS > 128) new_transparency = 255 - DRAWING_TICKS;
    SetLayeredWindowAttributes(
        hwnd,
        BACKGROUND_TRANSPARENT_COLOR,
        255 - new_transparency * 2, LWA_ALPHA | LWA_COLORKEY
    );
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        if (wparam == TIMER_ID && DRAWING_TICKS > 0) {
            set_window_transparency(hwnd);
            if (DRAWING_TICKS < TICK_DECREASE_STEP) {
                DRAWING_TICKS = 0;
            }
            else {
                DRAWING_TICKS -= TICK_DECREASE_STEP;
            }
        }
        return 0;
    case WM_HOTKEY:
        if (wparam == HOTKEY_ID) {
            if (GetFocus() == hwnd) {
                POINT tl{};
                tl.x = 0;
                tl.y = 0;
                ClientToScreen(hwnd, &tl);

                RECT ClientRect{};
                GetClientRect(hwnd, &ClientRect);
                int x = tl.x;
                int y = tl.y;
                int width = ClientRect.right;
                int height = ClientRect.bottom;

                HDC dc = GetDC(NULL);
                HDC hdc = CreateCompatibleDC(dc);
                HBITMAP bitmap = CreateCompatibleBitmap(dc, width, height);
                HGDIOBJ old_obj = SelectObject(hdc, bitmap);

                BitBlt(hdc, 0, 0, width, height, dc, x, y, SRCCOPY);

                OpenClipboard(NULL);
                EmptyClipboard();
                SetClipboardData(CF_BITMAP, bitmap);
                CloseClipboard();

                std::cout << "Copied!\n";

                DRAWING_TICKS = 255;

                SelectObject(hdc, old_obj);
                DeleteDC(hdc);
                ReleaseDC(NULL, dc);
                DeleteObject(bitmap);
            }
            else {
                SetForegroundWindow(hwnd);
                SetActiveWindow(hwnd);
            }
        }
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

int main()
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    WNDCLASS win_class{};
    win_class.style = CS_VREDRAW | CS_HREDRAW;
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = CreateSolidBrush(BACKGROUND_TRANSPARENT_COLOR);
    win_class.lpfnWndProc = WindowProc;
    win_class.lpszClassName = TEXT("Main Capture Window");

    RegisterClass(&win_class);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_CLIENTEDGE,
        win_class.lpszClassName,
        TEXT("Capture"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        200, 200,
        500, 500,
        NULL, NULL,
        win_class.hInstance,
        NULL
    );
    if (hwnd == NULL) return 0;

    set_window_transparency(hwnd);

    RegisterHotKey(hwnd, HOTKEY_ID, MOD_ALT | MOD_NOREPEAT, 0x43); // ALT+C
    SetTimer(hwnd, TIMER_ID, 5, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        DispatchMessageW(&msg);
    }
}