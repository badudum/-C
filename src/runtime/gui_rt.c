/*
 * gui_rt.c — from-scratch GUI runtime for minusC. No third-party dependencies.
 *
 * macOS: window + input via the Objective-C runtime (objc_msgSend straight
 * into AppKit), pixels via a software framebuffer presented on the content
 * view's CALayer with CoreGraphics. Text uses an embedded public-domain
 * 8x8 bitmap font (Marcel Sondaar / Daniel Hepper, public domain).
 *
 * Other platforms: stubs returning -1 (GUI not supported yet).
 *
 * minusC-facing API (all ints):
 *   GuiOpen(w, h, title)          -> 0 ok, -1 fail
 *   GuiClose()                    -> 0
 *   GuiClear(rgb)                 -> 0
 *   GuiRect(x, y, w, h, rgb)      -> 0
 *   GuiText(x, y, str, rgb, k)    -> 0 (k = integer scale, 8*k px glyphs)
 *   GuiPresent()                  -> 0
 *   GuiPoll()                     -> 0 none, 1 close, 2 keydown,
 *                                    3 mousedown, 4 mouseup, 5 mousemove
 *   GuiEventX() / GuiEventY()     -> last event position
 *   GuiEventKey()                 -> last key (ASCII of pressed char)
 *   GuiSleep(ms)                  -> 0
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include "../include/gui_mtl.h"
#endif

/* ---------------- embedded 8x8 font, ASCII 32..126 (public domain) ------- */

static const unsigned char mc_font8x8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, /* ! */
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, /* " */
    {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00}, /* # */
    {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00}, /* $ */
    {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00}, /* % */
    {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00}, /* & */
    {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00}, /* ' */
    {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00}, /* ( */
    {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00}, /* ) */
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, /* * */
    {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00}, /* + */
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06}, /* , */
    {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00}, /* - */
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00}, /* . */
    {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, /* / */
    {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00}, /* 0 */
    {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00}, /* 1 */
    {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00}, /* 2 */
    {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00}, /* 3 */
    {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00}, /* 4 */
    {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00}, /* 5 */
    {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00}, /* 6 */
    {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00}, /* 7 */
    {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00}, /* 8 */
    {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00}, /* 9 */
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00}, /* : */
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06}, /* ; */
    {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00}, /* < */
    {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00}, /* = */
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, /* > */
    {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00}, /* ? */
    {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00}, /* @ */
    {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00}, /* A */
    {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00}, /* B */
    {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00}, /* C */
    {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00}, /* D */
    {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00}, /* E */
    {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00}, /* F */
    {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00}, /* G */
    {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00}, /* H */
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, /* I */
    {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00}, /* J */
    {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00}, /* K */
    {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00}, /* L */
    {0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00}, /* M */
    {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00}, /* N */
    {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00}, /* O */
    {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00}, /* P */
    {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00}, /* Q */
    {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00}, /* R */
    {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00}, /* S */
    {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, /* T */
    {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00}, /* U */
    {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00}, /* V */
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, /* W */
    {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00}, /* X */
    {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00}, /* Y */
    {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00}, /* Z */
    {0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00}, /* [ */
    {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, /* \ */
    {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00}, /* ] */
    {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00}, /* ^ */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, /* _ */
    {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, /* ` */
    {0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00}, /* a */
    {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00}, /* b */
    {0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00}, /* c */
    {0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0x00}, /* d */
    {0x00,0x00,0x1E,0x33,0x3F,0x03,0x1E,0x00}, /* e */
    {0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00}, /* f */
    {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F}, /* g */
    {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00}, /* h */
    {0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00}, /* i */
    {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E}, /* j */
    {0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00}, /* k */
    {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, /* l */
    {0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00}, /* m */
    {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00}, /* n */
    {0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00}, /* o */
    {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F}, /* p */
    {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78}, /* q */
    {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00}, /* r */
    {0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00}, /* s */
    {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00}, /* t */
    {0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00}, /* u */
    {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00}, /* v */
    {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, /* w */
    {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00}, /* x */
    {0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F}, /* y */
    {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00}, /* z */
    {0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00}, /* { */
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, /* | */
    {0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00}, /* } */
    {0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00}, /* ~ */
};

/* ---------------- shared software framebuffer state ---------------------- */

static uint32_t *mc_fb;
static int mc_fb_w;
static int mc_fb_h;
static int mc_ev_x;
static int mc_ev_y;
static int mc_ev_key;
static int mc_clear_rgb;

static void mc_fb_clear(int color)
{
    if (!mc_fb)
        return;
    uint32_t c = (uint32_t)color;
    long n = (long)mc_fb_w * mc_fb_h;
    for (long i = 0; i < n; i++)
        mc_fb[i] = c;
}

static void mc_fb_rect(int x, int y, int w, int h, int color)
{
    if (!mc_fb)
        return;
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w;
    int y1 = y + h;
    if (x1 > mc_fb_w) x1 = mc_fb_w;
    if (y1 > mc_fb_h) y1 = mc_fb_h;
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            mc_fb[(long)yy * mc_fb_w + xx] = (uint32_t)color;
}

static void mc_fb_text(int x, int y, const char *s, int color, int scale)
{
    if (!mc_fb || !s)
        return;
    if (scale < 1)
        scale = 1;
    int pen = x;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c < 32 || c > 126) {
            pen += 8 * scale;
            continue;
        }
        const unsigned char *g = mc_font8x8[c - 32];
        for (int gy = 0; gy < 8; gy++) {
            unsigned char row = g[gy];
            for (int gx = 0; gx < 8; gx++) {
                if (!(row & (1u << gx)))
                    continue;
                mc_fb_rect(pen + gx * scale, y + gy * scale, scale, scale, color);
            }
        }
        pen += 8 * scale;
    }
}

/* ---------------- GPU batches + 3D (Metal on macOS) ---------------------- */

#ifdef __APPLE__
static int mc_use_mtl;

static mc_mtl_vtx *mc_v3;
static int mc_n3, mc_cap3;
static mc_mtl_vtx *mc_v2;
static int mc_n2, mc_cap2;

typedef struct {
    float minx, miny, minz, maxx, maxy, maxz;
    int id;
} mc_aabb;

static mc_aabb *mc_hits;
static int mc_nhit, mc_caphit;
static int mc_cur_id;
static float mc_mvp[16];
static int mc_have_cam;
static float mc_eye[3] = {0, 560, 900};
static float mc_look[3] = {0, 0, 40};
static float mc_light[3] = {0.45f, 0.85f, 0.35f};
static float mc_cam_radius = 1126.0f;
static float mc_cam_yaw;
static float mc_cam_pitch = 0.55f;
static int mc_orbit_user;
static int mc_orbit_drag;

static void mc_grow_vtx(mc_mtl_vtx **v, int *cap, int need)
{
    if (need <= *cap)
        return;
    int n = *cap ? *cap : 8192;
    while (n < need)
        n *= 2;
    *v = (mc_mtl_vtx *)realloc(*v, (size_t)n * sizeof(mc_mtl_vtx));
    *cap = n;
}

static void mc_rgb(int color, float *r, float *g, float *b)
{
    *r = ((color >> 16) & 255) / 255.0f;
    *g = ((color >> 8) & 255) / 255.0f;
    *b = (color & 255) / 255.0f;
}

static void mc_push3(float x, float y, float z,
                     float nx, float ny, float nz, int color)
{
    mc_grow_vtx(&mc_v3, &mc_cap3, mc_n3 + 1);
    mc_mtl_vtx *v = &mc_v3[mc_n3++];
    float r, g, b;
    mc_rgb(color, &r, &g, &b);
    v->x = x; v->y = y; v->z = z;
    v->nx = nx; v->ny = ny; v->nz = nz;
    v->r = r; v->g = g; v->b = b; v->a = 1;
}

static void mc_push2(float x, float y, int color)
{
    mc_grow_vtx(&mc_v2, &mc_cap2, mc_n2 + 1);
    mc_mtl_vtx *v = &mc_v2[mc_n2++];
    float r, g, b;
    mc_rgb(color, &r, &g, &b);
    v->x = x; v->y = y; v->z = 0;
    v->nx = 0; v->ny = 0; v->nz = 1;
    v->r = r; v->g = g; v->b = b; v->a = 1;
}

static void mc_quad2(int x, int y, int w, int h, int color)
{
    float x0 = (float)x, y0 = (float)y;
    float x1 = (float)(x + w), y1 = (float)(y + h);
    mc_push2(x0, y0, color);
    mc_push2(x0, y1, color);
    mc_push2(x1, y1, color);
    mc_push2(x0, y0, color);
    mc_push2(x1, y1, color);
    mc_push2(x1, y0, color);
}

static void mc_tri3(float ax, float ay, float az,
                    float bx, float by, float bz,
                    float cx, float cy, float cz,
                    float nx, float ny, float nz, int color)
{
    mc_push3(ax, ay, az, nx, ny, nz, color);
    mc_push3(bx, by, bz, nx, ny, nz, color);
    mc_push3(cx, cy, cz, nx, ny, nz, color);
}

static void mc_face(float ax, float ay, float az,
                    float bx, float by, float bz,
                    float cx, float cy, float cz,
                    float dx, float dy, float dz,
                    float nx, float ny, float nz, int color)
{
    mc_tri3(ax, ay, az, bx, by, bz, cx, cy, cz, nx, ny, nz, color);
    mc_tri3(ax, ay, az, cx, cy, cz, dx, dy, dz, nx, ny, nz, color);
}

static void mc_m4_mul(float *o, const float *a, const float *b)
{
    float t[16];
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            t[c * 4 + r] =
                a[0 * 4 + r] * b[c * 4 + 0] +
                a[1 * 4 + r] * b[c * 4 + 1] +
                a[2 * 4 + r] * b[c * 4 + 2] +
                a[3 * 4 + r] * b[c * 4 + 3];
        }
    }
    memcpy(o, t, sizeof(t));
}

static void mc_lookat(float *out, float ex, float ey, float ez,
                      float cx, float cy, float cz)
{
    float fx = cx - ex, fy = cy - ey, fz = cz - ez;
    float fl = sqrtf(fx * fx + fy * fy + fz * fz);
    if (fl < 1e-6f)
        fl = 1;
    fx /= fl; fy /= fl; fz /= fl;
    float ux = 0, uy = 1, uz = 0;
    float rx = fy * uz - fz * uy;
    float ry = fz * ux - fx * uz;
    float rz = fx * uy - fy * ux;
    float rl = sqrtf(rx * rx + ry * ry + rz * rz);
    if (rl < 1e-6f)
        rl = 1;
    rx /= rl; ry /= rl; rz /= rl;
    ux = ry * fz - rz * fy;
    uy = rz * fx - rx * fz;
    uz = rx * fy - ry * fx;
    float v[16] = {
        rx, ux, -fx, 0,
        ry, uy, -fy, 0,
        rz, uz, -fz, 0,
        -(rx * ex + ry * ey + rz * ez),
        -(ux * ex + uy * ey + uz * ez),
        -(-fx * ex + -fy * ey + -fz * ez),
        1
    };
    memcpy(out, v, sizeof(v));
}

static void mc_perspective(float *out, float fov_deg, float aspect,
                           float zn, float zf)
{
    float f = 1.0f / tanf(fov_deg * 0.01745329252f * 0.5f);
    memset(out, 0, 16 * sizeof(float));
    out[0] = f / aspect;
    out[5] = f;
    out[10] = zf / (zn - zf);
    out[11] = -1.0f;
    out[14] = (zf * zn) / (zn - zf);
}

static void mc_rebuild_cam(void);

static void mc_eye_from_orbit(void)
{
    float cp = cosf(mc_cam_pitch);
    float sp = sinf(mc_cam_pitch);
    float cy = cosf(mc_cam_yaw);
    float sy = sinf(mc_cam_yaw);
    mc_eye[0] = mc_look[0] + mc_cam_radius * cp * sy;
    mc_eye[1] = mc_look[1] + mc_cam_radius * sp;
    mc_eye[2] = mc_look[2] + mc_cam_radius * cp * cy;
}

static void mc_orbit_from_eye(void)
{
    float dx = mc_eye[0] - mc_look[0];
    float dy = mc_eye[1] - mc_look[1];
    float dz = mc_eye[2] - mc_look[2];
    float r = sqrtf(dx * dx + dy * dy + dz * dz);
    if (r < 8.0f)
        r = 8.0f;
    mc_cam_radius = r;
    mc_cam_yaw = atan2f(dx, dz);
    float p = asinf(dy / r);
    if (p > 1.45f) p = 1.45f;
    if (p < -0.08f) p = -0.08f;
    mc_cam_pitch = p;
}

static void mc_orbit_drag_by(float dx, float dy)
{
    mc_orbit_user = 1;
    mc_cam_yaw -= dx * 0.0075f;
    mc_cam_pitch += dy * 0.0075f;
    if (mc_cam_pitch > 1.45f)
        mc_cam_pitch = 1.45f;
    if (mc_cam_pitch < -0.08f)
        mc_cam_pitch = -0.08f;
    mc_eye_from_orbit();
    mc_rebuild_cam();
}

static void mc_rebuild_cam(void)
{
    float view[16], proj[16];
    float aspect = (mc_fb_h > 0) ? (float)mc_fb_w / (float)mc_fb_h : 1.0f;
    mc_lookat(view, mc_eye[0], mc_eye[1], mc_eye[2],
              mc_look[0], mc_look[1], mc_look[2]);
    mc_perspective(proj, 50.0f, aspect, 8.0f, 8000.0f);
    mc_m4_mul(mc_mvp, proj, view);
    mc_have_cam = 1;
}

static int mc_m4_inv(float *o, const float *m)
{
    float inv[16];
    inv[0] = m[5]*m[10]*m[15]-m[5]*m[11]*m[14]-m[9]*m[6]*m[15]+m[9]*m[7]*m[14]+m[13]*m[6]*m[11]-m[13]*m[7]*m[10];
    inv[4] = -m[4]*m[10]*m[15]+m[4]*m[11]*m[14]+m[8]*m[6]*m[15]-m[8]*m[7]*m[14]-m[12]*m[6]*m[11]+m[12]*m[7]*m[10];
    inv[8] = m[4]*m[9]*m[15]-m[4]*m[11]*m[13]-m[8]*m[5]*m[15]+m[8]*m[7]*m[13]+m[12]*m[5]*m[11]-m[12]*m[7]*m[9];
    inv[12]= -m[4]*m[9]*m[14]+m[4]*m[10]*m[13]+m[8]*m[5]*m[14]-m[8]*m[6]*m[13]-m[12]*m[5]*m[10]+m[12]*m[6]*m[9];
    inv[1] = -m[1]*m[10]*m[15]+m[1]*m[11]*m[14]+m[9]*m[2]*m[15]-m[9]*m[3]*m[14]-m[13]*m[2]*m[11]+m[13]*m[3]*m[10];
    inv[5] = m[0]*m[10]*m[15]-m[0]*m[11]*m[14]-m[8]*m[2]*m[15]+m[8]*m[3]*m[14]+m[12]*m[2]*m[11]-m[12]*m[3]*m[10];
    inv[9] = -m[0]*m[9]*m[15]+m[0]*m[11]*m[13]+m[8]*m[1]*m[15]-m[8]*m[3]*m[13]-m[12]*m[1]*m[11]+m[12]*m[3]*m[9];
    inv[13]= m[0]*m[9]*m[14]-m[0]*m[10]*m[13]-m[8]*m[1]*m[14]+m[8]*m[2]*m[13]+m[12]*m[1]*m[10]-m[12]*m[2]*m[9];
    inv[2] = m[1]*m[6]*m[15]-m[1]*m[7]*m[14]-m[5]*m[2]*m[15]+m[5]*m[3]*m[14]+m[13]*m[2]*m[7]-m[13]*m[3]*m[6];
    inv[6] = -m[0]*m[6]*m[15]+m[0]*m[7]*m[14]+m[4]*m[2]*m[15]-m[4]*m[3]*m[14]-m[12]*m[2]*m[7]+m[12]*m[3]*m[6];
    inv[10]= m[0]*m[5]*m[15]-m[0]*m[7]*m[13]-m[4]*m[1]*m[15]+m[4]*m[3]*m[13]+m[12]*m[1]*m[7]-m[12]*m[3]*m[5];
    inv[14]= -m[0]*m[5]*m[14]+m[0]*m[6]*m[13]+m[4]*m[1]*m[14]-m[4]*m[2]*m[13]-m[12]*m[1]*m[6]+m[12]*m[2]*m[5];
    inv[3] = -m[1]*m[6]*m[11]+m[1]*m[7]*m[10]+m[5]*m[2]*m[11]-m[5]*m[3]*m[10]-m[9]*m[2]*m[7]+m[9]*m[3]*m[6];
    inv[7] = m[0]*m[6]*m[11]-m[0]*m[7]*m[10]-m[4]*m[2]*m[11]+m[4]*m[3]*m[10]+m[8]*m[2]*m[7]-m[8]*m[3]*m[6];
    inv[11]= -m[0]*m[5]*m[11]+m[0]*m[7]*m[9]+m[4]*m[1]*m[11]-m[4]*m[3]*m[9]-m[8]*m[1]*m[7]+m[8]*m[3]*m[5];
    inv[15]= m[0]*m[5]*m[10]-m[0]*m[6]*m[9]-m[4]*m[1]*m[10]+m[4]*m[2]*m[9]+m[8]*m[1]*m[6]-m[8]*m[2]*m[5];
    float det = m[0]*inv[0]+m[1]*inv[4]+m[2]*inv[8]+m[3]*inv[12];
    if (det > -1e-8f && det < 1e-8f)
        return 0;
    det = 1.0f / det;
    for (int i = 0; i < 16; i++)
        o[i] = inv[i] * det;
    return 1;
}

static void mc_xf(const float *m, float x, float y, float z, float w,
                  float *ox, float *oy, float *oz, float *ow)
{
    *ox = m[0]*x + m[4]*y + m[8]*z + m[12]*w;
    *oy = m[1]*x + m[5]*y + m[9]*z + m[13]*w;
    *oz = m[2]*x + m[6]*y + m[10]*z + m[14]*w;
    *ow = m[3]*x + m[7]*y + m[11]*z + m[15]*w;
}

static int mc_ray_aabb(float ox, float oy, float oz,
                       float dx, float dy, float dz, const mc_aabb *b, float *t_out)
{
    float tmin = 0.0f, tmax = 1e9f;
    float orig[3] = {ox, oy, oz};
    float dir[3] = {dx, dy, dz};
    float bmin[3] = {b->minx, b->miny, b->minz};
    float bmax[3] = {b->maxx, b->maxy, b->maxz};
    for (int i = 0; i < 3; i++) {
        if (dir[i] > -1e-8f && dir[i] < 1e-8f) {
            if (orig[i] < bmin[i] || orig[i] > bmax[i])
                return 0;
            continue;
        }
        float t1 = (bmin[i] - orig[i]) / dir[i];
        float t2 = (bmax[i] - orig[i]) / dir[i];
        if (t1 > t2) {
            float tmp = t1; t1 = t2; t2 = tmp;
        }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax)
            return 0;
    }
    if (tmax < 0)
        return 0;
    *t_out = tmin >= 0 ? tmin : tmax;
    return 1;
}

static void mc_batch_reset(void)
{
    mc_n3 = 0;
    mc_n2 = 0;
    mc_nhit = 0;
    mc_cur_id = -1;
}
#endif

#ifdef __APPLE__

/* ---------------- macOS backend: raw Objective-C runtime ----------------- */

#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#include <unistd.h>

static id mc_app;
static id mc_win;
static id mc_layer;
static id mc_loop_mode;
static int mc_open;

static id mc_cls(const char *n) { return (id)objc_getClass(n); }
static SEL mc_sel(const char *n) { return sel_registerName(n); }

typedef id (*mc_msg)(id, SEL);
typedef void (*mc_msg_v)(id, SEL);
typedef id (*mc_msg_id)(id, SEL, id);
typedef void (*mc_msg_vid)(id, SEL, id);
typedef void (*mc_msg_vlong)(id, SEL, long);
typedef id (*mc_msg_str)(id, SEL, const char *);

static id mc_nsstring(const char *s)
{
    return ((mc_msg_str)objc_msgSend)(mc_cls("NSString"),
                                      mc_sel("stringWithUTF8String:"), s);
}

int GuiOpen(int w, int h, const char *title)
{
    if (mc_open)
        return -1;
    if (w < 32 || h < 32 || w > 4096 || h > 4096)
        return -1;

    mc_fb = (uint32_t *)calloc((size_t)w * h, 4);
    if (!mc_fb)
        return -1;
    mc_fb_w = w;
    mc_fb_h = h;

    mc_app = ((mc_msg)objc_msgSend)(mc_cls("NSApplication"),
                                    mc_sel("sharedApplication"));
    ((mc_msg_vlong)objc_msgSend)(mc_app, mc_sel("setActivationPolicy:"), 0);
    ((mc_msg_v)objc_msgSend)(mc_app, mc_sel("finishLaunching"));

    id win_alloc = ((mc_msg)objc_msgSend)(mc_cls("NSWindow"), mc_sel("alloc"));
    CGRect frame = CGRectMake(0, 0, (CGFloat)w, (CGFloat)h);
    /* styleMask 7 = titled | closable | miniaturizable */
    mc_win = ((id (*)(id, SEL, CGRect, unsigned long, unsigned long, int))objc_msgSend)(
        win_alloc, mc_sel("initWithContentRect:styleMask:backing:defer:"),
        frame, 7UL, 2UL, 0);
    if (!mc_win) {
        free(mc_fb);
        mc_fb = 0;
        return -1;
    }
    ((mc_msg_vid)objc_msgSend)(mc_win, mc_sel("setTitle:"),
                               mc_nsstring(title ? title : "minusC"));
    /* keep our handle valid after the user clicks close */
    ((void (*)(id, SEL, int))objc_msgSend)(mc_win,
                                           mc_sel("setReleasedWhenClosed:"), 0);
    ((mc_msg_v)objc_msgSend)(mc_win, mc_sel("center"));
    ((mc_msg_vid)objc_msgSend)(mc_win, mc_sel("makeKeyAndOrderFront:"), 0);

    id view = ((mc_msg)objc_msgSend)(mc_win, mc_sel("contentView"));
    ((void (*)(id, SEL, int))objc_msgSend)(view, mc_sel("setWantsLayer:"), 1);
    mc_layer = ((mc_msg)objc_msgSend)(view, mc_sel("layer"));

    mc_use_mtl = 0;
    if (mc_mtl_init((void *)view, w, h) == 0)
        mc_use_mtl = 1;

    ((void (*)(id, SEL, int))objc_msgSend)(mc_app,
        mc_sel("activateIgnoringOtherApps:"), 1);

    mc_loop_mode = mc_nsstring("kCFRunLoopDefaultMode");
    mc_open = 1;
    mc_ev_x = 0;
    mc_ev_y = 0;
    mc_ev_key = 0;
    mc_orbit_user = 0;
    mc_orbit_drag = 0;
    return 0;
}

int GuiClose(void)
{
    if (!mc_open)
        return 0;
    ((mc_msg_vid)objc_msgSend)(mc_win, mc_sel("close"), 0);
    if (mc_use_mtl)
        mc_mtl_shutdown();
    mc_use_mtl = 0;
    free(mc_fb);
    mc_fb = 0;
    mc_open = 0;
    return 0;
}

int GuiPresent(void)
{
    if (!mc_open || !mc_fb)
        return -1;
    if (mc_use_mtl) {
        if (!mc_have_cam)
            mc_rebuild_cam();
        mc_mtl_begin(mc_clear_rgb);
        mc_mtl_set_mvp(mc_mvp);
        mc_mtl_set_light(mc_light[0], mc_light[1], mc_light[2]);
        mc_mtl_draw3d(mc_v3, mc_n3);
        mc_mtl_draw2d(mc_v2, mc_n2);
        if (mc_mtl_present() == 0) {
            mc_mtl_readback(mc_fb, mc_fb_w, mc_fb_h);
            return 0;
        }
    }
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef dp = CGDataProviderCreateWithData(
        NULL, mc_fb, (size_t)mc_fb_w * mc_fb_h * 4, NULL);
    CGImageRef img = CGImageCreate(
        (size_t)mc_fb_w, (size_t)mc_fb_h, 8, 32, (size_t)mc_fb_w * 4, cs,
        (CGBitmapInfo)kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little,
        dp, NULL, 0, kCGRenderingIntentDefault);
    if (img) {
        ((mc_msg_vid)objc_msgSend)(mc_layer, mc_sel("setContents:"), (id)img);
        ((mc_msg_v)objc_msgSend)(mc_cls("CATransaction"), mc_sel("flush"));
        CGImageRelease(img);
    }
    CGDataProviderRelease(dp);
    CGColorSpaceRelease(cs);
    return 0;
}

int GuiPoll(void)
{
    if (!mc_open)
        return 1;

    id past = ((mc_msg)objc_msgSend)(mc_cls("NSDate"), mc_sel("distantPast"));
    id ev = ((id (*)(id, SEL, unsigned long, id, id, int))objc_msgSend)(
        mc_app, mc_sel("nextEventMatchingMask:untilDate:inMode:dequeue:"),
        ~0UL, past, mc_loop_mode, 1);

    if (!ev) {
        signed char vis = ((signed char (*)(id, SEL))objc_msgSend)(
            mc_win, mc_sel("isVisible"));
        if (!vis)
            return 1;
        ((mc_msg_v)objc_msgSend)(mc_app, mc_sel("updateWindows"));
        return 0;
    }

    unsigned long t = ((unsigned long (*)(id, SEL))objc_msgSend)(
        ev, mc_sel("type"));

    if (t == 1 || t == 2 || t == 3 || t == 4 || t == 5 || t == 6 ||
        t == 7 || t == 25 || t == 26 || t == 27) {
        CGPoint p = ((CGPoint (*)(id, SEL))objc_msgSend)(
            ev, mc_sel("locationInWindow"));
        mc_ev_x = (int)p.x;
        mc_ev_y = mc_fb_h - 1 - (int)p.y;
        if (mc_ev_x < 0) mc_ev_x = 0;
        if (mc_ev_y < 0) mc_ev_y = 0;
        if (mc_ev_x >= mc_fb_w) mc_ev_x = mc_fb_w - 1;
        if (mc_ev_y >= mc_fb_h) mc_ev_y = mc_fb_h - 1;
    }

    int out = 0;
    if (t == 10) { /* keyDown: capture, do not forward (avoids beep) */
        id chars = ((mc_msg)objc_msgSend)(ev,
                                          mc_sel("charactersIgnoringModifiers"));
        if (chars) {
            const char *u = ((const char *(*)(id, SEL))objc_msgSend)(
                chars, mc_sel("UTF8String"));
            mc_ev_key = (u && u[0]) ? (int)(unsigned char)u[0] : 0;
        }
        return 2;
    }
    /* Middle (other) or right-button drag orbits the 3D camera. */
    if (t == 25 || t == 3)
        mc_orbit_drag = 1;
    if (t == 26 || t == 4)
        mc_orbit_drag = 0;
    if (mc_orbit_drag && (t == 7 || t == 27 || t == 5 || t == 6)) {
        CGFloat dx = ((CGFloat (*)(id, SEL))objc_msgSend)(ev, mc_sel("deltaX"));
        CGFloat dy = ((CGFloat (*)(id, SEL))objc_msgSend)(ev, mc_sel("deltaY"));
        mc_orbit_drag_by((float)dx, (float)dy);
        out = 8;
        ((mc_msg_v)objc_msgSend)(mc_app, mc_sel("updateWindows"));
        return out;
    }
    if (t == 25 || t == 26 || t == 3 || t == 4) {
        /* swallow so a right/middle click does not select a piece */
        ((mc_msg_v)objc_msgSend)(mc_app, mc_sel("updateWindows"));
        return 0;
    }
    if (t == 1)
        out = 3;
    else if (t == 2)
        out = 4;
    else if (t == 5 || t == 6)
        out = 5;

    ((mc_msg_vid)objc_msgSend)(mc_app, mc_sel("sendEvent:"), ev);
    ((mc_msg_v)objc_msgSend)(mc_app, mc_sel("updateWindows"));

    signed char vis = ((signed char (*)(id, SEL))objc_msgSend)(
        mc_win, mc_sel("isVisible"));
    if (!vis)
        return 1;
    return out;
}

int GuiSleep(int ms)
{
    if (ms > 0)
        usleep((useconds_t)ms * 1000);
    return 0;
}

#else /* !__APPLE__ — stubs until an X11/Wayland backend exists */

int GuiOpen(int w, int h, const char *title)
{
    (void)w; (void)h; (void)title;
    return -1;
}
int GuiClose(void) { return 0; }
int GuiPresent(void) { return -1; }
int GuiPoll(void) { return 1; }
int GuiSleep(int ms) { (void)ms; return 0; }

#endif

/* ---------------- platform-independent drawing entry points -------------- */

int GuiClear(int color)
{
    if (!mc_fb)
        return -1;
    mc_fb_clear(color);
    mc_clear_rgb = color;
#ifdef __APPLE__
    mc_batch_reset();
#endif
    return 0;
}

int GuiRect(int x, int y, int w, int h, int color)
{
    if (!mc_fb)
        return -1;
    mc_fb_rect(x, y, w, h, color);
#ifdef __APPLE__
    if (mc_use_mtl)
        mc_quad2(x, y, w, h, color);
#endif
    return 0;
}

int GuiText(int x, int y, const char *s, int color, int scale)
{
    if (!mc_fb)
        return -1;
    mc_fb_text(x, y, s, color, scale);
#ifdef __APPLE__
    if (mc_use_mtl && s) {
        if (scale < 1)
            scale = 1;
        int pen = x;
        for (; *s; s++) {
            unsigned char c = (unsigned char)*s;
            if (c < 32 || c > 126) {
                pen += 8 * scale;
                continue;
            }
            const unsigned char *g = mc_font8x8[c - 32];
            for (int gy = 0; gy < 8; gy++) {
                unsigned char row = g[gy];
                for (int gx = 0; gx < 8; gx++) {
                    if (!(row & (1u << gx)))
                        continue;
                    mc_quad2(pen + gx * scale, y + gy * scale, scale, scale, color);
                }
            }
            pen += 8 * scale;
        }
    }
#endif
    return 0;
}

int GuiEventX(void) { return mc_ev_x; }
int GuiEventY(void) { return mc_ev_y; }
int GuiEventKey(void) { return mc_ev_key; }

int GuiMask(int x, int y, const char *mask, int cols, int color, int scale)
{
    if (!mc_fb || !mask || cols <= 0)
        return -1;
    if (scale < 1)
        scale = 1;
    int cx = 0, cy = 0;
    for (const char *p = mask; *p; p++) {
        if (*p == '#') {
            mc_fb_rect(x + cx * scale, y + cy * scale, scale, scale, color);
#ifdef __APPLE__
            if (mc_use_mtl)
                mc_quad2(x + cx * scale, y + cy * scale, scale, scale, color);
#endif
        }
        cx++;
        if (cx == cols) {
            cx = 0;
            cy++;
        }
    }
    return 0;
}

int GuiCam(int ex, int ey, int ez, int lx, int ly, int lz)
{
#ifdef __APPLE__
    mc_look[0] = (float)lx;
    mc_look[1] = (float)ly;
    mc_look[2] = (float)lz;
    mc_eye[0] = (float)ex;
    mc_eye[1] = (float)ey;
    mc_eye[2] = (float)ez;
    if (!mc_orbit_user)
        mc_orbit_from_eye();
    else {
        float dx = (float)ex - (float)lx;
        float dy = (float)ey - (float)ly;
        float dz = (float)ez - (float)lz;
        float r = sqrtf(dx * dx + dy * dy + dz * dz);
        if (r >= 8.0f)
            mc_cam_radius = r;
    }
    mc_eye_from_orbit();
    mc_rebuild_cam();
    return 0;
#else
    (void)ex; (void)ey; (void)ez; (void)lx; (void)ly; (void)lz;
    return -1;
#endif
}

int GuiLight(int dx, int dy, int dz)
{
#ifdef __APPLE__
    float len = sqrtf((float)(dx * dx + dy * dy + dz * dz));
    if (len < 1)
        len = 1;
    mc_light[0] = dx / len;
    mc_light[1] = dy / len;
    mc_light[2] = dz / len;
    return 0;
#else
    (void)dx; (void)dy; (void)dz;
    return -1;
#endif
}

int GuiId(int id)
{
#ifdef __APPLE__
    mc_cur_id = id;
    return 0;
#else
    (void)id;
    return -1;
#endif
}

int GuiBox(int x, int y, int z, int sx, int sy, int sz, int color)
{
#ifdef __APPLE__
    float cx = (float)x, cy = (float)y, cz = (float)z;
    float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;
    if (hx < 0.5f) hx = 0.5f;
    if (hy < 0.5f) hy = 0.5f;
    if (hz < 0.5f) hz = 0.5f;
    float x0 = cx - hx, x1 = cx + hx;
    float y0 = cy - hy, y1 = cy + hy;
    float z0 = cz - hz, z1 = cz + hz;
    /* CCW from outside */
    mc_face(x0, y0, z1,  x1, y0, z1,  x1, y1, z1,  x0, y1, z1,  0, 0, 1, color);
    mc_face(x1, y0, z0,  x0, y0, z0,  x0, y1, z0,  x1, y1, z0,  0, 0, -1, color);
    mc_face(x0, y0, z0,  x0, y0, z1,  x0, y1, z1,  x0, y1, z0, -1, 0, 0, color);
    mc_face(x1, y0, z1,  x1, y0, z0,  x1, y1, z0,  x1, y1, z1,  1, 0, 0, color);
    mc_face(x0, y1, z1,  x1, y1, z1,  x1, y1, z0,  x0, y1, z0,  0, 1, 0, color);
    mc_face(x0, y0, z0,  x1, y0, z0,  x1, y0, z1,  x0, y0, z1,  0, -1, 0, color);
    if (mc_cur_id >= 0) {
        if (mc_nhit >= mc_caphit) {
            int n = mc_caphit ? mc_caphit * 2 : 128;
            mc_hits = (mc_aabb *)realloc(mc_hits, (size_t)n * sizeof(mc_aabb));
            mc_caphit = n;
        }
        mc_aabb *a = &mc_hits[mc_nhit++];
        a->minx = x0; a->miny = y0; a->minz = z0;
        a->maxx = x1; a->maxy = y1; a->maxz = z1;
        a->id = mc_cur_id;
    }
    return 0;
#else
    (void)x; (void)y; (void)z; (void)sx; (void)sy; (void)sz; (void)color;
    return -1;
#endif
}

int GuiHit(int mx, int my)
{
#ifdef __APPLE__
    if (!mc_have_cam)
        mc_rebuild_cam();
    if (mc_fb_w < 1 || mc_fb_h < 1)
        return -1;
    float inv[16];
    if (!mc_m4_inv(inv, mc_mvp))
        return -1;
    float ndc_x = (2.0f * (float)mx / (float)mc_fb_w) - 1.0f;
    float ndc_y = 1.0f - (2.0f * (float)my / (float)mc_fb_h);
    float ax, ay, az, aw, bx, by, bz, bw;
    mc_xf(inv, ndc_x, ndc_y, 0.0f, 1.0f, &ax, &ay, &az, &aw);
    mc_xf(inv, ndc_x, ndc_y, 1.0f, 1.0f, &bx, &by, &bz, &bw);
    if (aw != 0) { ax /= aw; ay /= aw; az /= aw; }
    if (bw != 0) { bx /= bw; by /= bw; bz /= bw; }
    float dx = bx - ax, dy = by - ay, dz = bz - az;
    float best = 1e9f;
    int best_id = -1;
    for (int i = 0; i < mc_nhit; i++) {
        float t;
        if (!mc_ray_aabb(ax, ay, az, dx, dy, dz, &mc_hits[i], &t))
            continue;
        if (t < best) {
            best = t;
            best_id = mc_hits[i].id;
        }
    }
    return best_id;
#else
    (void)mx; (void)my;
    return -1;
#endif
}

/* Save the current framebuffer as a 24-bit BMP (screenshot builtin). */
int GuiSave(const char *path)
{
    if (!mc_fb || !path || !*path)
        return -1;
    FILE *fp = fopen(path, "wb");
    if (!fp)
        return -1;
    int w = mc_fb_w, h = mc_fb_h;
    int row = (w * 3 + 3) & ~3;
    unsigned int img = (unsigned int)row * h;
    unsigned int off = 54;
    unsigned int size = off + img;
    unsigned char hdr[54] = {0};
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = size & 255; hdr[3] = (size >> 8) & 255;
    hdr[4] = (size >> 16) & 255; hdr[5] = (size >> 24) & 255;
    hdr[10] = off & 255;
    hdr[14] = 40;
    hdr[18] = w & 255; hdr[19] = (w >> 8) & 255;
    hdr[20] = (w >> 16) & 255; hdr[21] = (w >> 24) & 255;
    hdr[22] = h & 255; hdr[23] = (h >> 8) & 255;
    hdr[24] = (h >> 16) & 255; hdr[25] = (h >> 24) & 255;
    hdr[26] = 1;
    hdr[28] = 24;
    hdr[34] = img & 255; hdr[35] = (img >> 8) & 255;
    hdr[36] = (img >> 16) & 255; hdr[37] = (img >> 24) & 255;
    fwrite(hdr, 1, 54, fp);
    unsigned char *line = (unsigned char *)calloc(1, (size_t)row);
    if (!line) {
        fclose(fp);
        return -1;
    }
    for (int y = h - 1; y >= 0; y--) { /* BMP rows bottom-up */
        for (int x = 0; x < w; x++) {
            uint32_t p = mc_fb[(long)y * w + x];
            line[x * 3 + 0] = p & 255;         /* B */
            line[x * 3 + 1] = (p >> 8) & 255;  /* G */
            line[x * 3 + 2] = (p >> 16) & 255; /* R */
        }
        fwrite(line, 1, (size_t)row, fp);
    }
    free(line);
    fclose(fp);
    return 0;
}
