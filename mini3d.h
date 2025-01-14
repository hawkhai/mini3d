#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

#include "math.h"

// 坐标变换
typedef struct Transform {
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
typedef struct Color { float r, g, b; } color_t;
typedef struct TexCoord { float u, v; } texcoord_t;
typedef struct Vertex { point_t pos; texcoord_t tc; color_t color; float rhw; } vertex_t;

typedef struct Edge { vertex_t v, v1, v2; } edge_t;
typedef struct Trapezoid { float top, bottom; edge_t left, right; } trapezoid_t;
typedef struct Scanline { vertex_t v, step; int x, y, w; } scanline_t;


void vertex_rhw_init(vertex_t *v);
void vertex_interp(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float t);
void vertex_division(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float w);
void vertex_add(vertex_t *y, const vertex_t *x);

// 根据三角形生成 0-2 个梯形，并且返回合法梯形的数量
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1, const vertex_t *p2, const vertex_t *p3);
// 按照 Y 坐标计算出左右两条边纵坐标等于 Y 的顶点
void trapezoid_edge_interp(trapezoid_t *trap, float y);
// 根据左右两边的端点，初始化计算出扫描线的起点和步长
void trapezoid_init_scan_line(const trapezoid_t *trap, scanline_t *scanline, int y);


#define RENDER_STATE_WIREFRAME      1		// 渲染线框
#define RENDER_STATE_TEXTURE        2		// 渲染纹理
#define RENDER_STATE_COLOR          4		// 渲染颜色

#define DEVICE_KEYS_SIZE            512

// 渲染设备
struct Device {
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
    
public:
    void draw_plane(int a, int b, int c, int d);
    void draw_box(float theta);
    void camera_at_zero(float x, float y, float z);
    void init_texture();

    // 设备初始化，fb 为外部帧缓存，非 NULL 将引用外部帧缓存（每行 4 字节对齐）
    void device_init(int width, int height, void *fb);
    // 删除设备
    void device_destroy(Device *device);
    // 设置当前纹理
    void device_set_texture(void *bits, long pitch, int w, int h);
    // 清空 framebuffer 和 zbuffer
    void device_clear(int mode);
    // 画点
    void device_pixel(int x, int y, UINT32 color);
    // 绘制线段
    void device_draw_line(int x1, int y1, int x2, int y2, UINT32 c);
    // 根据坐标读取纹理
    UINT32 Device_texture_read(float u, float v);

    // 渲染实现
    // 绘制扫描线
    void device_draw_scanline(scanline_t *scanline);
    // 主渲染函数
    void device_render_trap(trapezoid_t *trap);
    // 根据 render_state 绘制原始三角形
    void device_draw_primitive(const vertex_t *v1, const vertex_t *v2, const vertex_t *v3);

};
