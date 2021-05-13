#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>

#include "math.h"

// ����任
typedef struct Transform {
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

// �������������� 0-2 �����Σ����ҷ��غϷ����ε�����
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1, const vertex_t *p2, const vertex_t *p3);
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
    void device_draw_primitive(const vertex_t *v1, const vertex_t *v2, const vertex_t *v3);

};
