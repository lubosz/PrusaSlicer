/*
 * PrusaSlicer
 * Copyright 2023 Lubosz Sarnecki <lubosz@gmail.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include "OpenGLManager.hpp"

namespace Slic3r {
namespace GUI {

struct Pass
{
  GLuint frame_buffer;
  GLuint color_render_buffer;
  GLuint depth_render_buffer;
};

class TAAManager
{
  public:
    TAAManager();
    ~TAAManager() { shutdownGL(); }

    void init();
    void begin_frame();
    void end_frame();
    void display_frame();

  private:
    void shutdownGL();
    void initGL(uint32_t width, uint32_t height);

    bool m_gl_data_initialized = false;
    uint32_t m_num_buffers = 8;
};

} // namespace GUI
} // namespace Slic3r