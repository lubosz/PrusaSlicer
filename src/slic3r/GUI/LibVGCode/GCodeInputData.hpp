///|/ Copyright (c) Prusa Research 2023 Enrico Turri @enricoturri1966
///|/
///|/ libvgcode is released under the terms of the AGPLv3 or higher
///|/
#ifndef VGCODE_GCODEINPUTDATA_HPP
#define VGCODE_GCODEINPUTDATA_HPP

//################################################################################################################################
// PrusaSlicer development only -> !!!TO BE REMOVED!!!
#if ENABLE_NEW_GCODE_VIEWER
//################################################################################################################################

#include "PathVertex.hpp"

namespace libvgcode {

struct GCodeInputData
{
    //
    // List of path vertices
    //
    std::vector<PathVertex> vertices;

    //
    // Total time for each time mode
    //
    std::array<float, static_cast<size_t>(ETimeMode::COUNT)> times{ 0.0f, 0.0f };
};

} // namespace libvgcode

//################################################################################################################################
// PrusaSlicer development only -> !!!TO BE REMOVED!!!
#endif // ENABLE_NEW_GCODE_VIEWER
//################################################################################################################################

#endif // VGCODE_BITSET_HPP
