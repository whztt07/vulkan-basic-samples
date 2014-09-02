/*
 * XGL
 *
 * Copyright (C) 2014 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Courtney Goeltzenleuchter <courtney@lunarg.com>
 *   Chia-I Wu <olv@lunarg.com>
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include "intel.h"
#include "obj.h"
#include "dev.h"

#define INTEL_RMAP_SLOT_RT ((XGL_UINT) -1)
#define INTEL_RMAP_SLOT_DYN ((XGL_UINT) -2)
struct intel_rmap_slot {
    /*
     *
     * When path_len is 0, the slot is unused.
     * When path_len is 1, the slot uses descriptor "index".
     * When path_len is INTEL_RMAP_SLOT_RT, the slot uses RT "index".
     * When path_len is INTEL_RMAP_SLOT_DYN, the slot uses the dynamic view.
     * Otherwise, the slot uses "path" to find the descriptor.
     */
    XGL_UINT path_len;

    union {
        XGL_UINT index;
        XGL_UINT *path;
    } u;
};

/**
 * Shader resource mapping.
 */
struct intel_rmap {
    /* this is not an intel_obj */

    XGL_UINT rt_count;
    XGL_UINT resource_count;
    XGL_UINT uav_count;
    XGL_UINT sampler_count;

    /*
     * rt_count slots +
     * resource_count slots +
     * uav_count slots +
     * sampler_count slots
     */
    struct intel_rmap_slot *slots;
    XGL_UINT slot_count;
};

#define SHADER_VERTEX_FLAG            (1 << XGL_SHADER_STAGE_VERTEX)
#define SHADER_TESS_CONTROL_FLAG      (1 << XGL_SHADER_STAGE_TESS_CONTROL)
#define SHADER_TESS_EVAL_FLAG         (1 << XGL_SHADER_STAGE_TESS_EVALUATION)
#define SHADER_GEOMETRY_FLAG          (1 << XGL_SHADER_STAGE_GEOMETRY)
#define SHADER_FRAGMENT_FLAG          (1 << XGL_SHADER_STAGE_FRAGMENT)
#define SHADER_COMPUTE_FLAG           (1 << XGL_SHADER_STAGE_COMPUTE)

struct intel_pipeline_shader {
    /* this is not an intel_obj */

    void *pCode;
    uint32_t codeSize;

    /*
     * must grab everything we need from shader object as that
     * can go away after the pipeline is created
     */
    XGL_FLAGS uses;

    XGL_UINT in_count;
    XGL_UINT out_count;

    XGL_UINT sampler_count;
    XGL_UINT surface_count;

    /*
     * Used by 3DSTATE_VS command
     */
    XGL_UINT urb_grf_start;
    XGL_UINT urb_read_length;

    XGL_FLAGS barycentric_interps;

    struct intel_rmap *rmap;

    void *pcb;
    XGL_SIZE pcb_size;
};

/*
 * On GEN6, there are
 *
 *  - 3DSTATE_URB (3)
 *  - 3DSTATE_VERTEX_ELEMENTS (3)
 *
 * On GEN7, there are
 *
 *  - 3DSTATE_URB_x (2*4)
 *  - 3DSTATE_PUSH_CONSTANT_ALLOC_x (2*5)
 *  - 3DSTATE_VERTEX_ELEMENTS (3)
 *  - 3DSTATE_HS (7)
 *  - 3DSTATE_TE (4)
 *  - 3DSTATE_DS (6)
 */
#define INTEL_PSO_CMD_ENTRIES   64

/**
 * 3D pipeline.
 */
struct intel_pipeline {
    struct intel_obj obj;

    struct intel_dev *dev;

    /* XGL IA_STATE */
    XGL_PIPELINE_IA_STATE_CREATE_INFO ia_state;
    int prim_type;
    bool primitive_restart;
    uint32_t primitive_restart_index;

    /* Index of provoking vertex for each prim type */
    int provoking_vertex_tri;
    int provoking_vertex_trifan;
    int provoking_vertex_line;

    // TODO: This should probably be Intel HW state, not XGL state.
    /* Depth Buffer format */
    XGL_FORMAT db_format;

    XGL_PIPELINE_CB_STATE cb_state;

    // XGL_PIPELINE_RS_STATE_CREATE_INFO rs_state;
    bool depthClipEnable;
    bool rasterizerDiscardEnable;
    float pointSize;

    XGL_PIPELINE_TESS_STATE_CREATE_INFO tess_state;

    uint32_t active_shaders;
    struct intel_pipeline_shader vs;
    struct intel_pipeline_shader tcs;
    struct intel_pipeline_shader tes;
    struct intel_pipeline_shader gs;
    struct intel_pipeline_shader fs;
    struct intel_pipeline_shader cs;

    uint32_t wa_flags;

    uint32_t cmds[INTEL_PSO_CMD_ENTRIES];
    XGL_UINT cmd_len;
};

static inline struct intel_pipeline *intel_pipeline(XGL_PIPELINE pipeline)
{
    return (struct intel_pipeline *) pipeline;
}

static inline struct intel_pipeline *intel_pipeline_from_base(struct intel_base *base)
{
    return (struct intel_pipeline *) base;
}

static inline struct intel_pipeline *intel_pipeline_from_obj(struct intel_obj *obj)
{
    return intel_pipeline_from_base(&obj->base);
}

XGL_RESULT XGLAPI intelCreateGraphicsPipeline(
    XGL_DEVICE                                  device,
    const XGL_GRAPHICS_PIPELINE_CREATE_INFO*    pCreateInfo,
    XGL_PIPELINE*                               pPipeline);

XGL_RESULT XGLAPI intelCreateComputePipeline(
    XGL_DEVICE                                  device,
    const XGL_COMPUTE_PIPELINE_CREATE_INFO*     pCreateInfo,
    XGL_PIPELINE*                               pPipeline);

XGL_RESULT XGLAPI intelStorePipeline(
    XGL_PIPELINE                                pipeline,
    XGL_SIZE*                                   pDataSize,
    XGL_VOID*                                   pData);

XGL_RESULT XGLAPI intelLoadPipeline(
    XGL_DEVICE                                  device,
    XGL_SIZE                                    dataSize,
    const XGL_VOID*                             pData,
    XGL_PIPELINE*                               pPipeline);

XGL_RESULT XGLAPI intelCreatePipelineDelta(
    XGL_DEVICE                                  device,
    XGL_PIPELINE                                p1,
    XGL_PIPELINE                                p2,
    XGL_PIPELINE_DELTA*                         delta);

#endif /* PIPELINE_H */
