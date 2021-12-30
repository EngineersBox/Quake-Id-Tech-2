#pragma once

#ifndef QUAKE_IMAGE_HPP
#define QUAKE_IMAGE_HPP

#include <istream>
#include <vector>
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <glm/glm.hpp>

#include "resource.hpp"
#include "../rendering/gpu/colorTypes.hpp"

namespace Resources {
    struct Image : Resource {
        typedef std::vector<unsigned char> DataType;
        typedef int BitDepthType;
        typedef glm::vec2 SizeType;

        Image() = default;
        Image(std::istream& istream);
        Image(const SizeType& size, BitDepthType bit_depth, Rendering::GPU::ColorType color_type, const unsigned char* data_ptr, size_t data_size);

        [[nodiscard]] BitDepthType get_bit_depth() const { return bit_depth; }
        [[nodiscard]] Rendering::GPU::ColorType get_color_type() const { return color_type; }
        [[nodiscard]] const DataType& get_data() const { return data; }
        [[nodiscard]] const SizeType& get_size() const { return size; }
        [[nodiscard]] unsigned int get_width() const { return static_cast<unsigned int>(size.x); }
        [[nodiscard]] unsigned int get_height() const { return static_cast<unsigned int>(size.y); }
        [[nodiscard]] size_t get_pixel_stride() const { return pixel_stride; }
        [[nodiscard]] size_t get_channel_count() const;
        std::mutex& get_data_mutex() { return data_mutex; }

    private:
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        SizeType size;
        BitDepthType bit_depth = 0;
        Rendering::GPU::ColorType color_type = Rendering::GPU::ColorType::G;
        DataType data;
        size_t pixel_stride = 1;
        std::mutex data_mutex;

        friend std::ostream& operator<<(std::ostream& ostream, Image& image);
    };
}

#endif //QUAKE_IMAGE_HPP
