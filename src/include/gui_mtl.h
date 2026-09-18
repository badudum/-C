#ifndef MC_GUI_MTL_H
#define MC_GUI_MTL_H

/* Generic 16x16 tile atlas. Tile 0 is white (flat-colored GuiBox).
 * Programs fill other tiles with GuiTex; the runtime does not ship
 * game-specific artwork. */
#define GUI_TILE_PX 16
#define GUI_TILE_COUNT 16
#define GUI_ATLAS_W (GUI_TILE_PX * GUI_TILE_COUNT)
#define GUI_ATLAS_H GUI_TILE_PX

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float r, g, b, a;
    float u, v;
} mc_mtl_vtx;

int mc_mtl_init(void *native, int w, int h);
void mc_mtl_shutdown(void);
#if defined(__linux__)
/* native Display* / Window for VK_KHR_xlib_surface; call before mc_mtl_init. */
void mc_mtl_set_x11(void *display, unsigned long window);
#endif
int mc_mtl_begin(int clear_rgb);
void mc_mtl_set_mvp(const float *mvp16);
void mc_mtl_set_light(float x, float y, float z);
void mc_mtl_draw3d(const mc_mtl_vtx *v, int n);
void mc_mtl_draw2d(const mc_mtl_vtx *v, int n);
int mc_mtl_present(void);
int mc_mtl_readback(unsigned int *out_rgb, int w, int h);
int mc_mtl_tex(int tile, int x, int y, int rgb);
void mc_mtl_tex_flush(void);

#endif
