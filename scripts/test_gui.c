/* Pixel-level tests for the from-scratch GUI runtime.
 * Includes gui_rt.c directly so the static framebuffer is inspectable. */
#include "../src/runtime/gui_rt.c"
#include <stdio.h>

static int fails;

static void check(const char *name, int ok)
{
    if (ok) {
        printf("PASS %s\n", name);
    } else {
        fails++;
        fprintf(stderr, "FAIL %s\n", name);
    }
}

static uint32_t px(int x, int y)
{
    return mc_fb[(long)y * mc_fb_w + x];
}

int main(void)
{
    int r = GuiOpen(320, 200, "gui_rt test");
    check("GuiOpen", r == 0);
    if (r != 0)
        return 1;

    check("fb allocated", mc_fb != 0 && mc_fb_w == 320 && mc_fb_h == 200);

    GuiClear(0x112233);
    check("clear color", px(0, 0) == 0x112233 && px(319, 199) == 0x112233);

    GuiRect(10, 10, 50, 40, 0xFF0000);
    check("rect inside", px(10, 10) == 0xFF0000 && px(59, 49) == 0xFF0000);
    check("rect outside", px(9, 10) == 0x112233 && px(60, 50) == 0x112233);

    GuiRect(-20, -20, 30, 30, 0x00FF00); /* clipped at origin */
    check("rect clipped", px(0, 0) == 0x00FF00 && px(10, 10) == 0xFF0000);

    GuiClear(0);
    GuiText(0, 0, "A", 0xFFFFFF, 1);
    int lit = 0;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            if (px(x, y) == 0xFFFFFF)
                lit++;
    check("glyph A pixel count", lit == 28);

    GuiClear(0);
    GuiText(0, 0, "A", 0xFFFFFF, 3);
    int lit3 = 0;
    for (int y = 0; y < 24; y++)
        for (int x = 0; x < 24; x++)
            if (px(x, y) == 0xFFFFFF)
                lit3++;
    check("glyph scale 3", lit3 == 28 * 9);

    check("present", GuiPresent() == 0);

    check("save bmp", GuiSave("/tmp/gui_rt_test.bmp") == 0);


    for (int i = 0; i < 10; i++)
        GuiPoll();
    check("poll no close", GuiPoll() != 1);

    check("close", GuiClose() == 0);
    check("draw after close", GuiRect(0, 0, 4, 4, 1) == -1);

    printf("%s\n", fails ? "GUI TESTS FAILED" : "All GUI runtime tests OK");
    return fails ? 1 : 0;
}
