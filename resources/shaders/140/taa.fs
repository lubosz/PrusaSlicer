/*
 * PrusaSlicer
 * Copyright 2023 Lubosz Sarnecki <lubosz@gmail.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#version 140

in vec2 tex_coord;

out vec4 out_color;


void main()
{
    out_color = vec4(tex_coord, 0, 1);
}
