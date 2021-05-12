#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

//typedef unsigned int UINT32;

typedef struct { float m[4][4]; } matrix_t;
typedef struct { float x, y, z, w; } vector_t;
typedef vector_t point_t;

inline int clamp(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }

// �����ֵ��t Ϊ [0, 1] ֮�����ֵ
inline float interp(float x1, float x2, float t) { return x1 + (x2 - x1) * t; }

//#ifdef __cplusplus
//extern "C" {
//#endif

// | v |
float vector_length(const vector_t *v);
// z = x + y
void vector_add(vector_t *z, const vector_t *x, const vector_t *y);
// z = x - y
void vector_sub(vector_t *z, const vector_t *x, const vector_t *y);
// ʸ�����
float vector_dotproduct(const vector_t *x, const vector_t *y);
// ʸ�����
void vector_crossproduct(vector_t *z, const vector_t *x, const vector_t *y);
// ʸ����ֵ��tȡֵ [0, 1]
void vector_interp(vector_t *z, const vector_t *x1, const vector_t *x2, float t);
// ʸ����һ��
void vector_normalize(vector_t *v);



// c = a + b
void matrix_add(matrix_t *c, const matrix_t *a, const matrix_t *b);
// c = a - b
void matrix_sub(matrix_t *c, const matrix_t *a, const matrix_t *b);
// c = a * b
void matrix_mul(matrix_t *c, const matrix_t *a, const matrix_t *b);
// c = a * f
void matrix_scale(matrix_t *c, const matrix_t *a, float f);
// y = x * m
void matrix_apply(vector_t *y, const vector_t *x, const matrix_t *m);
// ��λ����
void matrix_set_identity(matrix_t *m);
// 0 ����
void matrix_set_zero(matrix_t *m);
// ƽ�Ʊ任
void matrix_set_translate(matrix_t *m, float x, float y, float z);
// ���ű任
void matrix_set_scale(matrix_t *m, float x, float y, float z);
// ��ת����
void matrix_set_rotate(matrix_t *m, float x, float y, float z, float theta);
// ���������
void matrix_set_lookat(matrix_t *m, const vector_t *eye, const vector_t *at, const vector_t *up);
// D3DXMatrixPerspectiveFovLH
void matrix_set_perspective(matrix_t *m, float fovy, float aspect, float zn, float zf);


// ����任
typedef struct {
    matrix_t world;         // ��������任
    matrix_t view;          // ��Ӱ������任
    matrix_t projection;    // ͶӰ�任
    matrix_t transform;     // transform = world * view * projection
    float w, h;             // ��Ļ��С
} transform_t;

// ������£����� transform = world * view * projection
void transform_update(transform_t *ts);
// ��ʼ����������Ļ����
void transform_init(transform_t *ts, int width, int height);
// ��ʸ�� x ���� project 
void transform_apply(const transform_t *ts, vector_t *y, const vector_t *x);
// ����������ͬ cvv �ı߽�������׶�ü�
int transform_check_cvv(const vector_t *v);
// ��һ�����õ���Ļ����
void transform_homogenize(const transform_t *ts, vector_t *y, const vector_t *x);


// ���μ��㣺���㡢ɨ���ߡ���Ե�����Ρ���������
typedef struct { float r, g, b; } color_t;
typedef struct { float u, v; } texcoord_t;
typedef struct { point_t pos; texcoord_t tc; color_t color; float rhw; } vertex_t;

typedef struct { vertex_t v, v1, v2; } edge_t;
typedef struct { float top, bottom; edge_t left, right; } trapezoid_t;
typedef struct { vertex_t v, step; int x, y, w; } scanline_t;


void vertex_rhw_init(vertex_t *v);
void vertex_interp(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float t);
void vertex_division(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float w);
void vertex_add(vertex_t *y, const vertex_t *x);

// �������������� 0-2 �����Σ����ҷ��غϷ����ε�����
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1,
    const vertex_t *p2, const vertex_t *p3);
// ���� Y ��������������������������� Y �Ķ���
void trapezoid_edge_interp(trapezoid_t *trap, float y);
// �����������ߵĶ˵㣬��ʼ�������ɨ���ߵ����Ͳ���
void trapezoid_init_scan_line(const trapezoid_t *trap, scanline_t *scanline, int y);


#define RENDER_STATE_WIREFRAME      1		// ��Ⱦ�߿�
#define RENDER_STATE_TEXTURE        2		// ��Ⱦ����
#define RENDER_STATE_COLOR          4		// ��Ⱦ��ɫ

#define DEVICE_KEYS_SIZE            512

// ��Ⱦ�豸
struct Device {
    transform_t transform;      // ����任��
    int width;                  // ���ڿ��
    int height;                 // ���ڸ߶�
    UINT32 **framebuffer;       // ���ػ��棺framebuffer[y] ����� y ��
    float **zbuffer;            // ��Ȼ��棺zbuffer[y] Ϊ�� y ��ָ��
    UINT32 **texture;           // ����ͬ����ÿ������
    int tex_width;              // ������
    int tex_height;             // ����߶�
    float max_u;                // ��������ȣ�tex_width - 1
    float max_v;                // �������߶ȣ�tex_height - 1
    int render_state;           // ��Ⱦ״̬
    UINT32 background;          // ������ɫ
    UINT32 foreground;          // �߿���ɫ

    static int device_exit;
    static int device_keys[DEVICE_KEYS_SIZE];	// ��ǰ���̰���״̬

public:
    Device() {
        device_exit = 0;
        memset(device_keys, 0, sizeof(int) * DEVICE_KEYS_SIZE);
    }

    // win32 event handler
    static LRESULT win_events(HWND, UINT, WPARAM, LPARAM);
    void win_dispatch(void);							// ������Ϣ

public:
    void draw_plane(int a, int b, int c, int d);
    void draw_box(float theta);
    void camera_at_zero(float x, float y, float z);
    void init_texture();

    // �豸��ʼ����fb Ϊ�ⲿ֡���棬�� NULL �������ⲿ֡���棨ÿ�� 4 �ֽڶ��룩
    void device_init(int width, int height, void *fb);
    // ɾ���豸
    void device_destroy(Device *device);
    // ���õ�ǰ����
    void device_set_texture(void *bits, long pitch, int w, int h);
    // ��� framebuffer �� zbuffer
    void device_clear(int mode);
    // ����
    void device_pixel(int x, int y, UINT32 color);
    // �����߶�
    void device_draw_line(int x1, int y1, int x2, int y2, UINT32 c);
    // ���������ȡ����
    UINT32 Device_texture_read(float u, float v);

    // ��Ⱦʵ��
    // ����ɨ����
    void device_draw_scanline(scanline_t *scanline);
    // ����Ⱦ����
    void device_render_trap(trapezoid_t *trap);
    // ���� render_state ����ԭʼ������
    void device_draw_primitive(const vertex_t *v1,
        const vertex_t *v2, const vertex_t *v3);

};


//#ifdef __cplusplus
//}
//#endif

// Win32 ���ڼ�ͼ�λ��ƣ�Ϊ device �ṩһ�� DibSection �� FB
struct Window {
    int screen_w, screen_h;
    int screen_mx, screen_my, screen_mb;
    HWND screen_handle;		// ������ HWND
    HDC screen_dc;			// ���׵� HDC
    HBITMAP screen_hb;		// DIB
    HBITMAP screen_ob;		// �ϵ� BITMAP
    unsigned char *screen_fb;	// frame buffer
    long screen_pitch;

    Window() {
        screen_mx = 0, screen_my = 0, screen_mb = 0;
        screen_handle = NULL;
        screen_dc = NULL;
        screen_hb = NULL;
        screen_ob = NULL;
        screen_fb = NULL;
        screen_pitch = 0;
    }

    int screen_init(int w, int h, const TCHAR *title, WNDPROC wndproc);	// ��Ļ��ʼ��
    int screen_close(void);								// �ر���Ļ
    void screen_update(void);							// ��ʾ FrameBuffer
};
