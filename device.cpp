#include "mini3d.h"

// 设备初始化，fb为外部帧缓存，非 NULL 将引用外部帧缓存（每行 4字节对齐）
void device_init(device_t *device, int width, int height, void *fb) {
    int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
    char *ptr = (char*)malloc(need + 64);
    char *framebuf, *zbuf;
    int j;
    assert(ptr);
    device->framebuffer = (UINT32**)ptr;
    device->zbuffer = (float**)(ptr + sizeof(void*) * height);
    ptr += sizeof(void*) * height * 2;
    device->texture = (UINT32**)ptr;
    ptr += sizeof(void*) * 1024;
    framebuf = (char*)ptr;
    zbuf = (char*)ptr + width * height * 4;
    ptr += width * height * 8;
    if (fb != NULL) framebuf = (char*)fb;
    for (j = 0; j < height; j++) {
        device->framebuffer[j] = (UINT32*)(framebuf + width * 4 * j);
        device->zbuffer[j] = (float*)(zbuf + width * 4 * j);
    }
    device->texture[0] = (UINT32*)ptr;
    device->texture[1] = (UINT32*)(ptr + 16);
    memset(device->texture[0], 0, 64);
    device->tex_width = 2;
    device->tex_height = 2;
    device->max_u = 1.0f;
    device->max_v = 1.0f;
    device->width = width;
    device->height = height;
    device->background = 0xc0c0c0;
    device->foreground = 0;
    transform_init(&device->transform, width, height);
    device->render_state = RENDER_STATE_WIREFRAME;
}

// 删除设备
void device_destroy(device_t *device) {
    if (device->framebuffer)
        free(device->framebuffer);
    device->framebuffer = NULL;
    device->zbuffer = NULL;
    device->texture = NULL;
}

// 设置当前纹理
void device_set_texture(device_t *device, void *bits, long pitch, int w, int h) {
    char *ptr = (char*)bits;
    int j;
    assert(w <= 1024 && h <= 1024);
    for (j = 0; j < h; ptr += pitch, j++) 	// 重新计算每行纹理的指针
        device->texture[j] = (UINT32*)ptr;
    device->tex_width = w;
    device->tex_height = h;
    device->max_u = (float)(w - 1);
    device->max_v = (float)(h - 1);
}

// 清空 framebuffer 和 zbuffer
void device_clear(device_t *device, int mode) {
    int y, x, height = device->height;
    for (y = 0; y < device->height; y++) {
        UINT32 *dst = device->framebuffer[y];
        UINT32 cc = (height - 1 - y) * 230 / (height - 1);
        cc = (cc << 16) | (cc << 8) | cc;
        if (mode == 0) cc = device->background;
        for (x = device->width; x > 0; dst++, x--) dst[0] = cc;
    }
    for (y = 0; y < device->height; y++) {
        float *dst = device->zbuffer[y];
        for (x = device->width; x > 0; dst++, x--) dst[0] = 0.0f;
    }
}

// 画点
void device_pixel(device_t *device, int x, int y, UINT32 color) {
    if (((UINT32)x) < (UINT32)device->width && ((UINT32)y) < (UINT32)device->height) {
        device->framebuffer[y][x] = color;
    }
}

// 绘制线段
void device_draw_line(device_t *device, int x1, int y1, int x2, int y2, UINT32 c) {
    int x, y, rem = 0;
    if (x1 == x2 && y1 == y2) {
        device_pixel(device, x1, y1, c);
    }
    else if (x1 == x2) {
        int inc = (y1 <= y2) ? 1 : -1;
        for (y = y1; y != y2; y += inc) device_pixel(device, x1, y, c);
        device_pixel(device, x2, y2, c);
    }
    else if (y1 == y2) {
        int inc = (x1 <= x2) ? 1 : -1;
        for (x = x1; x != x2; x += inc) device_pixel(device, x, y1, c);
        device_pixel(device, x2, y2, c);
    }
    else {
        int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
        int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
        if (dx >= dy) {
            if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
            for (x = x1, y = y1; x <= x2; x++) {
                device_pixel(device, x, y, c);
                rem += dy;
                if (rem >= dx) {
                    rem -= dx;
                    y += (y2 >= y1) ? 1 : -1;
                    device_pixel(device, x, y, c);
                }
            }
            device_pixel(device, x2, y2, c);
        }
        else {
            if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
            for (x = x1, y = y1; y <= y2; y++) {
                device_pixel(device, x, y, c);
                rem += dx;
                if (rem >= dy) {
                    rem -= dy;
                    x += (x2 >= x1) ? 1 : -1;
                    device_pixel(device, x, y, c);
                }
            }
            device_pixel(device, x2, y2, c);
        }
    }
}

// 根据坐标读取纹理
UINT32 device_texture_read(const device_t *device, float u, float v) {
    int x, y;
    u = u * device->max_u;
    v = v * device->max_v;
    x = (int)(u + 0.5f);
    y = (int)(v + 0.5f);
    x = clamp(x, 0, device->tex_width - 1);
    y = clamp(y, 0, device->tex_height - 1);
    return device->texture[y][x];
}


//=====================================================================
// 渲染实现
//=====================================================================

// 绘制扫描线
void device_draw_scanline(device_t *device, scanline_t *scanline) {
    UINT32 *framebuffer = device->framebuffer[scanline->y];
    float *zbuffer = device->zbuffer[scanline->y];
    int x = scanline->x;
    int w = scanline->w;
    int width = device->width;
    int render_state = device->render_state;
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
                    UINT32 cc = device_texture_read(device, u, v);
                    framebuffer[x] = cc;
                }
            }
        }
        vertex_add(&scanline->v, &scanline->step);
        if (x >= width) break;
    }
}

// 主渲染函数
void device_render_trap(device_t *device, trapezoid_t *trap) {
    scanline_t scanline;
    int j, top, bottom;
    top = (int)(trap->top + 0.5f);
    bottom = (int)(trap->bottom + 0.5f);
    for (j = top; j < bottom; j++) {
        if (j >= 0 && j < device->height) {
            trapezoid_edge_interp(trap, (float)j + 0.5f);
            trapezoid_init_scan_line(trap, &scanline, j);
            device_draw_scanline(device, &scanline);
        }
        if (j >= device->height) break;
    }
}

// 根据 render_state 绘制原始三角形
void device_draw_primitive(device_t *device, const vertex_t *v1,
    const vertex_t *v2, const vertex_t *v3) {
    point_t p1, p2, p3, c1, c2, c3;
    int render_state = device->render_state;

    // 按照 Transform 变化
    transform_apply(&device->transform, &c1, &v1->pos);
    transform_apply(&device->transform, &c2, &v2->pos);
    transform_apply(&device->transform, &c3, &v3->pos);

    // 裁剪，注意此处可以完善为具体判断几个点在 cvv内以及同cvv相交平面的坐标比例
    // 进行进一步精细裁剪，将一个分解为几个完全处在 cvv内的三角形
    if (transform_check_cvv(&c1) != 0) return;
    if (transform_check_cvv(&c2) != 0) return;
    if (transform_check_cvv(&c3) != 0) return;

    // 归一化
    transform_homogenize(&device->transform, &p1, &c1);
    transform_homogenize(&device->transform, &p2, &c2);
    transform_homogenize(&device->transform, &p3, &c3);

    // 纹理或者色彩绘制
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

        vertex_rhw_init(&t1);	// 初始化 w
        vertex_rhw_init(&t2);	// 初始化 w
        vertex_rhw_init(&t3);	// 初始化 w

        // 拆分三角形为0-2个梯形，并且返回可用梯形数量
        n = trapezoid_init_triangle(traps, &t1, &t2, &t3);

        if (n >= 1) device_render_trap(device, &traps[0]);
        if (n >= 2) device_render_trap(device, &traps[1]);
    }

    if (render_state & RENDER_STATE_WIREFRAME) {		// 线框绘制
        device_draw_line(device, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, device->foreground);
        device_draw_line(device, (int)p1.x, (int)p1.y, (int)p3.x, (int)p3.y, device->foreground);
        device_draw_line(device, (int)p3.x, (int)p3.y, (int)p2.x, (int)p2.y, device->foreground);
    }
}
