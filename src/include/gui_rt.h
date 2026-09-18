#ifndef MC_GUI_RT_H
#define MC_GUI_RT_H

/* From-scratch GUI runtime (no third-party dependencies). */

int GuiOpen(int w, int h, const char *title);
int GuiClose(void);
int GuiClear(int color);
int GuiRect(int x, int y, int w, int h, int color);
int GuiText(int x, int y, const char *s, int color, int scale);
int GuiPresent(void);
int GuiPoll(void);
int GuiEventX(void);
int GuiEventY(void);
int GuiEventKey(void);
int GuiHeld(int code);
int GuiFwdX(void);
int GuiFwdZ(void);
int GuiSleep(int ms);
int GuiSave(const char *path);
int GuiMask(int x, int y, const char *mask, int cols, int color, int scale);
int GuiCam(int ex, int ey, int ez, int lx, int ly, int lz);
int GuiLight(int dx, int dy, int dz);
int GuiId(int id);
int GuiBox(int x, int y, int z, int sx, int sy, int sz, int color);
int GuiBoxTex(int x, int y, int z, int size, int color, int kind);
int GuiHit(int mx, int my);

#endif
