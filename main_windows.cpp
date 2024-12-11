typedef signed char s8;
typedef short s16;
typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;
struct v4 { f32 x, y, z, w; };

template<typename T> inline T min(T x, T y) { return x < y ? x : y; }
template<typename T> inline T max(T x, T y) { return x > y ? x : y; }

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <Windows.h>
#include <Winsock2.h>
#include <Dwmapi.h>
#include <mmsystem.h>

static HINSTANCE platform_hinstance;
static HWND platform_hwnd;
static HDC platform_hdc;
static s32 platform_screen_width;
static s32 platform_screen_height;
static s32 platform_mouse_x;
static s32 platform_mouse_y;

static HDC backbuffer_hdc;
static HBITMAP backbuffer_hbm;
static void* backbuffer_pixels;
static s32 backbuffer_width;
static s32 backbuffer_height;
static s8 backbuffer_padding;

static u32 argb_color(v4 color) {
  u32 result = ((u32) (u8) (color.w * 255.99f) << 24) |
    ((u32) (u8) (color.x * 255.99f) << 16) |
    ((u32) (u8) (color.y * 255.99f) << 8) |
    ((u32) (u8) (color.z * 255.99f) << 0);
  return result;
}

static void draw_rect(s32 xmin, s32 ymin, s32 xmax, s32 ymax, v4 color) {
  xmin = max(xmin, 0);
  ymin = max(ymin, 0);
  xmax = min(xmax, platform_screen_width - 1);
  ymax = min(ymax, platform_screen_height - 1);

  u32 color32 = argb_color(color);

  u32* row = (u32*) backbuffer_pixels + ((backbuffer_padding + ymin) * (backbuffer_width + 2 * backbuffer_padding)) + (backbuffer_padding + xmin);
  for (s32 y = ymin; y <= ymax; y += 1) {
    u32* pixel = row;
    for (s32 x = xmin; x <= xmax; x += 1) {
      *pixel = color32;
      pixel += 1;
    }
    row += backbuffer_width + 2 * backbuffer_padding;
  }
}

static bool draw_button(s32 xmin, s32 ymin, s32 xmax, s32 ymax) {
  s32 mouse_x = platform_mouse_x;
  s32 mouse_y = platform_screen_height - platform_mouse_y;

  bool result = false;
  v4 color = {1.0, 0.0, 0.0, 1.0};
  if (mouse_x >= xmin && mouse_x <= xmax &&
    mouse_y >= ymin && mouse_y <= ymax)
  {
    color = {0.0f, 1.0f, 0.0f, 1.0f};
    result = true;
  }
  draw_rect(xmin, ymin, xmax, ymax, color);
  return result;
}

static LRESULT WINAPI window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_PAINT: {
      ValidateRect(hwnd, nullptr);
      return 0;
    }
    case WM_ERASEBKGND: {
      return 1;
    }
    case WM_MOUSEMOVE: {
      platform_mouse_x = (s16) lParam;
      platform_mouse_y = (s16) ((WPARAM) lParam >> 16);
      return 0;
    }
    case WM_ACTIVATEAPP: {
      // if (wParam != 0) update_cursor_clip();
      // else clear_held_keys();
      return 0;
    }
    case WM_SIZE: {
      platform_screen_width = (u16) (WPARAM) lParam;
      platform_screen_height = (u16) ((WPARAM) lParam >> 16);

      backbuffer_padding = 8;
      backbuffer_width = platform_screen_width;
      backbuffer_height = platform_screen_height;

      static BITMAPINFO bmi = {};
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) + 0;
      bmi.bmiHeader.biWidth = backbuffer_width + 2 * backbuffer_padding;
      bmi.bmiHeader.biHeight = backbuffer_height + 2 * backbuffer_padding;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;

      if (backbuffer_hdc) DeleteDC(backbuffer_hdc);
      if (backbuffer_hbm) DeleteObject(backbuffer_hbm);
      backbuffer_hdc = CreateCompatibleDC(platform_hdc);
      backbuffer_hbm = CreateDIBSection(platform_hdc, &bmi, 0, &backbuffer_pixels, nullptr, 0);
      SelectObject(backbuffer_hdc, backbuffer_hbm);
      return 0;
    }
    case WM_CREATE: {
      platform_hwnd = hwnd;
      platform_hdc = GetDC(hwnd);

      u32 dark_mode = 1;
      DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, 4);
      u32 round_mode = DWMWCP_DONOTROUND;
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

  WSADATA wsadata;
  bool networking_supported = WSAStartup(0x202, &wsadata) == 0;

  bool sleep_is_granular = timeBeginPeriod(1) == 0;

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

    if (draw_button(0, 0, 100, 100)) {
      // DestroyWindow(platform_hwnd);
    }

    BitBlt(platform_hdc, 0, 0, platform_screen_width, platform_screen_height,
      backbuffer_hdc, backbuffer_padding, backbuffer_padding + (backbuffer_height - platform_screen_height), SRCCOPY);

    if (sleep_is_granular) {
      Sleep(1);
    }
  }
main_loop_end:

  if (networking_supported) WSACleanup();
  ExitProcess(0);
}

extern "C" int _fltused = 0;
