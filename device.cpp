#include "mini3d.h"

// �豸��ʼ����fbΪ�ⲿ֡���棬�� NULL �������ⲿ֡���棨ÿ�� 4�ֽڶ��룩
void Device::device_init(int width, int height, void *fb) {
    int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
    char *ptr = (char*)malloc(need + 64);
    char *framebuf, *zbuf;
    int j;
    assert(ptr);
    this->framebuffer = (UINT32**)ptr;
    this->zbuffer = (float**)(ptr + sizeof(void*) * height);
    ptr += sizeof(void*) * height * 2;
    this->texture = (UINT32**)ptr;
    ptr += sizeof(void*) * 1024;
    framebuf = (char*)ptr;
    zbuf = (char*)ptr + width * height * 4;
    ptr += width * height * 8;
    if (fb != NULL) framebuf = (char*)fb;
    for (j = 0; j < height; j++) {
        this->framebuffer[j] = (UINT32*)(framebuf + width * 4 * j);
        this->zbuffer[j] = (float*)(zbuf + width * 4 * j);
    }
    this->texture[0] = (UINT32*)ptr;
    this->texture[1] = (UINT32*)(ptr + 16);
    memset(this->texture[0], 0, 64);
    this->tex_width = 2;
    this->tex_height = 2;
    this->max_u = 1.0f;
    this->max_v = 1.0f;
    this->width = width;
    this->height = height;
    this->background = 0xc0c0c0;
    this->foreground = 0;
    transform_init(&this->transform, width, height);
    this->render_state = RENDER_STATE_WIREFRAME;
}

// ɾ���豸
void Device::device_destroy(Device *device) {
    if (this->framebuffer)
        free(this->framebuffer);
    this->framebuffer = NULL;
    this->zbuffer = NULL;
    this->texture = NULL;
}

// ���õ�ǰ����
void Device::device_set_texture(void *bits, long pitch, int w, int h) {
    char *ptr = (char*)bits;
    int j;
    assert(w <= 1024 && h <= 1024);
    for (j = 0; j < h; ptr += pitch, j++) 	// ���¼���ÿ�������ָ��
        this->texture[j] = (UINT32*)ptr;
    this->tex_width = w;
    this->tex_height = h;
    this->max_u = (float)(w - 1);
    this->max_v = (float)(h - 1);
}

// ��� framebuffer �� zbuffer
void Device::device_clear(int mode) {
    int y, x, height = this->height;
    for (y = 0; y < this->height; y++) {
        UINT32 *dst = this->framebuffer[y];
        UINT32 cc = (height - 1 - y) * 230 / (height - 1);
        cc = (cc << 16) | (cc << 8) | cc;
        if (mode == 0) cc = this->background;
        for (x = this->width; x > 0; dst++, x--) dst[0] = cc;
    }
    for (y = 0; y < this->height; y++) {
        float *dst = this->zbuffer[y];
        for (x = this->width; x > 0; dst++, x--) dst[0] = 0.0f;
    }
}

// ����
void Device::device_pixel(int x, int y, UINT32 color) {
    if (((UINT32)x) < (UINT32)this->width && ((UINT32)y) < (UINT32)this->height) {
        this->framebuffer[y][x] = color;
    }
}

// �����߶�
void Device::device_draw_line(int x1, int y1, int x2, int y2, UINT32 c) {
    int x, y, rem = 0;
    if (x1 == x2 && y1 == y2) {
        device_pixel(x1, y1, c);
    }
    else if (x1 == x2) {
        int inc = (y1 <= y2) ? 1 : -1;
        for (y = y1; y != y2; y += inc) device_pixel(x1, y, c);
        device_pixel(x2, y2, c);
    }
    else if (y1 == y2) {
        int inc = (x1 <= x2) ? 1 : -1;
        for (x = x1; x != x2; x += inc) device_pixel(x, y1, c);
        device_pixel(x2, y2, c);
    }
    else {
        int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
        int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
        if (dx >= dy) {
            if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
            for (x = x1, y = y1; x <= x2; x++) {
                device_pixel(x, y, c);
                rem += dy;
                if (rem >= dx) {
                    rem -= dx;
                    y += (y2 >= y1) ? 1 : -1;
                    device_pixel(x, y, c);
                }
            }
            device_pixel(x2, y2, c);
        }
        else {
            if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
            for (x = x1, y = y1; y <= y2; y++) {
                device_pixel(x, y, c);
                rem += dx;
                if (rem >= dy) {
                    rem -= dy;
                    x += (x2 >= x1) ? 1 : -1;
                    device_pixel(x, y, c);
                }
            }
            device_pixel(x2, y2, c);
        }
    }
}

// ���������ȡ����
UINT32 Device::Device_texture_read(float u, float v) {
    int x, y;
    u = u * this->max_u;
    v = v * this->max_v;
    x = (int)(u + 0.5f);
    y = (int)(v + 0.5f);
    x = clamp(x, 0, this->tex_width - 1);
    y = clamp(y, 0, this->tex_height - 1);
    return this->texture[y][x];
}


//=====================================================================
// ��Ⱦʵ��
//=====================================================================

// ����ɨ����
void Device::device_draw_scanline(scanline_t *scanline) {

    UINT32 *framebuffer = this->framebuffer[scanline->y];
    float *zbuffer = this->zbuffer[scanline->y];
    int x = scanline->x;
    int w = scanline->w;
    int width = this->width;
    int render_state = this->render_state;
    for (; w > 0; x++, w--) {
        if (x >= 0 && x < width) {
            float rhw = scanline->v.rhw;
            if (rhw >= zbuffer[x]) {
                float w = 1.0f / rhw;
                zbuffer[x] = rhw;
                if (render_state & RENDER_STATE_COLOR) {
                    float r = scanline->v.color.r * w;
                    float g = scanline->v.color.g * w;
                    float b = scanline->v.color.b * w;
                    int R = (int)(r * 255.0f);
                    int G = (int)(g * 255.0f);
                    int B = (int)(b * 255.0f);
                    R = clamp(R, 0, 255);
                    G = clamp(G, 0, 255);
                    B = clamp(B, 0, 255);
                    framebuffer[x] = (R << 16) | (G << 8) | (B);
                }
                if (render_state & RENDER_STATE_TEXTURE) {
                    float u = scanline->v.tc.u * w;
                    float v = scanline->v.tc.v * w;
                    UINT32 cc = Device_texture_read(u, v);
                    framebuffer[x] = cc;
                }
            }
        }
        vertex_add(&scanline->v, &scanline->step);
        if (x >= width) break;
    }
}

// ����Ⱦ����
void Device::device_render_trap(trapezoid_t *trap) {

    scanline_t scanline;
    int j, top, bottom;
    top = (int)(trap->top + 0.5f);
    bottom = (int)(trap->bottom + 0.5f);
    for (j = top; j < bottom; j++) {
        if (j >= 0 && j < this->height) {
            trapezoid_edge_interp(trap, (float)j + 0.5f);
            trapezoid_init_scan_line(trap, &scanline, j);
            device_draw_scanline(&scanline);
        }
        if (j >= this->height) break;
    }
}

// ���� render_state ����ԭʼ������
void Device::device_draw_primitive(const vertex_t *v1, const vertex_t *v2, const vertex_t *v3) {

    point_t p1, p2, p3, c1, c2, c3;
    int render_state = this->render_state;

    // ���� Transform �仯
    transform_apply(&this->transform, &c1, &v1->pos);
    transform_apply(&this->transform, &c2, &v2->pos);
    transform_apply(&this->transform, &c3, &v3->pos);

    // �ü���ע��˴���������Ϊ�����жϼ������� cvv���Լ�ͬcvv�ཻƽ����������
    // ���н�һ����ϸ�ü�����һ���ֽ�Ϊ������ȫ���� cvv�ڵ�������
    if (transform_check_cvv(&c1) != 0) return;
    if (transform_check_cvv(&c2) != 0) return;
    if (transform_check_cvv(&c3) != 0) return;

    // ��һ��
    transform_homogenize(&this->transform, &p1, &c1);
    transform_homogenize(&this->transform, &p2, &c2);
    transform_homogenize(&this->transform, &p3, &c3);

    // �������ɫ�ʻ���
    if (render_state & (RENDER_STATE_TEXTURE | RENDER_STATE_COLOR)) {
        vertex_t t1 = *v1, t2 = *v2, t3 = *v3;
        trapezoid_t traps[2];
        int n;

        t1.pos = p1;
        t2.pos = p2;
        t3.pos = p3;
        t1.pos.w = c1.w;
        t2.pos.w = c2.w;
        t3.pos.w = c3.w;

        vertex_rhw_init(&t1);	// ��ʼ�� w
        vertex_rhw_init(&t2);	// ��ʼ�� w
        vertex_rhw_init(&t3);	// ��ʼ�� w

        // ���������Ϊ0-2�����Σ����ҷ��ؿ�����������
        n = trapezoid_init_triangle(traps, &t1, &t2, &t3);

        if (n >= 1) device_render_trap(&traps[0]);
        if (n >= 2) device_render_trap(&traps[1]);
    }

    if (render_state & RENDER_STATE_WIREFRAME) {		// �߿����
        device_draw_line((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, this->foreground);
        device_draw_line((int)p1.x, (int)p1.y, (int)p3.x, (int)p3.y, this->foreground);
        device_draw_line((int)p3.x, (int)p3.y, (int)p2.x, (int)p2.y, this->foreground);
    }
}

void Device::draw_plane(int a, int b, int c, int d) {
    vertex_t mesh[8] = {
    { { -1, -1,  1, 1 }, { 0, 0 }, { 1.0f, 0.2f, 0.2f }, 1 },
    { {  1, -1,  1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 0.2f }, 1 },
    { {  1,  1,  1, 1 }, { 1, 1 }, { 0.2f, 0.2f, 1.0f }, 1 },
    { { -1,  1,  1, 1 }, { 1, 0 }, { 1.0f, 0.2f, 1.0f }, 1 },
    { { -1, -1, -1, 1 }, { 0, 0 }, { 1.0f, 1.0f, 0.2f }, 1 },
    { {  1, -1, -1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 1.0f }, 1 },
    { {  1,  1, -1, 1 }, { 1, 1 }, { 1.0f, 0.3f, 0.3f }, 1 },
    { { -1,  1, -1, 1 }, { 1, 0 }, { 0.2f, 1.0f, 0.3f }, 1 },
    };
    vertex_t p1 = mesh[a], p2 = mesh[b], p3 = mesh[c], p4 = mesh[d];
    p1.tc.u = 0, p1.tc.v = 0, p2.tc.u = 0, p2.tc.v = 1;
    p3.tc.u = 1, p3.tc.v = 1, p4.tc.u = 1, p4.tc.v = 0;
    device_draw_primitive(&p1, &p2, &p3);
    device_draw_primitive(&p3, &p4, &p1);
}

void Device::draw_box(float theta) {
    matrix_t m;
    matrix_set_rotate(&m, -1, -0.5, 1, theta);
    this->transform.world = m;
    transform_update(&this->transform);
    draw_plane(0, 1, 2, 3);
    draw_plane(7, 6, 5, 4);
    draw_plane(0, 4, 5, 1);
    draw_plane(1, 5, 6, 2);
    draw_plane(2, 6, 7, 3);
    draw_plane(3, 7, 4, 0);
}

void Device::camera_at_zero(float x, float y, float z) {
    Device *device = this;
    point_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 0, 1, 1 };
    matrix_set_lookat(&this->transform.view, &eye, &at, &up);
    transform_update(&this->transform);
}

void Device::init_texture() {
    static UINT32 texture[256][256];
    int i, j;
    for (j = 0; j < 256; j++) {
        for (i = 0; i < 256; i++) {
            int x = i / 32, y = j / 32;
            texture[j][i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
        }
    }
    device_set_texture(texture, 256 * 4, 256, 256);
}
