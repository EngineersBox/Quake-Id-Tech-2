#pragma once

#ifndef QUAKE_TEXTURE_HPP
#define QUAKE_TEXTURE_HPP

#include "resource.hpp"
#include "image.hpp"
#include "../rendering/gpu/colorTypes.hpp"
#include "../rendering/gpu/gpuDefs.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace Resources {
    struct Image;
    struct Texture : Resource, boost::enable_shared_from_this<Texture> {
        typedef Rendering::GPU::GpuId IdType;
        typedef int FormatType;
        typedef int TypeType;

        Texture(Rendering::GPU::ColorType color_type, const glm::vec2& size, const void* data);
        Texture(const boost::shared_ptr<Image>& image);
        Texture(std::istream& istream);
        virtual ~Texture();

        Rendering::GPU::ColorType get_color_type() const { return color_type; }
        const glm::vec2& get_size() const { return size; }
        unsigned int get_width() const { return static_cast<unsigned int>(size.x); }
        unsigned int get_height() const { return static_cast<unsigned int>(size.y); }

        IdType get_id() const { return id; }

        void set_size(const glm::vec2& size);

    private:
        Rendering::GPU::ColorType color_type;
        glm::vec2 size;
        IdType id = 0;

        Texture(Texture&) = delete;
        Texture& operator=(Texture&) = delete;
    };
}

#endif //QUAKE_TEXTURE_HPP
