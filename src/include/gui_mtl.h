#ifndef MC_GUI_MTL_H
#define MC_GUI_MTL_H

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float r, g, b, a;
} mc_mtl_vtx;

int mc_mtl_init(void *nsview, int w, int h);
void mc_mtl_shutdown(void);
int mc_mtl_begin(int clear_rgb);
void mc_mtl_set_mvp(const float *mvp16);
void mc_mtl_set_light(float x, float y, float z);
void mc_mtl_draw3d(const mc_mtl_vtx *v, int n);
void mc_mtl_draw2d(const mc_mtl_vtx *v, int n);
int mc_mtl_present(void);
int mc_mtl_readback(unsigned int *out_rgb, int w, int h);

#endif
