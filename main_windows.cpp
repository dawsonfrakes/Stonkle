#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include "Windows.h"
#include "Winsock2.h"
#include "Dwmapi.h"
#include "mmsystem.h"

static HINSTANCE platform_hinstance;
static HWND platform_hwnd;
static HDC platform_hdc;
static unsigned short platform_screen_width;
static unsigned short platform_screen_height;

static HDC backbuffer_hdc;
static HBITMAP backbuffer_hbm;
static void* backbuffer_data;

extern "C" LRESULT WINAPI window_proc(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_PAINT: {
      ValidateRect(hwnd, nullptr);
      return 0;
    }
    case WM_ERASEBKGND: {
      return 1;
    }
    case WM_ACTIVATEAPP: {
      // if (wParam != 0) update_cursor_clip();
      // else clear_held_keys();
      return 0;
    }
    case WM_SIZE: {
      platform_screen_width = (unsigned short) (WPARAM) lParam;
      platform_screen_height = (unsigned short) ((WPARAM) lParam >> 16);

      BITMAPINFO bmi = {};
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth = platform_screen_width;
      bmi.bmiHeader.biHeight = platform_screen_height;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;

      if (backbuffer_hdc) DeleteDC(backbuffer_hdc);
      if (backbuffer_hbm) DeleteObject(backbuffer_hbm);
      backbuffer_hdc = CreateCompatibleDC(platform_hdc);
      backbuffer_hbm = CreateDIBSection(platform_hdc, &bmi, 0, &backbuffer_data, nullptr, 0);
      SelectObject(backbuffer_hdc, backbuffer_hbm);
      return 0;
    }
    case WM_CREATE: {
      platform_hwnd = hwnd;
      platform_hdc = GetDC(hwnd);

      unsigned int dark_mode = 1;
      DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, 4);
      unsigned int round_mode = DWMWCP_DONOTROUND;
      DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &round_mode, 4);
      return 0;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_SYSCOMMAND: {
      if (wParam == SC_KEYMENU) return 0;
      // fallthrough
    }
    default: {
      return DefWindowProcW(hwnd, message, wParam, lParam);
    }
  }
}

extern "C" [[noreturn]] void WINAPI WinMainCRTStartup() {
  platform_hinstance = GetModuleHandleW(nullptr);

  bool sleep_is_granular = timeBeginPeriod(1) == 0;

  WSADATA wsadata;
  bool networking_supported = WSAStartup(0x202, &wsadata) == 0;

  SetProcessDPIAware();
  WNDCLASSEXW wndclass = {};
  wndclass.cbSize = sizeof(WNDCLASSEXW);
  wndclass.style = CS_OWNDC;
  wndclass.lpfnWndProc = window_proc;
  wndclass.hInstance = platform_hinstance;
  wndclass.hIcon = LoadIconW(nullptr, IDI_WARNING);
  wndclass.hCursor = LoadCursorW(nullptr, IDC_CROSS);
  wndclass.lpszClassName = L"A";
  RegisterClassExW(&wndclass);
  CreateWindowExW(0, wndclass.lpszClassName, L"Stonkle",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    nullptr, nullptr, platform_hinstance, nullptr);

  for (;;) {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      switch (msg.message) {
        case WM_QUIT: {
          goto main_loop_end;
        }
        default: {
          DispatchMessageW(&msg);
        }
      }
    }

    BitBlt(platform_hdc, 0, 0, platform_screen_width, platform_screen_height,
      backbuffer_hdc, 0, 0, SRCCOPY);

    if (sleep_is_granular) {
      Sleep(1);
    }
  }
main_loop_end:

  if (networking_supported) WSACleanup();
  ExitProcess(0);
}
