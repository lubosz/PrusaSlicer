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

// typedef struct
// {
//     float position[2];
//     float uv[2];
// } Vertex;


void TAAManager::initVertices() {

    // static const Vertex vertices[4] = {
    //     {{-1.f, -1.f}, {1.f, 0.f}},
    //     {{ 1.f, -1.f}, {0.f, 0.f}},
    //     {{ 1.f,  1.f}, {0.f, 1.f}},
    //     {{-1.f,  1.f}, {1.f, 1.f}}
    // };

    float vertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int vertex_array, vertex_buffer;
    glGenVertexArrays(1, &vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

}

void TAAManager::initFrameBuffers(uint32_t width, uint32_t height) {
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

void TAAManager::initGL(uint32_t width, uint32_t height) {
    initFrameBuffers(width, height);
}

void TAAManager::shutdownGL() {
    printf("Shutting down TAAManager!\n");
    for (const Pass& pass : m_passes) {
        glsafe(::glDeleteRenderbuffers(1, &pass.depth_render_buffer));
        glsafe(::glDeleteRenderbuffers(1, &pass.color_render_buffer));
        glsafe(::glDeleteFramebuffers(1, &pass.frame_buffer));
    }
    m_passes.clear();
}

void TAAManager::begin_frame() {
    glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, m_passes[0].frame_buffer));

}

void TAAManager::end_frame() {
    glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void TAAManager::display_frame() {

}

} // namespace GUI
} // namespace Slic3r
