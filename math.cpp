#include "mini3d.h"

// | v |
float vector_length(const vector_t *v) {
    float sq = v->x * v->x + v->y * v->y + v->z * v->z;
    return (float)sqrt(sq);
}

// z = x + y
void vector_add(vector_t *z, const vector_t *x, const vector_t *y) {
    z->x = x->x + y->x;
    z->y = x->y + y->y;
    z->z = x->z + y->z;
    z->w = 1.0;
}

// z = x - y
void vector_sub(vector_t *z, const vector_t *x, const vector_t *y) {
    z->x = x->x - y->x;
    z->y = x->y - y->y;
    z->z = x->z - y->z;
    z->w = 1.0;
}

// 矢量点乘
float vector_dotproduct(const vector_t *x, const vector_t *y) {
    return x->x * y->x + x->y * y->y + x->z * y->z;
}

// 矢量叉乘
void vector_crossproduct(vector_t *z, const vector_t *x, const vector_t *y) {
    float m1, m2, m3;
    m1 = x->y * y->z - x->z * y->y;
    m2 = x->z * y->x - x->x * y->z;
    m3 = x->x * y->y - x->y * y->x;
    z->x = m1;
    z->y = m2;
    z->z = m3;
    z->w = 1.0f;
}

// 矢量插值，t取值 [0, 1]
void vector_interp(vector_t *z, const vector_t *x1, const vector_t *x2, float t) {
    z->x = interp(x1->x, x2->x, t);
    z->y = interp(x1->y, x2->y, t);
    z->z = interp(x1->z, x2->z, t);
    z->w = 1.0f;
}

// 矢量归一化
void vector_normalize(vector_t *v) {
    float length = vector_length(v);
    if (length != 0.0f) {
        float inv = 1.0f / length;
        v->x *= inv;
        v->y *= inv;
        v->z *= inv;
    }
}


// c = a + b
void matrix_add(matrix_t *c, const matrix_t *a, const matrix_t *b) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            c->m[i][j] = a->m[i][j] + b->m[i][j];
    }
}

// c = a - b
void matrix_sub(matrix_t *c, const matrix_t *a, const matrix_t *b) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            c->m[i][j] = a->m[i][j] - b->m[i][j];
    }
}

// c = a * b
void matrix_mul(matrix_t *c, const matrix_t *a, const matrix_t *b) {
    matrix_t z;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            z.m[j][i] = (a->m[j][0] * b->m[0][i]) +
                (a->m[j][1] * b->m[1][i]) +
                (a->m[j][2] * b->m[2][i]) +
                (a->m[j][3] * b->m[3][i]);
        }
    }
    c[0] = z;
}

// c = a * f
void matrix_scale(matrix_t *c, const matrix_t *a, float f) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            c->m[i][j] = a->m[i][j] * f;
    }
}

// y = x * m
void matrix_apply(vector_t *y, const vector_t *x, const matrix_t *m) {
    float X = x->x, Y = x->y, Z = x->z, W = x->w;
    y->x = X * m->m[0][0] + Y * m->m[1][0] + Z * m->m[2][0] + W * m->m[3][0];
    y->y = X * m->m[0][1] + Y * m->m[1][1] + Z * m->m[2][1] + W * m->m[3][1];
    y->z = X * m->m[0][2] + Y * m->m[1][2] + Z * m->m[2][2] + W * m->m[3][2];
    y->w = X * m->m[0][3] + Y * m->m[1][3] + Z * m->m[2][3] + W * m->m[3][3];
}

void matrix_set_identity(matrix_t *m) {
    m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] = 1.0f;
    m->m[0][1] = m->m[0][2] = m->m[0][3] = 0.0f;
    m->m[1][0] = m->m[1][2] = m->m[1][3] = 0.0f;
    m->m[2][0] = m->m[2][1] = m->m[2][3] = 0.0f;
    m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
}

void matrix_set_zero(matrix_t *m) {
    m->m[0][0] = m->m[0][1] = m->m[0][2] = m->m[0][3] = 0.0f;
    m->m[1][0] = m->m[1][1] = m->m[1][2] = m->m[1][3] = 0.0f;
    m->m[2][0] = m->m[2][1] = m->m[2][2] = m->m[2][3] = 0.0f;
    m->m[3][0] = m->m[3][1] = m->m[3][2] = m->m[3][3] = 0.0f;
}

// 平移变换
void matrix_set_translate(matrix_t *m, float x, float y, float z) {
    matrix_set_identity(m);
    m->m[3][0] = x;
    m->m[3][1] = y;
    m->m[3][2] = z;
}

// 缩放变换
void matrix_set_scale(matrix_t *m, float x, float y, float z) {
    matrix_set_identity(m);
    m->m[0][0] = x;
    m->m[1][1] = y;
    m->m[2][2] = z;
}

// 旋转矩阵
void matrix_set_rotate(matrix_t *m, float x, float y, float z, float theta) {
    float qsin = (float)sin(theta * 0.5f);
    float qcos = (float)cos(theta * 0.5f);
    vector_t vec = { x, y, z, 1.0f };
    float w = qcos;
    vector_normalize(&vec);
    x = vec.x * qsin;
    y = vec.y * qsin;
    z = vec.z * qsin;
    m->m[0][0] = 1 - 2 * y * y - 2 * z * z;
    m->m[1][0] = 2 * x * y - 2 * w * z;
    m->m[2][0] = 2 * x * z + 2 * w * y;
    m->m[0][1] = 2 * x * y + 2 * w * z;
    m->m[1][1] = 1 - 2 * x * x - 2 * z * z;
    m->m[2][1] = 2 * y * z - 2 * w * x;
    m->m[0][2] = 2 * x * z - 2 * w * y;
    m->m[1][2] = 2 * y * z + 2 * w * x;
    m->m[2][2] = 1 - 2 * x * x - 2 * y * y;
    m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
    m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
    m->m[3][3] = 1.0f;
}

// 设置摄像机
void matrix_set_lookat(matrix_t *m, const vector_t *eye, const vector_t *at, const vector_t *up) {
    vector_t xaxis, yaxis, zaxis;

    vector_sub(&zaxis, at, eye);
    vector_normalize(&zaxis);
    vector_crossproduct(&xaxis, up, &zaxis);
    vector_normalize(&xaxis);
    vector_crossproduct(&yaxis, &zaxis, &xaxis);

    m->m[0][0] = xaxis.x;
    m->m[1][0] = xaxis.y;
    m->m[2][0] = xaxis.z;
    m->m[3][0] = -vector_dotproduct(&xaxis, eye);

    m->m[0][1] = yaxis.x;
    m->m[1][1] = yaxis.y;
    m->m[2][1] = yaxis.z;
    m->m[3][1] = -vector_dotproduct(&yaxis, eye);

    m->m[0][2] = zaxis.x;
    m->m[1][2] = zaxis.y;
    m->m[2][2] = zaxis.z;
    m->m[3][2] = -vector_dotproduct(&zaxis, eye);

    m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
    m->m[3][3] = 1.0f;
}

// D3DXMatrixPerspectiveFovLH
void matrix_set_perspective(matrix_t *m, float fovy, float aspect, float zn, float zf) {
    float fax = 1.0f / (float)tan(fovy * 0.5f);
    matrix_set_zero(m);
    m->m[0][0] = (float)(fax / aspect);
    m->m[1][1] = (float)(fax);
    m->m[2][2] = zf / (zf - zn);
    m->m[3][2] = -zn * zf / (zf - zn);
    m->m[2][3] = 1;
}
