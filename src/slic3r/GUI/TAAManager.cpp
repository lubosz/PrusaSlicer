/*
 * PrusaSlicer
 * Copyright 2023 Lubosz Sarnecki <lubosz@gmail.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "TAAManager.hpp"

#include <cstdio>

#include "3DScene.hpp"

namespace Slic3r {
namespace GUI {

TAAManager::TAAManager() {}

void TAAManager::init()
{
    if (m_gl_data_initialized)
        return;

    printf("Initializing TAAManager!\n");

    initGL(1920, 1080);

    m_gl_data_initialized = true;
}

void TAAManager::initGL(uint32_t width, uint32_t height) {
    for (uint32_t i = 0; i < m_num_buffers; i++) {
        Pass pass = {};

        glsafe(::glGenFramebuffers(1, &pass.frame_buffer));
        glsafe(::glGenRenderbuffers(1, &pass.color_render_buffer));
        glsafe(::glGenRenderbuffers(1, &pass.depth_render_buffer));

        glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, pass.frame_buffer));
        glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, pass.color_render_buffer));
        glsafe(::glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
        glsafe(::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, pass.color_render_buffer));

        glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, pass.depth_render_buffer));
        glsafe(::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
        glsafe(::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pass.depth_render_buffer));

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("ERROR: Framebuffer incomplete.\n");
        }

        glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, 0));
        glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, 0));

        m_passes.push_back(pass);
    }

}

void TAAManager::shutdownGL() {
    for (const Pass& pass : m_passes) {
        glsafe(::glDeleteRenderbuffers(1, &pass.depth_render_buffer));
        glsafe(::glDeleteRenderbuffers(1, &pass.color_render_buffer));
        glsafe(::glDeleteFramebuffers(1, &pass.frame_buffer));
    }
    m_passes.clear();
}

void TAAManager::begin_frame() {
    GLenum drawBufs[] = { GL_COLOR_ATTACHMENT0 };
    glsafe(::glDrawBuffers(1, drawBufs));

}

void TAAManager::end_frame() {

}

void TAAManager::display_frame() {

}

} // namespace GUI
} // namespace Slic3r
