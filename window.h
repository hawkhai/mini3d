#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
class Window {
    int screen_w, screen_h;
    int screen_mx, screen_my, screen_mb;
    HWND screen_handle;		// 主窗口 HWND
    HDC screen_dc;			// 配套的 HDC
    HBITMAP screen_hb;		// DIB
    HBITMAP screen_ob;		// 老的 BITMAP
    unsigned char *screen_fb;	// frame buffer
    long screen_pitch;

public:
    static int device_exit;
    static int device_keys[DEVICE_KEYS_SIZE];	// 当前键盘按下状态

public:
    Window() {
        screen_mx = 0, screen_my = 0, screen_mb = 0;
        screen_handle = NULL;
        screen_dc = NULL;
        screen_hb = NULL;
        screen_ob = NULL;
        screen_fb = NULL;
        screen_pitch = 0;
        device_exit = 0;
        memset(device_keys, 0, sizeof(int) * DEVICE_KEYS_SIZE);
    }

    int screen_init(int w, int h, const TCHAR *title);	// 屏幕初始化
    int screen_close(void);								// 关闭屏幕
    void screen_update(void);							// 显示 FrameBuffer
    unsigned char *getScreenFrameBuffer() { return screen_fb; }

    // win32 event handler
    static LRESULT win_events(HWND, UINT, WPARAM, LPARAM);
    void win_dispatch(void);							// 处理消息
};
