#pragma once

#ifndef QUAKE_COLORTYPES_HPP
#define QUAKE_COLORTYPES_HPP

namespace Device::GPU {
    enum class ColorType {
        G,
        GA,
        RGB,
        RGBA,
        PALETTE,
        DEPTH,
        DEPTH_STENCIL
    };
}

#endif //QUAKE_COLORTYPES_HPP
