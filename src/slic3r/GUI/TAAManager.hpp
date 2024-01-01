/*
 * PrusaSlicer
 * Copyright 2023 Lubosz Sarnecki <lubosz@gmail.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include <vector>

#include <cstdint>

#include <GL/glew.h>

namespace Slic3r {
namespace GUI {

class Size;

struct Pass
{
  GLuint frame_buffer;
  // GLuint color_render_buffer;
  GLuint color_texture;
  GLuint depth_render_buffer;
};

class TAAManager
{
  public:
    TAAManager();
    ~TAAManager() { shutdownGL(); }

    void init(const Size& canvas_size);
    void begin_frame();
    void end_frame();
    void display_frame();

  private:
    void shutdownGL();
    void initGL(const Size& canvas_size);
    void initFrameBuffers(const Size& canvas_size);
    void initVertices();

    bool m_gl_data_initialized = false;
    uint32_t m_num_buffers = 8;
    GLuint m_plane_vertex_array, m_plane_vertex_buffer;
    std::vector<Pass> m_passes;
};

} // namespace GUI
} // namespace Slic3r