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

    ((void (*)(id, SEL, int))objc_msgSend)(mc_app,
        mc_sel("activateIgnoringOtherApps:"), 1);

    mc_loop_mode = mc_nsstring("kCFRunLoopDefaultMode");
    mc_open = 1;
    mc_ev_x = 0;
    mc_ev_y = 0;
    mc_ev_key = 0;
    return 0;
}

int GuiClose(void)
{
    if (!mc_open)
        return 0;
    ((mc_msg_vid)objc_msgSend)(mc_win, mc_sel("close"), 0);
    free(mc_fb);
    mc_fb = 0;
    mc_open = 0;
    return 0;
}

int GuiPresent(void)
{
    if (!mc_open || !mc_fb)
        return -1;
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

    if (t == 1 || t == 2 || t == 5 || t == 6) { /* mouse down/up/move/drag */
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
    return 0;
}

int GuiRect(int x, int y, int w, int h, int color)
{
    if (!mc_fb)
        return -1;
    mc_fb_rect(x, y, w, h, color);
    return 0;
}

int GuiText(int x, int y, const char *s, int color, int scale)
{
    if (!mc_fb)
        return -1;
    mc_fb_text(x, y, s, color, scale);
    return 0;
}

int GuiEventX(void) { return mc_ev_x; }
int GuiEventY(void) { return mc_ev_y; }
int GuiEventKey(void) { return mc_ev_key; }

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
