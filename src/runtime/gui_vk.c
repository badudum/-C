/*
 * gui_vk.c — Vulkan GPU backend for minusC Gui* builtins.
 *
 * Same mc_mtl_* contract as gui_metal.m: generic 16x16 tile atlas, tiled
 * boxes, 2D overlay. No program-specific artwork or game logic lives here.
 */
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include "include/gui_mtl.h"
#include "runtime/gui_vk_spv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#define MC_MAX_SWAP 16

typedef struct {
    float mvp[16];
    float light[4];
    float screen[4];
} mc_ubo;

static Display *g_dpy;
static Window g_win;
static int g_w, g_h;
static int g_ready;
static int g_clear_rgb = 0x1a1a22;
static mc_ubo g_u;
static const mc_mtl_vtx *g_v3;
static int g_n3;
static const mc_mtl_vtx *g_v2;
static int g_n2;

static unsigned char *g_atlas_cpu;
static int g_atlas_dirty;

static VkInstance g_inst;
static VkPhysicalDevice g_phys;
static VkDevice g_dev;
static VkQueue g_q;
static uint32_t g_qfam;
static VkSurfaceKHR g_surf;
static VkSwapchainKHR g_swap;
static VkFormat g_col_fmt;
static VkColorSpaceKHR g_cs;
static int g_swap_bgra;
static uint32_t g_nimg;
static VkImage g_imgs[MC_MAX_SWAP];
static VkImageView g_views[MC_MAX_SWAP];
static VkFramebuffer g_fb[MC_MAX_SWAP];
static VkImage g_depth;
static VkDeviceMemory g_depth_mem;
static VkImageView g_depth_view;
static VkRenderPass g_rp;
static VkDescriptorSetLayout g_dsl;
static VkPipelineLayout g_pl;
static VkPipeline g_p3, g_p2;
static VkDescriptorPool g_dpool;
static VkDescriptorSet g_dset;
static VkCommandPool g_cpool;
static VkCommandBuffer g_cmd;
static VkSemaphore g_sem_acq;
static VkSemaphore g_sem_pres;
static VkFence g_fence;

static VkBuffer g_ubo;
static VkDeviceMemory g_ubo_mem;
static void *g_ubo_map;
static VkBuffer g_vb3, g_vb2;
static VkDeviceMemory g_vb3_mem, g_vb2_mem;
static void *g_vb3_map, *g_vb2_map;
static int g_vb3_cap, g_vb2_cap;
static VkBuffer g_read;
static VkDeviceMemory g_read_mem;
static void *g_read_map;
static int g_can_copy;

static VkImage g_atlas;
static VkDeviceMemory g_atlas_mem;
static VkImageView g_atlas_view;
static VkSampler g_samp;
static VkBuffer g_atlas_stg;
static VkDeviceMemory g_atlas_stg_mem;
static void *g_atlas_stg_map;
static VkImageLayout g_atlas_layout;

static int find_mem(uint32_t bits, VkMemoryPropertyFlags flags)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(g_phys, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++) {
        if ((bits & (1u << i)) &&
            (mp.memoryTypes[i].propertyFlags & flags) == flags)
            return (int)i;
    }
    return -1;
}

static int make_buf(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags props, VkBuffer *b, VkDeviceMemory *m,
                    void **map)
{
    VkBufferCreateInfo bi = {0};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size = size;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(g_dev, &bi, 0, b) != VK_SUCCESS)
        return -1;
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(g_dev, *b, &req);
    int mt = find_mem(req.memoryTypeBits, props);
    if (mt < 0)
        return -1;
    VkMemoryAllocateInfo ai = {0};
    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = (uint32_t)mt;
    if (vkAllocateMemory(g_dev, &ai, 0, m) != VK_SUCCESS)
        return -1;
    vkBindBufferMemory(g_dev, *b, *m, 0);
    if (map) {
        if (vkMapMemory(g_dev, *m, 0, size, 0, map) != VK_SUCCESS)
            return -1;
    }
    return 0;
}

static int make_img(uint32_t w, uint32_t h, VkFormat fmt, VkImageUsageFlags usage,
                    VkImage *img, VkDeviceMemory *mem)
{
    VkImageCreateInfo ii = {0};
    ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = fmt;
    ii.extent.width = w;
    ii.extent.height = h;
    ii.extent.depth = 1;
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = usage;
    ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (vkCreateImage(g_dev, &ii, 0, img) != VK_SUCCESS)
        return -1;
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(g_dev, *img, &req);
    int mt = find_mem(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (mt < 0)
        return -1;
    VkMemoryAllocateInfo ai = {0};
    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = (uint32_t)mt;
    if (vkAllocateMemory(g_dev, &ai, 0, mem) != VK_SUCCESS)
        return -1;
    vkBindImageMemory(g_dev, *img, *mem, 0);
    return 0;
}

static VkImageView make_view(VkImage img, VkFormat fmt, VkImageAspectFlags aspect)
{
    VkImageViewCreateInfo vi = {0};
    vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vi.image = img;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = fmt;
    vi.subresourceRange.aspectMask = aspect;
    vi.subresourceRange.levelCount = 1;
    vi.subresourceRange.layerCount = 1;
    VkImageView v = VK_NULL_HANDLE;
    if (vkCreateImageView(g_dev, &vi, 0, &v) != VK_SUCCESS)
        return VK_NULL_HANDLE;
    return v;
}

static VkShaderModule make_shader(const uint32_t *words, uint32_t nwords)
{
    VkShaderModuleCreateInfo si = {0};
    si.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    si.codeSize = (size_t)nwords * 4u;
    si.pCode = words;
    VkShaderModule m = VK_NULL_HANDLE;
    if (vkCreateShaderModule(g_dev, &si, 0, &m) != VK_SUCCESS)
        return VK_NULL_HANDLE;
    return m;
}

static void atlas_put(unsigned char *buf, int tile, int x, int y,
                      unsigned char r, unsigned char g, unsigned char b)
{
    if (!buf || tile < 0 || tile >= GUI_TILE_COUNT)
        return;
    if (x < 0 || x >= GUI_TILE_PX || y < 0 || y >= GUI_TILE_PX)
        return;
    int px = tile * GUI_TILE_PX + x;
    int idx = (y * GUI_ATLAS_W + px) * 4;
    buf[idx] = r;
    buf[idx + 1] = g;
    buf[idx + 2] = b;
    buf[idx + 3] = 255;
}

static unsigned char *build_atlas(void)
{
    unsigned char *buf = (unsigned char *)calloc(1, (size_t)GUI_ATLAS_W * GUI_ATLAS_H * 4);
    if (!buf)
        return NULL;
    for (int y = 0; y < GUI_TILE_PX; y++)
        for (int x = 0; x < GUI_TILE_PX; x++)
            atlas_put(buf, 0, x, y, 255, 255, 255);
    return buf;
}

int mc_mtl_tex(int tile, int x, int y, int rgb)
{
    if (!g_atlas_cpu)
        return -1;
    if (tile < 0 || tile >= GUI_TILE_COUNT || x < 0 || x >= GUI_TILE_PX ||
        y < 0 || y >= GUI_TILE_PX)
        return -1;
    atlas_put(g_atlas_cpu, tile, x, y,
              (unsigned char)((rgb >> 16) & 255),
              (unsigned char)((rgb >> 8) & 255),
              (unsigned char)(rgb & 255));
    g_atlas_dirty = 1;
    return 0;
}

static void destroy_swapchain(void)
{
    if (!g_dev)
        return;
    vkDeviceWaitIdle(g_dev);
    for (uint32_t i = 0; i < g_nimg; i++) {
        if (g_fb[i])
            vkDestroyFramebuffer(g_dev, g_fb[i], 0);
        if (g_views[i])
            vkDestroyImageView(g_dev, g_views[i], 0);
        g_fb[i] = VK_NULL_HANDLE;
        g_views[i] = VK_NULL_HANDLE;
    }
    g_nimg = 0;
    if (g_depth_view)
        vkDestroyImageView(g_dev, g_depth_view, 0);
    if (g_depth)
        vkDestroyImage(g_dev, g_depth, 0);
    if (g_depth_mem)
        vkFreeMemory(g_dev, g_depth_mem, 0);
    g_depth_view = VK_NULL_HANDLE;
    g_depth = VK_NULL_HANDLE;
    g_depth_mem = VK_NULL_HANDLE;
    if (g_swap)
        vkDestroySwapchainKHR(g_dev, g_swap, 0);
    g_swap = VK_NULL_HANDLE;
}

static int pick_surface_format(void)
{
    uint32_t nfmt = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_phys, g_surf, &nfmt, 0);
    if (!nfmt)
        return -1;
    VkSurfaceFormatKHR *fmts = (VkSurfaceFormatKHR *)malloc(sizeof(*fmts) * nfmt);
    if (!fmts)
        return -1;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_phys, g_surf, &nfmt, fmts);
    VkSurfaceFormatKHR pick = fmts[0];
    for (uint32_t i = 0; i < nfmt; i++) {
        if (fmts[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
            pick = fmts[i];
            break;
        }
    }
    if (pick.format != VK_FORMAT_B8G8R8A8_UNORM) {
        for (uint32_t i = 0; i < nfmt; i++) {
            if (fmts[i].format == VK_FORMAT_R8G8B8A8_UNORM) {
                pick = fmts[i];
                break;
            }
        }
    }
    free(fmts);
    g_col_fmt = pick.format;
    g_cs = pick.colorSpace;
    g_swap_bgra = (g_col_fmt == VK_FORMAT_B8G8R8A8_UNORM ||
                   g_col_fmt == VK_FORMAT_B8G8R8A8_SRGB);
    return 0;
}

static int create_swapchain(void)
{
    VkSurfaceCapabilitiesKHR caps;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_phys, g_surf, &caps) != VK_SUCCESS)
        return -1;

    uint32_t npm = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g_phys, g_surf, &npm, 0);
    VkPresentModeKHR pm = VK_PRESENT_MODE_FIFO_KHR;
    if (npm) {
        VkPresentModeKHR *pms = (VkPresentModeKHR *)malloc(sizeof(*pms) * npm);
        if (pms) {
            vkGetPhysicalDeviceSurfacePresentModesKHR(g_phys, g_surf, &npm, pms);
            for (uint32_t i = 0; i < npm; i++) {
                if (pms[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                    pm = pms[i];
                    break;
                }
            }
            free(pms);
        }
    }

    VkExtent2D ext = caps.currentExtent;
    if (ext.width == 0xFFFFFFFFu) {
        ext.width = (uint32_t)g_w;
        ext.height = (uint32_t)g_h;
    }
    if (ext.width < caps.minImageExtent.width)
        ext.width = caps.minImageExtent.width;
    if (ext.height < caps.minImageExtent.height)
        ext.height = caps.minImageExtent.height;
    if (ext.width > caps.maxImageExtent.width)
        ext.width = caps.maxImageExtent.width;
    if (ext.height > caps.maxImageExtent.height)
        ext.height = caps.maxImageExtent.height;
    g_w = (int)ext.width;
    g_h = (int)ext.height;

    uint32_t count = caps.minImageCount + 1;
    if (caps.maxImageCount && count > caps.maxImageCount)
        count = caps.maxImageCount;
    if (count > MC_MAX_SWAP)
        count = MC_MAX_SWAP;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    g_can_copy = 0;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        g_can_copy = 1;
    }

    VkSwapchainKHR old = g_swap;
    VkSwapchainCreateInfoKHR sci = {0};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = g_surf;
    sci.minImageCount = count;
    sci.imageFormat = g_col_fmt;
    sci.imageColorSpace = g_cs;
    sci.imageExtent = ext;
    sci.imageArrayLayers = 1;
    sci.imageUsage = usage;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = pm;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = old;
    VkSwapchainKHR neu = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(g_dev, &sci, 0, &neu) != VK_SUCCESS)
        return -1;
    if (old)
        vkDestroySwapchainKHR(g_dev, old, 0);
    g_swap = neu;

    g_nimg = 0;
    if (vkGetSwapchainImagesKHR(g_dev, g_swap, &g_nimg, 0) != VK_SUCCESS ||
        g_nimg == 0 || g_nimg > MC_MAX_SWAP)
        return -1;
    if (vkGetSwapchainImagesKHR(g_dev, g_swap, &g_nimg, g_imgs) != VK_SUCCESS)
        return -1;
    for (uint32_t i = 0; i < g_nimg; i++) {
        g_views[i] = make_view(g_imgs[i], g_col_fmt, VK_IMAGE_ASPECT_COLOR_BIT);
        if (!g_views[i])
            return -1;
    }

    if (make_img(ext.width, ext.height, VK_FORMAT_D32_SFLOAT,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &g_depth, &g_depth_mem) != 0)
        return -1;
    g_depth_view = make_view(g_depth, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
    if (!g_depth_view)
        return -1;

    for (uint32_t i = 0; i < g_nimg; i++) {
        VkImageView atts[2] = { g_views[i], g_depth_view };
        VkFramebufferCreateInfo fi = {0};
        fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fi.renderPass = g_rp;
        fi.attachmentCount = 2;
        fi.pAttachments = atts;
        fi.width = ext.width;
        fi.height = ext.height;
        fi.layers = 1;
        if (vkCreateFramebuffer(g_dev, &fi, 0, &g_fb[i]) != VK_SUCCESS)
            return -1;
    }
    return 0;
}

static void img_barrier(VkCommandBuffer cmd, VkImage img, VkImageLayout oldl,
                        VkImageLayout newl, VkImageAspectFlags aspect,
                        VkAccessFlags srca, VkAccessFlags dsta,
                        VkPipelineStageFlags srcs, VkPipelineStageFlags dsts)
{
    VkImageMemoryBarrier b = {0};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    b.srcAccessMask = srca;
    b.dstAccessMask = dsta;
    b.oldLayout = oldl;
    b.newLayout = newl;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = img;
    b.subresourceRange.aspectMask = aspect;
    b.subresourceRange.levelCount = 1;
    b.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(cmd, srcs, dsts, 0, 0, 0, 0, 0, 1, &b);
}

static int ensure_vbuf(int n3, int n2)
{
    VkMemoryPropertyFlags hp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (n3 > g_vb3_cap) {
        if (g_vb3)
            vkDestroyBuffer(g_dev, g_vb3, 0);
        if (g_vb3_mem)
            vkFreeMemory(g_dev, g_vb3_mem, 0);
        g_vb3 = VK_NULL_HANDLE;
        g_vb3_mem = VK_NULL_HANDLE;
        g_vb3_map = 0;
        int cap = g_vb3_cap ? g_vb3_cap : 4096;
        while (cap < n3)
            cap *= 2;
        VkDeviceSize sz = (VkDeviceSize)cap * sizeof(mc_mtl_vtx);
        if (make_buf(sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, hp, &g_vb3, &g_vb3_mem,
                     &g_vb3_map) != 0)
            return -1;
        g_vb3_cap = cap;
    }
    if (n2 > g_vb2_cap) {
        if (g_vb2)
            vkDestroyBuffer(g_dev, g_vb2, 0);
        if (g_vb2_mem)
            vkFreeMemory(g_dev, g_vb2_mem, 0);
        g_vb2 = VK_NULL_HANDLE;
        g_vb2_mem = VK_NULL_HANDLE;
        g_vb2_map = 0;
        int cap = g_vb2_cap ? g_vb2_cap : 4096;
        while (cap < n2)
            cap *= 2;
        VkDeviceSize sz = (VkDeviceSize)cap * sizeof(mc_mtl_vtx);
        if (make_buf(sz, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, hp, &g_vb2, &g_vb2_mem,
                     &g_vb2_map) != 0)
            return -1;
        g_vb2_cap = cap;
    }
    return 0;
}

static VkVertexInputBindingDescription vbind(void)
{
    VkVertexInputBindingDescription b = {0};
    b.binding = 0;
    b.stride = (uint32_t)sizeof(mc_mtl_vtx);
    b.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return b;
}

static void vattrs(VkVertexInputAttributeDescription *a)
{
    a[0].location = 0;
    a[0].binding = 0;
    a[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    a[0].offset = 0;
    a[1].location = 1;
    a[1].binding = 0;
    a[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    a[1].offset = 12;
    a[2].location = 2;
    a[2].binding = 0;
    a[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    a[2].offset = 24;
    a[3].location = 3;
    a[3].binding = 0;
    a[3].format = VK_FORMAT_R32G32_SFLOAT;
    a[3].offset = 40;
}

static int make_pipe(VkShaderModule vs, VkShaderModule fs, int depth_test, VkPipeline *out)
{
    VkPipelineShaderStageCreateInfo st[2] = {0};
    st[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    st[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    st[0].module = vs;
    st[0].pName = "main";
    st[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    st[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    st[1].module = fs;
    st[1].pName = "main";

    VkVertexInputBindingDescription bind = vbind();
    VkVertexInputAttributeDescription attr[4];
    vattrs(attr);
    VkPipelineVertexInputStateCreateInfo vi = {0};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &bind;
    vi.vertexAttributeDescriptionCount = 4;
    vi.pVertexAttributeDescriptions = attr;

    VkPipelineInputAssemblyStateCreateInfo ia = {0};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vp = {0};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs = {0};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds = {0};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = depth_test ? VK_TRUE : VK_FALSE;
    ds.depthWriteEnable = depth_test ? VK_TRUE : VK_FALSE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState ba = {0};
    ba.blendEnable = VK_TRUE;
    ba.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ba.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ba.colorBlendOp = VK_BLEND_OP_ADD;
    ba.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ba.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ba.alphaBlendOp = VK_BLEND_OP_ADD;
    ba.colorWriteMask = 0xF;
    VkPipelineColorBlendStateCreateInfo cb = {0};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &ba;

    VkDynamicState dyns[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {0};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dyns;

    VkGraphicsPipelineCreateInfo pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pi.stageCount = 2;
    pi.pStages = st;
    pi.pVertexInputState = &vi;
    pi.pInputAssemblyState = &ia;
    pi.pViewportState = &vp;
    pi.pRasterizationState = &rs;
    pi.pMultisampleState = &ms;
    pi.pDepthStencilState = &ds;
    pi.pColorBlendState = &cb;
    pi.pDynamicState = &dyn;
    pi.layout = g_pl;
    pi.renderPass = g_rp;
    pi.subpass = 0;
    return vkCreateGraphicsPipelines(g_dev, VK_NULL_HANDLE, 1, &pi, 0, out) == VK_SUCCESS
               ? 0
               : -1;
}

static int pick_gpu(void)
{
    uint32_t n = 0;
    vkEnumeratePhysicalDevices(g_inst, &n, 0);
    if (!n)
        return -1;
    VkPhysicalDevice *devs = (VkPhysicalDevice *)malloc(sizeof(*devs) * n);
    if (!devs)
        return -1;
    vkEnumeratePhysicalDevices(g_inst, &n, devs);
    int found = -1;
    for (uint32_t i = 0; i < n && found < 0; i++) {
        uint32_t nq = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devs[i], &nq, 0);
        VkQueueFamilyProperties *qp = (VkQueueFamilyProperties *)malloc(sizeof(*qp) * nq);
        if (!qp)
            continue;
        vkGetPhysicalDeviceQueueFamilyProperties(devs[i], &nq, qp);
        for (uint32_t q = 0; q < nq; q++) {
            VkBool32 pres = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(devs[i], q, g_surf, &pres);
            if ((qp[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) && pres) {
                g_phys = devs[i];
                g_qfam = q;
                found = 0;
                break;
            }
        }
        free(qp);
    }
    free(devs);
    return found;
}

void mc_mtl_set_x11(void *display, unsigned long window)
{
    g_dpy = (Display *)display;
    g_win = (Window)window;
}

static void destroy_all(void)
{
    if (g_dev)
        vkDeviceWaitIdle(g_dev);
    destroy_swapchain();
    if (g_dev) {
        if (g_p3)
            vkDestroyPipeline(g_dev, g_p3, 0);
        if (g_p2)
            vkDestroyPipeline(g_dev, g_p2, 0);
        if (g_pl)
            vkDestroyPipelineLayout(g_dev, g_pl, 0);
        if (g_rp)
            vkDestroyRenderPass(g_dev, g_rp, 0);
        if (g_dpool)
            vkDestroyDescriptorPool(g_dev, g_dpool, 0);
        if (g_dsl)
            vkDestroyDescriptorSetLayout(g_dev, g_dsl, 0);
        if (g_samp)
            vkDestroySampler(g_dev, g_samp, 0);
        if (g_atlas_view)
            vkDestroyImageView(g_dev, g_atlas_view, 0);
        if (g_atlas)
            vkDestroyImage(g_dev, g_atlas, 0);
        if (g_atlas_mem)
            vkFreeMemory(g_dev, g_atlas_mem, 0);
        if (g_atlas_stg)
            vkDestroyBuffer(g_dev, g_atlas_stg, 0);
        if (g_atlas_stg_mem)
            vkFreeMemory(g_dev, g_atlas_stg_mem, 0);
        if (g_ubo)
            vkDestroyBuffer(g_dev, g_ubo, 0);
        if (g_ubo_mem)
            vkFreeMemory(g_dev, g_ubo_mem, 0);
        if (g_vb3)
            vkDestroyBuffer(g_dev, g_vb3, 0);
        if (g_vb3_mem)
            vkFreeMemory(g_dev, g_vb3_mem, 0);
        if (g_vb2)
            vkDestroyBuffer(g_dev, g_vb2, 0);
        if (g_vb2_mem)
            vkFreeMemory(g_dev, g_vb2_mem, 0);
        if (g_read)
            vkDestroyBuffer(g_dev, g_read, 0);
        if (g_read_mem)
            vkFreeMemory(g_dev, g_read_mem, 0);
        if (g_cpool)
            vkDestroyCommandPool(g_dev, g_cpool, 0);
        if (g_sem_acq)
            vkDestroySemaphore(g_dev, g_sem_acq, 0);
        if (g_sem_pres)
            vkDestroySemaphore(g_dev, g_sem_pres, 0);
        if (g_fence)
            vkDestroyFence(g_dev, g_fence, 0);
        vkDestroyDevice(g_dev, 0);
    }
    if (g_surf && g_inst)
        vkDestroySurfaceKHR(g_inst, g_surf, 0);
    if (g_inst)
        vkDestroyInstance(g_inst, 0);

    g_p3 = g_p2 = VK_NULL_HANDLE;
    g_pl = VK_NULL_HANDLE;
    g_rp = VK_NULL_HANDLE;
    g_dset = VK_NULL_HANDLE;
    g_dpool = VK_NULL_HANDLE;
    g_dsl = VK_NULL_HANDLE;
    g_samp = VK_NULL_HANDLE;
    g_atlas_view = VK_NULL_HANDLE;
    g_atlas = VK_NULL_HANDLE;
    g_atlas_mem = VK_NULL_HANDLE;
    g_atlas_stg = VK_NULL_HANDLE;
    g_atlas_stg_mem = VK_NULL_HANDLE;
    g_atlas_stg_map = 0;
    g_ubo = VK_NULL_HANDLE;
    g_ubo_mem = VK_NULL_HANDLE;
    g_ubo_map = 0;
    g_vb3 = g_vb2 = VK_NULL_HANDLE;
    g_vb3_mem = g_vb2_mem = VK_NULL_HANDLE;
    g_vb3_map = g_vb2_map = 0;
    g_vb3_cap = g_vb2_cap = 0;
    g_read = VK_NULL_HANDLE;
    g_read_mem = VK_NULL_HANDLE;
    g_read_map = 0;
    g_cmd = VK_NULL_HANDLE;
    g_cpool = VK_NULL_HANDLE;
    g_sem_acq = g_sem_pres = VK_NULL_HANDLE;
    g_fence = VK_NULL_HANDLE;
    g_dev = VK_NULL_HANDLE;
    g_surf = VK_NULL_HANDLE;
    g_inst = VK_NULL_HANDLE;
    g_phys = VK_NULL_HANDLE;
    g_q = VK_NULL_HANDLE;
    g_atlas_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void mc_mtl_shutdown(void)
{
    g_ready = 0;
    destroy_all();
    free(g_atlas_cpu);
    g_atlas_cpu = 0;
    g_atlas_dirty = 0;
    g_v3 = 0;
    g_n3 = 0;
    g_v2 = 0;
    g_n2 = 0;
}

int mc_mtl_init(void *native, int w, int h)
{
    (void)native;
    if (!g_dpy || !g_win || w < 32 || h < 32)
        return -1;
    g_w = w;
    g_h = h;
    memset(&g_u, 0, sizeof(g_u));
    g_u.light[0] = 0.35f;
    g_u.light[1] = 0.85f;
    g_u.light[2] = 0.4f;
    g_u.screen[0] = (float)w;
    g_u.screen[1] = (float)h;

    const char *iext[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    };
    VkApplicationInfo app = {0};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "minusC";
    app.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo ici = {0};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = 2;
    ici.ppEnabledExtensionNames = iext;
    if (vkCreateInstance(&ici, 0, &g_inst) != VK_SUCCESS)
        return -1;

    VkXlibSurfaceCreateInfoKHR xsi = {0};
    xsi.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    xsi.dpy = g_dpy;
    xsi.window = g_win;
    if (vkCreateXlibSurfaceKHR(g_inst, &xsi, 0, &g_surf) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    if (pick_gpu() != 0) {
        destroy_all();
        return -1;
    }
    if (pick_surface_format() != 0) {
        destroy_all();
        return -1;
    }

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci = {0};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = g_qfam;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;
    const char *dext[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo dci = {0};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = dext;
    if (vkCreateDevice(g_phys, &dci, 0, &g_dev) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    vkGetDeviceQueue(g_dev, g_qfam, 0, &g_q);

    VkAttachmentDescription atts[2] = {0};
    atts[0].format = g_col_fmt;
    atts[0].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    atts[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    atts[1].format = VK_FORMAT_D32_SFLOAT;
    atts[1].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference cref = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference dref = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    VkSubpassDescription sub = {0};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &cref;
    sub.pDepthStencilAttachment = &dref;
    VkSubpassDependency dep = {0};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo rpi = {0};
    rpi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpi.attachmentCount = 2;
    rpi.pAttachments = atts;
    rpi.subpassCount = 1;
    rpi.pSubpasses = &sub;
    rpi.dependencyCount = 1;
    rpi.pDependencies = &dep;

    if (vkCreateRenderPass(g_dev, &rpi, 0, &g_rp) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }

    VkDescriptorSetLayoutBinding lb[2] = {0};
    lb[0].binding = 0;
    lb[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lb[0].descriptorCount = 1;
    lb[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    lb[1].binding = 1;
    lb[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lb[1].descriptorCount = 1;
    lb[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo dsli = {0};
    dsli.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsli.bindingCount = 2;
    dsli.pBindings = lb;
    if (vkCreateDescriptorSetLayout(g_dev, &dsli, 0, &g_dsl) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    VkPipelineLayoutCreateInfo pli = {0};
    pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount = 1;
    pli.pSetLayouts = &g_dsl;
    if (vkCreatePipelineLayout(g_dev, &pli, 0, &g_pl) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }

    VkShaderModule vs3 = make_shader(mc_spv_v3d_vert, mc_spv_v3d_vert_words);
    VkShaderModule fs3 = make_shader(mc_spv_v3d_frag, mc_spv_v3d_frag_words);
    VkShaderModule vs2 = make_shader(mc_spv_v2d_vert, mc_spv_v2d_vert_words);
    VkShaderModule fs2 = make_shader(mc_spv_v2d_frag, mc_spv_v2d_frag_words);
    if (!vs3 || !fs3 || !vs2 || !fs2 ||
        make_pipe(vs3, fs3, 1, &g_p3) != 0 ||
        make_pipe(vs2, fs2, 0, &g_p2) != 0) {
        if (vs3) vkDestroyShaderModule(g_dev, vs3, 0);
        if (fs3) vkDestroyShaderModule(g_dev, fs3, 0);
        if (vs2) vkDestroyShaderModule(g_dev, vs2, 0);
        if (fs2) vkDestroyShaderModule(g_dev, fs2, 0);
        destroy_all();
        return -1;
    }
    vkDestroyShaderModule(g_dev, vs3, 0);
    vkDestroyShaderModule(g_dev, fs3, 0);
    vkDestroyShaderModule(g_dev, vs2, 0);
    vkDestroyShaderModule(g_dev, fs2, 0);

    if (create_swapchain() != 0) {
        destroy_all();
        return -1;
    }

    VkMemoryPropertyFlags hp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (make_buf(256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hp, &g_ubo, &g_ubo_mem,
                 &g_ubo_map) != 0) {
        destroy_all();
        return -1;
    }
    VkDeviceSize rsz = (VkDeviceSize)g_w * (VkDeviceSize)g_h * 4u;
    if (make_buf(rsz, VK_BUFFER_USAGE_TRANSFER_DST_BIT, hp, &g_read, &g_read_mem,
                 &g_read_map) != 0) {
        destroy_all();
        return -1;
    }

    g_atlas_cpu = build_atlas();
    if (!g_atlas_cpu) {
        destroy_all();
        return -1;
    }
    if (make_img(GUI_ATLAS_W, GUI_ATLAS_H, VK_FORMAT_R8G8B8A8_UNORM,
                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 &g_atlas, &g_atlas_mem) != 0) {
        destroy_all();
        return -1;
    }
    g_atlas_view = make_view(g_atlas, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    if (!g_atlas_view) {
        destroy_all();
        return -1;
    }
    VkDeviceSize asz = (VkDeviceSize)GUI_ATLAS_W * GUI_ATLAS_H * 4u;
    if (make_buf(asz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, hp, &g_atlas_stg,
                 &g_atlas_stg_mem, &g_atlas_stg_map) != 0) {
        destroy_all();
        return -1;
    }
    memcpy(g_atlas_stg_map, g_atlas_cpu, (size_t)asz);
    g_atlas_dirty = 0;
    g_atlas_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkSamplerCreateInfo spi = {0};
    spi.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    spi.magFilter = VK_FILTER_NEAREST;
    spi.minFilter = VK_FILTER_NEAREST;
    spi.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    spi.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    spi.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    spi.maxLod = 0.0f;
    if (vkCreateSampler(g_dev, &spi, 0, &g_samp) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }

    VkDescriptorPoolSize ps[2] = {0};
    ps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ps[0].descriptorCount = 1;
    ps[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ps[1].descriptorCount = 1;
    VkDescriptorPoolCreateInfo dpi = {0};
    dpi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpi.maxSets = 1;
    dpi.poolSizeCount = 2;
    dpi.pPoolSizes = ps;
    if (vkCreateDescriptorPool(g_dev, &dpi, 0, &g_dpool) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    VkDescriptorSetAllocateInfo dai = {0};
    dai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dai.descriptorPool = g_dpool;
    dai.descriptorSetCount = 1;
    dai.pSetLayouts = &g_dsl;
    if (vkAllocateDescriptorSets(g_dev, &dai, &g_dset) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    VkDescriptorBufferInfo dbi = {0};
    dbi.buffer = g_ubo;
    dbi.range = sizeof(mc_ubo);
    VkDescriptorImageInfo dii = {0};
    dii.sampler = g_samp;
    dii.imageView = g_atlas_view;
    dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet wr[2] = {0};
    wr[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wr[0].dstSet = g_dset;
    wr[0].dstBinding = 0;
    wr[0].descriptorCount = 1;
    wr[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wr[0].pBufferInfo = &dbi;
    wr[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wr[1].dstSet = g_dset;
    wr[1].dstBinding = 1;
    wr[1].descriptorCount = 1;
    wr[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wr[1].pImageInfo = &dii;
    vkUpdateDescriptorSets(g_dev, 2, wr, 0, 0);

    VkCommandPoolCreateInfo cpi = {0};
    cpi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cpi.queueFamilyIndex = g_qfam;
    if (vkCreateCommandPool(g_dev, &cpi, 0, &g_cpool) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    VkCommandBufferAllocateInfo cai = {0};
    cai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cai.commandPool = g_cpool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(g_dev, &cai, &g_cmd) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }
    VkSemaphoreCreateInfo sei = {0};
    sei.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fi = {0};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(g_dev, &sei, 0, &g_sem_acq) != VK_SUCCESS ||
        vkCreateSemaphore(g_dev, &sei, 0, &g_sem_pres) != VK_SUCCESS ||
        vkCreateFence(g_dev, &fi, 0, &g_fence) != VK_SUCCESS) {
        destroy_all();
        return -1;
    }

    /* Upload white atlas before the first draw. */
    {
        VkCommandBufferBeginInfo bi = {0};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(g_cmd, &bi);
        img_barrier(g_cmd, g_atlas, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                    0, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        VkBufferImageCopy cpy = {0};
        cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cpy.imageSubresource.layerCount = 1;
        cpy.imageExtent.width = GUI_ATLAS_W;
        cpy.imageExtent.height = GUI_ATLAS_H;
        cpy.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(g_cmd, g_atlas_stg, g_atlas,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
        img_barrier(g_cmd, g_atlas, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        vkEndCommandBuffer(g_cmd);
        VkSubmitInfo si = {0};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &g_cmd;
        vkQueueSubmit(g_q, 1, &si, VK_NULL_HANDLE);
        vkQueueWaitIdle(g_q);
        g_atlas_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    g_ready = 1;
    return 0;
}

void mc_mtl_set_mvp(const float *m)
{
    if (!m)
        return;
    memcpy(g_u.mvp, m, 16 * sizeof(float));
}

int mc_mtl_begin(int clear_rgb)
{
    g_clear_rgb = clear_rgb;
    return g_ready ? 0 : -1;
}

void mc_mtl_set_light(float x, float y, float z)
{
    g_u.light[0] = x;
    g_u.light[1] = y;
    g_u.light[2] = z;
    g_u.light[3] = 0.0f;
}

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

void mc_mtl_tex_flush(void)
{
    if (!g_ready || !g_atlas_dirty || !g_atlas_cpu || !g_atlas_stg_map)
        return;
    memcpy(g_atlas_stg_map, g_atlas_cpu, (size_t)GUI_ATLAS_W * GUI_ATLAS_H * 4u);
    /* Actual GPU copy happens in present() so it shares the frame command buffer. */
}

int mc_mtl_present(void)
{
    if (!g_ready || !g_swap)
        return -1;
    g_u.screen[0] = (float)g_w;
    g_u.screen[1] = (float)g_h;
    memcpy(g_ubo_map, &g_u, sizeof(g_u));

    if (ensure_vbuf(g_n3 > 0 ? g_n3 : 1, g_n2 > 0 ? g_n2 : 1) != 0)
        return -1;
    if (g_n3 > 0 && g_v3 && g_vb3_map)
        memcpy(g_vb3_map, g_v3, (size_t)g_n3 * sizeof(mc_mtl_vtx));
    if (g_n2 > 0 && g_v2 && g_vb2_map)
        memcpy(g_vb2_map, g_v2, (size_t)g_n2 * sizeof(mc_mtl_vtx));

    vkWaitForFences(g_dev, 1, &g_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(g_dev, 1, &g_fence);

    uint32_t idx = 0;
    VkResult ar = vkAcquireNextImageKHR(g_dev, g_swap, UINT64_MAX, g_sem_acq,
                                        VK_NULL_HANDLE, &idx);
    if (ar == VK_ERROR_OUT_OF_DATE_KHR) {
        destroy_swapchain();
        if (create_swapchain() != 0)
            return -1;
        g_n3 = g_n2 = 0;
        g_v3 = g_v2 = 0;
        return 0;
    }
    if (ar != VK_SUCCESS && ar != VK_SUBOPTIMAL_KHR)
        return -1;
    if (idx >= g_nimg)
        return -1;

    vkResetCommandBuffer(g_cmd, 0);
    VkCommandBufferBeginInfo bi = {0};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(g_cmd, &bi);

    if (g_atlas_dirty) {
        memcpy(g_atlas_stg_map, g_atlas_cpu, (size_t)GUI_ATLAS_W * GUI_ATLAS_H * 4u);
        img_barrier(g_cmd, g_atlas, g_atlas_layout,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        VkBufferImageCopy cpy = {0};
        cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cpy.imageSubresource.layerCount = 1;
        cpy.imageExtent.width = GUI_ATLAS_W;
        cpy.imageExtent.height = GUI_ATLAS_H;
        cpy.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(g_cmd, g_atlas_stg, g_atlas,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
        img_barrier(g_cmd, g_atlas, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        g_atlas_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        g_atlas_dirty = 0;
    }

    VkClearValue cv[2];
    float cr = ((g_clear_rgb >> 16) & 255) / 255.0f;
    float cg = ((g_clear_rgb >> 8) & 255) / 255.0f;
    float cb = (g_clear_rgb & 255) / 255.0f;
    cv[0].color.float32[0] = cr;
    cv[0].color.float32[1] = cg;
    cv[0].color.float32[2] = cb;
    cv[0].color.float32[3] = 1.0f;
    cv[1].depthStencil.depth = 1.0f;
    cv[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rbi = {0};
    rbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rbi.renderPass = g_rp;
    rbi.framebuffer = g_fb[idx];
    rbi.renderArea.extent.width = (uint32_t)g_w;
    rbi.renderArea.extent.height = (uint32_t)g_h;
    rbi.clearValueCount = 2;
    rbi.pClearValues = cv;
    vkCmdBeginRenderPass(g_cmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport vp = {0};
    vp.width = (float)g_w;
    vp.height = (float)g_h;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    VkRect2D sc = {0};
    sc.extent.width = (uint32_t)g_w;
    sc.extent.height = (uint32_t)g_h;
    vkCmdSetViewport(g_cmd, 0, 1, &vp);
    vkCmdSetScissor(g_cmd, 0, 1, &sc);
    vkCmdBindDescriptorSets(g_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pl, 0, 1,
                            &g_dset, 0, 0);

    VkDeviceSize off = 0;
    if (g_n3 > 0 && g_vb3) {
        vkCmdBindPipeline(g_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_p3);
        vkCmdBindVertexBuffers(g_cmd, 0, 1, &g_vb3, &off);
        vkCmdDraw(g_cmd, (uint32_t)g_n3, 1, 0, 0);
    }
    if (g_n2 > 0 && g_vb2) {
        vkCmdBindPipeline(g_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_p2);
        vkCmdBindVertexBuffers(g_cmd, 0, 1, &g_vb2, &off);
        vkCmdDraw(g_cmd, (uint32_t)g_n2, 1, 0, 0);
    }
    vkCmdEndRenderPass(g_cmd);

    if (g_can_copy && g_read) {
        img_barrier(g_cmd, g_imgs[idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT);
        VkBufferImageCopy cpy = {0};
        cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cpy.imageSubresource.layerCount = 1;
        cpy.imageExtent.width = (uint32_t)g_w;
        cpy.imageExtent.height = (uint32_t)g_h;
        cpy.imageExtent.depth = 1;
        vkCmdCopyImageToBuffer(g_cmd, g_imgs[idx], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               g_read, 1, &cpy);
        img_barrier(g_cmd, g_imgs[idx], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_ACCESS_TRANSFER_READ_BIT, 0,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    } else {
        img_barrier(g_cmd, g_imgs[idx], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }

    vkEndCommandBuffer(g_cmd);

    VkPipelineStageFlags wait = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {0};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &g_sem_acq;
    si.pWaitDstStageMask = &wait;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &g_cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &g_sem_pres;
    if (vkQueueSubmit(g_q, 1, &si, g_fence) != VK_SUCCESS)
        return -1;

    VkPresentInfoKHR pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &g_sem_pres;
    pi.swapchainCount = 1;
    pi.pSwapchains = &g_swap;
    pi.pImageIndices = &idx;
    VkResult pr = vkQueuePresentKHR(g_q, &pi);
    vkWaitForFences(g_dev, 1, &g_fence, VK_TRUE, UINT64_MAX);

    g_v3 = 0;
    g_n3 = 0;
    g_v2 = 0;
    g_n2 = 0;
    if (pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR) {
        destroy_swapchain();
        create_swapchain();
    }
    return (pr == VK_SUCCESS || pr == VK_SUBOPTIMAL_KHR ||
            pr == VK_ERROR_OUT_OF_DATE_KHR)
               ? 0
               : -1;
}

int mc_mtl_readback(unsigned int *out_rgb, int w, int h)
{
    if (!g_ready || !g_read_map || !out_rgb)
        return -1;
    if (w != g_w || h != g_h)
        return -1;
    const unsigned char *src = (const unsigned char *)g_read_map;
    for (int i = 0; i < w * h; i++) {
        unsigned char b, g, r;
        if (g_swap_bgra) {
            b = src[i * 4 + 0];
            g = src[i * 4 + 1];
            r = src[i * 4 + 2];
        } else {
            r = src[i * 4 + 0];
            g = src[i * 4 + 1];
            b = src[i * 4 + 2];
        }
        out_rgb[i] = (unsigned int)b | ((unsigned int)g << 8) | ((unsigned int)r << 16);
    }
    return 0;
}
