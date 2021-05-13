#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

// Win32 ���ڼ�ͼ�λ��ƣ�Ϊ device �ṩһ�� DibSection �� FB
class Window {
    int screen_w, screen_h;
    int screen_mx, screen_my, screen_mb;
    HWND screen_handle;		// ������ HWND
    HDC screen_dc;			// ���׵� HDC
    HBITMAP screen_hb;		// DIB
    HBITMAP screen_ob;		// �ϵ� BITMAP
    unsigned char *screen_fb;	// frame buffer
    long screen_pitch;

public:
    static int device_exit;
    static int device_keys[DEVICE_KEYS_SIZE];	// ��ǰ���̰���״̬

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

    int screen_init(int w, int h, const TCHAR *title);	// ��Ļ��ʼ��
    int screen_close(void);								// �ر���Ļ
    void screen_update(void);							// ��ʾ FrameBuffer
    unsigned char *getScreenFrameBuffer() { return screen_fb; }

    // win32 event handler
    static LRESULT win_events(HWND, UINT, WPARAM, LPARAM);
    void win_dispatch(void);							// ������Ϣ
};
