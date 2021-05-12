#include "mini3d.h"

// 矩阵更新，计算 transform = world * view * projection
void transform_update(transform_t *ts) {
    matrix_t m;
    matrix_mul(&m, &ts->world, &ts->view);
    matrix_mul(&ts->transform, &m, &ts->projection);
}

// 初始化，设置屏幕长宽
void transform_init(transform_t *ts, int width, int height) {
    float aspect = (float)width / ((float)height);
    matrix_set_identity(&ts->world);
    matrix_set_identity(&ts->view);
    matrix_set_perspective(&ts->projection, 3.1415926f * 0.5f, aspect, 1.0f, 500.0f);
    ts->w = (float)width;
    ts->h = (float)height;
    transform_update(ts);
}

// 将矢量 x 进行 project 
void transform_apply(const transform_t *ts, vector_t *y, const vector_t *x) {
    matrix_apply(y, x, &ts->transform);
}

// 检查齐次坐标同 cvv 的边界用于视锥裁剪
int transform_check_cvv(const vector_t *v) {
    float w = v->w;
    int check = 0;
    if (v->z < 0.0f) check |= 1;
    if (v->z > w) check |= 2;
    if (v->x < -w) check |= 4;
    if (v->x > w) check |= 8;
    if (v->y < -w) check |= 16;
    if (v->y > w) check |= 32;
    return check;
}

// 归一化，得到屏幕坐标
void transform_homogenize(const transform_t *ts, vector_t *y, const vector_t *x) {
    float rhw = 1.0f / x->w;
    y->x = (x->x * rhw + 1.0f) * ts->w * 0.5f;
    y->y = (1.0f - x->y * rhw) * ts->h * 0.5f;
    y->z = x->z * rhw;
    y->w = 1.0f;
}

void vertex_rhw_init(vertex_t *v) {
    float rhw = 1.0f / v->pos.w;
    v->rhw = rhw;
    v->tc.u *= rhw;
    v->tc.v *= rhw;
    v->color.r *= rhw;
    v->color.g *= rhw;
    v->color.b *= rhw;
}

void vertex_interp(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float t) {
    vector_interp(&y->pos, &x1->pos, &x2->pos, t);
    y->tc.u = interp(x1->tc.u, x2->tc.u, t);
    y->tc.v = interp(x1->tc.v, x2->tc.v, t);
    y->color.r = interp(x1->color.r, x2->color.r, t);
    y->color.g = interp(x1->color.g, x2->color.g, t);
    y->color.b = interp(x1->color.b, x2->color.b, t);
    y->rhw = interp(x1->rhw, x2->rhw, t);
}

void vertex_division(vertex_t *y, const vertex_t *x1, const vertex_t *x2, float w) {
    float inv = 1.0f / w;
    y->pos.x = (x2->pos.x - x1->pos.x) * inv;
    y->pos.y = (x2->pos.y - x1->pos.y) * inv;
    y->pos.z = (x2->pos.z - x1->pos.z) * inv;
    y->pos.w = (x2->pos.w - x1->pos.w) * inv;
    y->tc.u = (x2->tc.u - x1->tc.u) * inv;
    y->tc.v = (x2->tc.v - x1->tc.v) * inv;
    y->color.r = (x2->color.r - x1->color.r) * inv;
    y->color.g = (x2->color.g - x1->color.g) * inv;
    y->color.b = (x2->color.b - x1->color.b) * inv;
    y->rhw = (x2->rhw - x1->rhw) * inv;
}

void vertex_add(vertex_t *y, const vertex_t *x) {
    y->pos.x += x->pos.x;
    y->pos.y += x->pos.y;
    y->pos.z += x->pos.z;
    y->pos.w += x->pos.w;
    y->rhw += x->rhw;
    y->tc.u += x->tc.u;
    y->tc.v += x->tc.v;
    y->color.r += x->color.r;
    y->color.g += x->color.g;
    y->color.b += x->color.b;
}

// 根据三角形生成 0-2 个梯形，并且返回合法梯形的数量
int trapezoid_init_triangle(trapezoid_t *trap, const vertex_t *p1,
    const vertex_t *p2, const vertex_t *p3) {
    const vertex_t *p;
    float k, x;

    if (p1->pos.y > p2->pos.y) p = p1, p1 = p2, p2 = p;
    if (p1->pos.y > p3->pos.y) p = p1, p1 = p3, p3 = p;
    if (p2->pos.y > p3->pos.y) p = p2, p2 = p3, p3 = p;
    if (p1->pos.y == p2->pos.y && p1->pos.y == p3->pos.y) return 0;
    if (p1->pos.x == p2->pos.x && p1->pos.x == p3->pos.x) return 0;

    if (p1->pos.y == p2->pos.y) { // triangle down
        if (p1->pos.x > p2->pos.x) p = p1, p1 = p2, p2 = p;
        trap[0].top = p1->pos.y;
        trap[0].bottom = p3->pos.y;
        trap[0].left.v1 = *p1;
        trap[0].left.v2 = *p3;
        trap[0].right.v1 = *p2;
        trap[0].right.v2 = *p3;
        return (trap[0].top < trap[0].bottom) ? 1 : 0;
    }

    if (p2->pos.y == p3->pos.y) { // triangle up
        if (p2->pos.x > p3->pos.x) p = p2, p2 = p3, p3 = p;
        trap[0].top = p1->pos.y;
        trap[0].bottom = p3->pos.y;
        trap[0].left.v1 = *p1;
        trap[0].left.v2 = *p2;
        trap[0].right.v1 = *p1;
        trap[0].right.v2 = *p3;
        return (trap[0].top < trap[0].bottom) ? 1 : 0;
    }

    trap[0].top = p1->pos.y;
    trap[0].bottom = p2->pos.y;
    trap[1].top = p2->pos.y;
    trap[1].bottom = p3->pos.y;

    k = (p3->pos.y - p1->pos.y) / (p2->pos.y - p1->pos.y);
    x = p1->pos.x + (p2->pos.x - p1->pos.x) * k;

    if (x <= p3->pos.x) { // triangle left
        trap[0].left.v1 = *p1;
        trap[0].left.v2 = *p2;
        trap[0].right.v1 = *p1;
        trap[0].right.v2 = *p3;
        trap[1].left.v1 = *p2;
        trap[1].left.v2 = *p3;
        trap[1].right.v1 = *p1;
        trap[1].right.v2 = *p3;
    }
    else { // triangle right
        trap[0].left.v1 = *p1;
        trap[0].left.v2 = *p3;
        trap[0].right.v1 = *p1;
        trap[0].right.v2 = *p2;
        trap[1].left.v1 = *p1;
        trap[1].left.v2 = *p3;
        trap[1].right.v1 = *p2;
        trap[1].right.v2 = *p3;
    }

    return 2;
}

// 按照 Y 坐标计算出左右两条边纵坐标等于 Y 的顶点
void trapezoid_edge_interp(trapezoid_t *trap, float y) {
    float s1 = trap->left.v2.pos.y - trap->left.v1.pos.y;
    float s2 = trap->right.v2.pos.y - trap->right.v1.pos.y;
    float t1 = (y - trap->left.v1.pos.y) / s1;
    float t2 = (y - trap->right.v1.pos.y) / s2;
    vertex_interp(&trap->left.v, &trap->left.v1, &trap->left.v2, t1);
    vertex_interp(&trap->right.v, &trap->right.v1, &trap->right.v2, t2);
}

// 根据左右两边的端点，初始化计算出扫描线的起点和步长
void trapezoid_init_scan_line(const trapezoid_t *trap, scanline_t *scanline, int y) {
    float width = trap->right.v.pos.x - trap->left.v.pos.x;
    scanline->x = (int)(trap->left.v.pos.x + 0.5f);
    scanline->w = (int)(trap->right.v.pos.x + 0.5f) - scanline->x;
    scanline->y = y;
    scanline->v = trap->left.v;
    if (trap->left.v.pos.x >= trap->right.v.pos.x) scanline->w = 0;
    vertex_division(&scanline->step, &trap->left.v, &trap->right.v, width);
}

