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

// 计算插值：t 为 [0, 1] 之间的数值
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
// 矢量点乘
float vector_dotproduct(const vector_t *x, const vector_t *y);
// 矢量叉乘
void vector_crossproduct(vector_t *z, const vector_t *x, const vector_t *y);
// 矢量插值，t取值 [0, 1]
void vector_interp(vector_t *z, const vector_t *x1, const vector_t *x2, float t);
// 矢量归一化
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
// 单位矩阵
void matrix_set_identity(matrix_t *m);
// 0 矩阵
void matrix_set_zero(matrix_t *m);
// 平移变换
void matrix_set_translate(matrix_t *m, float x, float y, float z);
// 缩放变换
void matrix_set_scale(matrix_t *m, float x, float y, float z);
// 旋转矩阵
void matrix_set_rotate(matrix_t *m, float x, float y, float z, float theta);
// 设置摄像机
void matrix_set_lookat(matrix_t *m, const vector_t *eye, const vector_t *at, const vector_t *up);
// D3DXMatrixPerspectiveFovLH
void matrix_set_perspective(matrix_t *m, float fovy, float aspect, float zn, float zf);


// 坐标变换
typedef struct {
    matrix_t world;         // 世界坐标变换
    matrix_t view;          // 摄影机坐标变换
    matrix_t projection;    // 投影变换
    matrix_t transform;     // transform = world * view * projection
    float w, h;             // 屏幕大小
} transform_t;

// 矩阵更新，计算 transform = world * view * projection
void transform_update(transform_t *ts);
// 初始化，设置屏幕长宽
void transform_init(transform_t *ts, int width, int height);
// 将矢量 x 进行 project 
void transform_apply(const transform_t *ts, vector_t *y, const vector_t *x);
// 检查齐次坐标同 cvv 的边界用于视锥裁剪
int transform_check_cvv(const vector_t *v);
// 归一化，得到屏幕坐标
void transform_homogenize(const transform_t *ts, vector_t *y, const vector_t *x);


// 几何计算：顶点、扫描线、边缘、矩形、步长计算
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

// 根据三角形生成 0-2 个梯形，并且返回合法梯形的数量
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1,
    const vertex_t *p2, const vertex_t *p3);
// 按照 Y 坐标计算出左右两条边纵坐标等于 Y 的顶点
void trapezoid_edge_interp(trapezoid_t *trap, float y);
// 根据左右两边的端点，初始化计算出扫描线的起点和步长
void trapezoid_init_scan_line(const trapezoid_t *trap, scanline_t *scanline, int y);


// 渲染设备
typedef struct {
    transform_t transform;      // 坐标变换器
    int width;                  // 窗口宽度
    int height;                 // 窗口高度
    UINT32 **framebuffer;       // 像素缓存：framebuffer[y] 代表第 y 行
    float **zbuffer;            // 深度缓存：zbuffer[y] 为第 y 行指针
    UINT32 **texture;           // 纹理：同样是每行索引
    int tex_width;              // 纹理宽度
    int tex_height;             // 纹理高度
    float max_u;                // 纹理最大宽度：tex_width - 1
    float max_v;                // 纹理最大高度：tex_height - 1
    int render_state;           // 渲染状态
    UINT32 background;          // 背景颜色
    UINT32 foreground;          // 线框颜色
} device_t;

#define RENDER_STATE_WIREFRAME      1		// 渲染线框
#define RENDER_STATE_TEXTURE        2		// 渲染纹理
#define RENDER_STATE_COLOR          4		// 渲染颜色


// 设备初始化，fb为外部帧缓存，非 NULL 将引用外部帧缓存（每行 4字节对齐）
void device_init(device_t *device, int width, int height, void *fb);
// 删除设备
void device_destroy(device_t *device);
// 设置当前纹理
void device_set_texture(device_t *device, void *bits, long pitch, int w, int h);
// 清空 framebuffer 和 zbuffer
void device_clear(device_t *device, int mode);
// 画点
void device_pixel(device_t *device, int x, int y, UINT32 color);
// 绘制线段
void device_draw_line(device_t *device, int x1, int y1, int x2, int y2, UINT32 c);
// 根据坐标读取纹理
UINT32 device_texture_read(const device_t *device, float u, float v);

// 渲染实现
// 绘制扫描线
void device_draw_scanline(device_t *device, scanline_t *scanline);
// 主渲染函数
void device_render_trap(device_t *device, trapezoid_t *trap);
// 根据 render_state 绘制原始三角形
void device_draw_primitive(device_t *device, const vertex_t *v1,
    const vertex_t *v2, const vertex_t *v3);

//#ifdef __cplusplus
//}
//#endif

// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
struct Screen {
    int screen_w, screen_h;
    int screen_mx, screen_my, screen_mb;
    HWND screen_handle;		// 主窗口 HWND
    HDC screen_dc;			// 配套的 HDC
    HBITMAP screen_hb;		// DIB
    HBITMAP screen_ob;		// 老的 BITMAP
    unsigned char *screen_fb;	// frame buffer
    long screen_pitch;

    static int screen_exit;
    static int screen_keys[512];	// 当前键盘按下状态

    Screen() {
        screen_exit = 0;
        screen_mx = 0, screen_my = 0, screen_mb = 0;
        screen_handle = NULL;
        screen_dc = NULL;
        screen_hb = NULL;
        screen_ob = NULL;
        screen_fb = NULL;
        screen_pitch = 0;
    }

    int screen_init(int w, int h, const TCHAR *title);	// 屏幕初始化
    int screen_close(void);								// 关闭屏幕
    void screen_dispatch(void);							// 处理消息
    void screen_update(void);							// 显示 FrameBuffer

    // win32 event handler
    static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);
};
