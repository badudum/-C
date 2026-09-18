#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/AppKit.h>
#import <simd/simd.h>
#include "include/gui_mtl.h"
#include <string.h>
#include <stdlib.h>

static const char *k_shader =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VIn {\n"
    "  float3 pos [[attribute(0)]];\n"
    "  float3 nrm [[attribute(1)]];\n"
    "  float4 col [[attribute(2)]];\n"
    "  float2 uv [[attribute(3)]];\n"
    "};\n"
    "struct VOut {\n"
    "  float4 pos [[position]];\n"
    "  float3 nrm;\n"
    "  float4 col;\n"
    "  float2 uv;\n"
    "};\n"
    "struct Uniforms {\n"
    "  float4x4 mvp;\n"
    "  float4 light;\n"
    "  float4 screen;\n"
    "};\n"
    "vertex VOut v3d(VIn in [[stage_in]], constant Uniforms &u [[buffer(1)]]) {\n"
    "  VOut o;\n"
    "  o.pos = u.mvp * float4(in.pos, 1.0);\n"
    "  o.nrm = in.nrm;\n"
    "  o.col = in.col;\n"
    "  o.uv = in.uv;\n"
    "  return o;\n"
    "}\n"
    "fragment float4 f3d(VOut in [[stage_in]], constant Uniforms &u [[buffer(1)]],\n"
    "                     texture2d<float> atlas [[texture(0)]],\n"
    "                     sampler samp [[sampler(0)]]) {\n"
    "  float3 L = normalize(u.light.xyz);\n"
    "  float ndl = max(dot(normalize(in.nrm), L), 0.0);\n"
    "  float amb = 0.28;\n"
    "  float spec = pow(max(ndl, 0.0), 8.0) * 0.18;\n"
    "  float lit = amb + ndl * 0.72 + spec;\n"
    "  float4 tex = atlas.sample(samp, in.uv);\n"
    "  return float4(in.col.rgb * tex.rgb * lit, in.col.a);\n"
    "}\n"
    "vertex VOut v2d(VIn in [[stage_in]], constant Uniforms &u [[buffer(1)]]) {\n"
    "  VOut o;\n"
    "  float2 p = in.pos.xy;\n"
    "  float x = (p.x / u.screen.x) * 2.0 - 1.0;\n"
    "  float y = 1.0 - (p.y / u.screen.y) * 2.0;\n"
    "  o.pos = float4(x, y, 0.0, 1.0);\n"
    "  o.nrm = in.nrm;\n"
    "  o.col = in.col;\n"
    "  o.uv = in.uv;\n"
    "  return o;\n"
    "}\n"
    "fragment float4 f2d(VOut in [[stage_in]]) {\n"
    "  return in.col;\n"
    "}\n";

typedef struct {
    simd_float4x4 mvp;
    simd_float4 light;
    simd_float4 screen;
} mc_uniforms;

static id<MTLDevice> g_dev;
static id<MTLCommandQueue> g_queue;
static id<MTLRenderPipelineState> g_p3;
static id<MTLRenderPipelineState> g_p2;
static id<MTLDepthStencilState> g_depth;
static id<MTLDepthStencilState> g_nodepth;
static id<MTLTexture> g_dtex;
static id<MTLBuffer> g_ubuf;
static id<MTLBuffer> g_readbuf;
static id<MTLTexture> g_atlas;
static id<MTLSamplerState> g_atlas_samp;
static CAMetalLayer *g_layer;
static int g_w, g_h;
static int g_ready;
static int g_clear_rgb = 0x1a1a22;
static mc_uniforms g_u;
static id<MTLTexture> g_last_color;

#define MC_TILE 16
#define MC_TILES 10
#define MC_ATLAS_W (MC_TILE * MC_TILES)
#define MC_ATLAS_H MC_TILE

static unsigned mc_atlas_hash(int x, int y, int seed)
{
    unsigned n = (unsigned)x * 374761393u + (unsigned)y * 668265263u +
                 (unsigned)seed * 2246822519u;
    n = (n ^ (n >> 13)) * 1274126177u;
    return n ^ (n >> 16);
}

static void mc_atlas_put(unsigned char *buf, int tile, int x, int y,
                          unsigned char r, unsigned char g, unsigned char b)
{
    int px = tile * MC_TILE + x;
    int idx = (y * MC_ATLAS_W + px) * 4;
    buf[idx] = r; buf[idx + 1] = g; buf[idx + 2] = b; buf[idx + 3] = 255;
}

static void mc_speckle(unsigned char *buf, int tile, int x, int y, int seed,
                        int br, int bg, int bb, int dev)
{
    unsigned h = mc_atlas_hash(x, y, seed);
    int v = (int)(h % (unsigned)(dev * 2 + 1)) - dev;
    int r = br + v, g = bg + v, b = bb + v;
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    mc_atlas_put(buf, tile, x, y, (unsigned char)r, (unsigned char)g, (unsigned char)b);
}

static unsigned char *mc_build_atlas(void)
{
    unsigned char *buf = (unsigned char *)calloc(1, (size_t)MC_ATLAS_W * MC_ATLAS_H * 4);
    if (!buf)
        return NULL;
    for (int y = 0; y < MC_TILE; y++) {
        for (int x = 0; x < MC_TILE; x++) {
            /* tile 0: white, used by untextured (flat-color) boxes */
            mc_atlas_put(buf, 0, x, y, 255, 255, 255);
            /* tile 1: grass top */
            mc_speckle(buf, 1, x, y, 101, 92, 156, 60, 18);
            /* tile 2: grass side (green cap over dirt) */
            if (y < 4)
                mc_speckle(buf, 2, x, y, 102, 92, 156, 60, 18);
            else
                mc_speckle(buf, 2, x, y, 103, 121, 85, 58, 16);
            /* tile 3: dirt */
            mc_speckle(buf, 3, x, y, 104, 121, 85, 58, 16);
            /* tile 4: stone */
            mc_speckle(buf, 4, x, y, 105, 130, 130, 133, 14);
            /* tile 5: wood side (bark stripes) */
            {
                int stripe = (x / 3) & 1;
                int br = stripe ? 101 : 87, bg = stripe ? 73 : 62, bb = stripe ? 47 : 38;
                mc_speckle(buf, 5, x, y, 106, br, bg, bb, 8);
            }
            /* tile 6: wood top (rings) */
            {
                int dx = x - 8, dy = y - 8;
                int ring = (dx < 0 ? -dx : dx);
                int ring2 = (dy < 0 ? -dy : dy);
                if (ring2 > ring) ring = ring2;
                int band = ring % 3;
                int br = band == 0 ? 185 : (band == 1 ? 168 : 150);
                mc_speckle(buf, 6, x, y, 107, br, br - 40, br - 90, 6);
            }
            /* tile 7: leaves */
            mc_speckle(buf, 7, x, y, 108, 53, 110, 32, 22);
            /* tile 8: sand */
            mc_speckle(buf, 8, x, y, 109, 219, 205, 145, 14);
            /* tile 9: bedrock */
            mc_speckle(buf, 9, x, y, 110, 62, 62, 66, 20);
        }
    }
    return buf;
}

static void mc_make_atlas(void)
{
    unsigned char *pixels = mc_build_atlas();
    if (!pixels)
        return;
    MTLTextureDescriptor *td = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                     width:MC_ATLAS_W
                                    height:MC_ATLAS_H
                                 mipmapped:NO];
    td.usage = MTLTextureUsageShaderRead;
    g_atlas = [g_dev newTextureWithDescriptor:td];
    MTLRegion region = MTLRegionMake2D(0, 0, MC_ATLAS_W, MC_ATLAS_H);
    [g_atlas replaceRegion:region mipmapLevel:0 withBytes:pixels
                bytesPerRow:(NSUInteger)MC_ATLAS_W * 4];
    free(pixels);

    MTLSamplerDescriptor *sd = [[MTLSamplerDescriptor alloc] init];
    sd.minFilter = MTLSamplerMinMagFilterNearest;
    sd.magFilter = MTLSamplerMinMagFilterNearest;
    sd.sAddressMode = MTLSamplerAddressModeClampToEdge;
    sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
    g_atlas_samp = [g_dev newSamplerStateWithDescriptor:sd];
}

static MTLVertexDescriptor *mc_vdesc(void)
{
    MTLVertexDescriptor *vd = [[MTLVertexDescriptor alloc] init];
    vd.attributes[0].format = MTLVertexFormatFloat3;
    vd.attributes[0].offset = 0;
    vd.attributes[0].bufferIndex = 0;
    vd.attributes[1].format = MTLVertexFormatFloat3;
    vd.attributes[1].offset = 12;
    vd.attributes[1].bufferIndex = 0;
    vd.attributes[2].format = MTLVertexFormatFloat4;
    vd.attributes[2].offset = 24;
    vd.attributes[2].bufferIndex = 0;
    vd.attributes[3].format = MTLVertexFormatFloat2;
    vd.attributes[3].offset = 40;
    vd.attributes[3].bufferIndex = 0;
    vd.layouts[0].stride = sizeof(mc_mtl_vtx);
    vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    return vd;
}

static id<MTLRenderPipelineState> mc_pipe(id<MTLLibrary> lib, NSString *vs,
                                          NSString *fs, BOOL depth, NSError **err)
{
    MTLRenderPipelineDescriptor *d = [[MTLRenderPipelineDescriptor alloc] init];
    d.vertexFunction = [lib newFunctionWithName:vs];
    d.fragmentFunction = [lib newFunctionWithName:fs];
    d.vertexDescriptor = mc_vdesc();
    d.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    d.colorAttachments[0].blendingEnabled = YES;
    d.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    d.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    d.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    d.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    if (depth)
        d.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    return [g_dev newRenderPipelineStateWithDescriptor:d error:err];
}

static void mc_make_depth(void)
{
    if (g_w < 1 || g_h < 1)
        return;
    MTLTextureDescriptor *td = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                     width:(NSUInteger)g_w
                                    height:(NSUInteger)g_h
                                 mipmapped:NO];
    td.usage = MTLTextureUsageRenderTarget;
    td.storageMode = MTLStorageModePrivate;
    g_dtex = [g_dev newTextureWithDescriptor:td];
}

int mc_mtl_init(void *nsview, int w, int h)
{
    @autoreleasepool {
        if (!nsview || w < 32 || h < 32)
            return -1;
        g_dev = MTLCreateSystemDefaultDevice();
        if (!g_dev)
            return -1;
        g_queue = [g_dev newCommandQueue];
        NSView *view = (__bridge NSView *)nsview;
        view.wantsLayer = YES;
        CAMetalLayer *layer = [CAMetalLayer layer];
        layer.device = g_dev;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        layer.framebufferOnly = NO;
        layer.contentsScale = 1.0;
        layer.drawableSize = CGSizeMake(w, h);
        view.layer = layer;
        g_layer = layer;
        g_w = w;
        g_h = h;

        NSError *err = nil;
        id<MTLLibrary> lib = [g_dev newLibraryWithSource:@(k_shader)
                                                 options:nil
                                                   error:&err];
        if (!lib) {
            fprintf(stderr, "Metal shader: %s\n",
                    err.localizedDescription.UTF8String);
            return -1;
        }
        g_p3 = mc_pipe(lib, @"v3d", @"f3d", YES, &err);
        if (!g_p3) {
            fprintf(stderr, "Metal 3d pipe: %s\n",
                    err.localizedDescription.UTF8String);
            return -1;
        }
        g_p2 = mc_pipe(lib, @"v2d", @"f2d", YES, &err);
        if (!g_p2) {
            fprintf(stderr, "Metal 2d pipe: %s\n",
                    err.localizedDescription.UTF8String);
            return -1;
        }
        MTLDepthStencilDescriptor *dd = [[MTLDepthStencilDescriptor alloc] init];
        dd.depthCompareFunction = MTLCompareFunctionLess;
        dd.depthWriteEnabled = YES;
        g_depth = [g_dev newDepthStencilStateWithDescriptor:dd];
        dd.depthCompareFunction = MTLCompareFunctionAlways;
        dd.depthWriteEnabled = NO;
        g_nodepth = [g_dev newDepthStencilStateWithDescriptor:dd];

        g_ubuf = [g_dev newBufferWithLength:sizeof(mc_uniforms)
                                    options:MTLResourceStorageModeShared];
        g_readbuf = [g_dev newBufferWithLength:(NSUInteger)w * h * 4
                                       options:MTLResourceStorageModeShared];
        mc_make_depth();
        mc_make_atlas();
        memset(&g_u, 0, sizeof(g_u));
        g_u.light = simd_make_float4(0.35f, 0.85f, 0.4f, 0);
        g_u.screen = simd_make_float4((float)w, (float)h, 0, 0);
        g_u.mvp = matrix_identity_float4x4;
        g_ready = 1;
        return 0;
    }
}

void mc_mtl_shutdown(void)
{
    g_ready = 0;
    g_dev = nil;
    g_queue = nil;
    g_p3 = nil;
    g_p2 = nil;
    g_depth = nil;
    g_nodepth = nil;
    g_dtex = nil;
    g_ubuf = nil;
    g_readbuf = nil;
    g_layer = nil;
    g_last_color = nil;
    g_atlas = nil;
    g_atlas_samp = nil;
}

void mc_mtl_set_mvp(const float *m)
{
    if (!m)
        return;
    memcpy(&g_u.mvp, m, 16 * sizeof(float));
}

int mc_mtl_begin(int clear_rgb)
{
    g_clear_rgb = clear_rgb;
    return g_ready ? 0 : -1;
}

void mc_mtl_set_light(float x, float y, float z)
{
    g_u.light = simd_make_float4(x, y, z, 0.0f);
}

static id<MTLBuffer> mc_upload(const mc_mtl_vtx *v, int n)
{
    if (!v || n <= 0)
        return nil;
    return [g_dev newBufferWithBytes:v
                              length:(NSUInteger)n * sizeof(mc_mtl_vtx)
                             options:MTLResourceStorageModeShared];
}

static const mc_mtl_vtx *g_v3;
static int g_n3;
static const mc_mtl_vtx *g_v2;
static int g_n2;

void mc_mtl_draw3d(const mc_mtl_vtx *v, int n)
{
    g_v3 = v;
    g_n3 = n;
}

void mc_mtl_draw2d(const mc_mtl_vtx *v, int n)
{
    g_v2 = v;
    g_n2 = n;
}

static void mc_set_clear(MTLRenderPassDescriptor *pass, int rgb)
{
    float r = ((rgb >> 16) & 255) / 255.0f;
    float g = ((rgb >> 8) & 255) / 255.0f;
    float b = (rgb & 255) / 255.0f;
    pass.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, 1);
}

int mc_mtl_present(void)
{
    @autoreleasepool {
        if (!g_ready || !g_layer)
            return -1;
        id<CAMetalDrawable> draw = [g_layer nextDrawable];
        if (!draw)
            return -1;
        memcpy(g_ubuf.contents, &g_u, sizeof(g_u));

        MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].texture = draw.texture;
        pass.colorAttachments[0].loadAction = MTLLoadActionClear;
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        mc_set_clear(pass, g_clear_rgb);
        pass.depthAttachment.texture = g_dtex;
        pass.depthAttachment.loadAction = MTLLoadActionClear;
        pass.depthAttachment.storeAction = MTLStoreActionDontCare;
        pass.depthAttachment.clearDepth = 1.0;

        id<MTLCommandBuffer> cmd = [g_queue commandBuffer];
        id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];
        [enc setCullMode:MTLCullModeNone];
        [enc setFrontFacingWinding:MTLWindingCounterClockwise];
        if (g_n3 > 0 && g_v3) {
            id<MTLBuffer> vb = mc_upload(g_v3, g_n3);
            [enc setRenderPipelineState:g_p3];
            [enc setDepthStencilState:g_depth];
            [enc setVertexBuffer:vb offset:0 atIndex:0];
            [enc setVertexBuffer:g_ubuf offset:0 atIndex:1];
            [enc setFragmentBuffer:g_ubuf offset:0 atIndex:1];
            [enc setFragmentTexture:g_atlas atIndex:0];
            [enc setFragmentSamplerState:g_atlas_samp atIndex:0];
            [enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0
                    vertexCount:(NSUInteger)g_n3];
        }
        if (g_n2 > 0 && g_v2) {
            id<MTLBuffer> vb = mc_upload(g_v2, g_n2);
            [enc setRenderPipelineState:g_p2];
            [enc setDepthStencilState:g_nodepth];
            [enc setVertexBuffer:vb offset:0 atIndex:0];
            [enc setVertexBuffer:g_ubuf offset:0 atIndex:1];
            [enc setFragmentBuffer:g_ubuf offset:0 atIndex:1];
            [enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0
                    vertexCount:(NSUInteger)g_n2];
        }
        [enc endEncoding];

        if (g_readbuf) {
            id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
            MTLSize size = { (NSUInteger)g_w, (NSUInteger)g_h, 1 };
            [blit copyFromTexture:draw.texture
                      sourceSlice:0
                      sourceLevel:0
                     sourceOrigin:MTLOriginMake(0, 0, 0)
                       sourceSize:size
                         toBuffer:g_readbuf
                destinationOffset:0
           destinationBytesPerRow:(NSUInteger)g_w * 4
         destinationBytesPerImage:(NSUInteger)g_w * g_h * 4];
            [blit endEncoding];
        }
        [cmd presentDrawable:draw];
        [cmd commit];
        [cmd waitUntilCompleted];
        g_v3 = 0;
        g_n3 = 0;
        g_v2 = 0;
        g_n2 = 0;
        return 0;
    }
}

void mc_mtl_set_clear_rgb(int rgb)
{
    g_clear_rgb = rgb;
}

int mc_mtl_readback(unsigned int *out_rgb, int w, int h)
{
    if (!g_ready || !g_readbuf || !out_rgb)
        return -1;
    if (w != g_w || h != g_h)
        return -1;
    const unsigned char *src = (const unsigned char *)g_readbuf.contents;
    for (int i = 0; i < w * h; i++) {
        unsigned char b = src[i * 4 + 0];
        unsigned char g = src[i * 4 + 1];
        unsigned char r = src[i * 4 + 2];
        out_rgb[i] = (unsigned int)b | ((unsigned int)g << 8) | ((unsigned int)r << 16);
    }
    return 0;
}
