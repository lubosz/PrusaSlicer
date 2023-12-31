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

    GLuint render_fbo;
    glsafe(::glGenFramebuffers(1, &render_fbo));
    glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, render_fbo));

    // GLuint render_tex = 0;
    // glsafe(::glGenTextures(1, &render_tex));
    // glsafe(::glBindTexture(GL_TEXTURE_2D, render_tex));
    // glsafe(::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
    //                       static_cast<GLsizei>(width),
    //                       static_cast<GLsizei>(height),
    //                       0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    // glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    // glsafe(::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    // glsafe(::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0));

    GLuint render_tex_buffer = 0;
    glsafe(::glGenRenderbuffers(1, &render_tex_buffer));
    glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, render_tex_buffer));
    // glsafe(::glRenderbufferStorageMultisample(GL_RENDERBUFFER, num_samples, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
    glsafe(::glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
    glsafe(::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_tex_buffer));


    GLuint render_depth;
    glsafe(::glGenRenderbuffers(1, &render_depth));
    glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, render_depth));
    glsafe(::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
    glsafe(::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_depth));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("ERROR: Framebuffer incomplete.\n");
    }

    glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void TAAManager::shutdownGL() {
    // glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, 0));
    // glsafe(::glDeleteRenderbuffers(1, &render_depth));
    // if (render_tex_buffer != 0)
    //     glsafe(::glDeleteRenderbuffers(1, &render_tex_buffer));
    // if (render_tex != 0)
    //     glsafe(::glDeleteTextures(1, &render_tex));
    // glsafe(::glDeleteFramebuffers(1, &render_fbo));
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