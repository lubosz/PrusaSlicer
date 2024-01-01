/*
 * PrusaSlicer
 * Copyright 2023 Lubosz Sarnecki <lubosz@gmail.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "TAAManager.hpp"

#include <cstdio>

#include "3DScene.hpp"
#include "GLCanvas3D.hpp"
#include "GLShader.hpp"
#include "GUI_App.hpp"

namespace Slic3r {
namespace GUI {

TAAManager::TAAManager() {}

void TAAManager::init(const Size& canvas_size)
{
    if (m_gl_data_initialized)
        return;

    initGL(canvas_size);

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

    float vertices[] = {
        // pos xy, tex coord uv
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_plane_vertex_array);
    glGenBuffers(1, &m_plane_vertex_buffer);
    glBindVertexArray(m_plane_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, m_plane_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

}

void TAAManager::initFrameBuffers(const Size& canvas_size) {
    glGenTextures(1, &m_color_texture_array);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_color_texture_array);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB,
                 static_cast<GLsizei>(canvas_size.get_width()),
                 static_cast<GLsizei>(canvas_size.get_height()),
                 m_num_buffers, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (uint32_t i = 0; i < m_num_buffers; i++) {
        Pass pass = {};

        glsafe(::glGenFramebuffers(1, &pass.frame_buffer));
        //glsafe(::glGenRenderbuffers(1, &pass.color_render_buffer));
        // glsafe(::glGenTextures(1, &pass.color_texture));
        glsafe(::glGenRenderbuffers(1, &pass.depth_render_buffer));

        glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, pass.frame_buffer));

        // glBindTexture(GL_TEXTURE_2D, pass.color_texture);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(canvas_size.get_width()), static_cast<GLsizei>(canvas_size.get_height()), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass.color_texture, 0);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_color_texture_array, 0, i);


        //glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, pass.color_render_buffer));
        //glsafe(::glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
        //glsafe(::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, pass.color_render_buffer));

        glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, pass.depth_render_buffer));
        glsafe(::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(canvas_size.get_width()), static_cast<GLsizei>(canvas_size.get_height())));
        glsafe(::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pass.depth_render_buffer));

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("ERROR: Framebuffer incomplete.\n");
        }

        glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, 0));
        glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, 0));
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        m_passes.push_back(pass);
    }
}

void TAAManager::initGL(const Size& canvas_size) {
    initFrameBuffers(canvas_size);
    initVertices();
}

void TAAManager::clearFrameBuffers() {
    printf("Shutting down TAAManager!\n");
    glsafe(::glDeleteTextures(1, &m_color_texture_array));
    for (const Pass& pass : m_passes) {
        glsafe(::glDeleteRenderbuffers(1, &pass.depth_render_buffer));
        // glsafe(::glDeleteTextures(1, &pass.color_texture));
        //glsafe(::glDeleteRenderbuffers(1, &pass.color_render_buffer));
        glsafe(::glDeleteFramebuffers(1, &pass.frame_buffer));
    }
    m_passes.clear();
}

void TAAManager::resize(unsigned int w, unsigned int h) {
    clearFrameBuffers();
    Size size = {(int) w, (int) h};
    initFrameBuffers(size);
}

void TAAManager::begin_frame() {
    glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, m_passes[0].frame_buffer));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void TAAManager::end_frame() {
    glsafe(::glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glsafe(::glBindRenderbuffer(GL_RENDERBUFFER, 0));

}

void TAAManager::display_frame() {
    GLShaderProgram* shader = wxGetApp().get_shader("taa");
    if (shader == nullptr)
        return;

    shader->start_using();
    shader->set_uniform("tex", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_color_texture_array);

    glBindVertexArray(m_plane_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader->stop_using();
}

} // namespace GUI
} // namespace Slic3r
