/*
 * PrusaSlicer
 * Copyright 2023 Lubosz Sarnecki <lubosz@gmail.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */


#version 140

in vec2 v_position;
in vec2 v_tex_coord;

out vec2 tex_coord;

void main()
{
    tex_coord = v_tex_coord;
    gl_Position = vec4(v_position, 0.0, 1.0);
}
