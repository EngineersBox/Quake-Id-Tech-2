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
        Image(const SizeType& size, BitDepthType bitDepth, Rendering::GPU::ColorType colorType, const unsigned char* dataPtr, size_t dataSize);

        [[nodiscard]] BitDepthType getBitDepth() const { return this->bitDepth; }
        [[nodiscard]] Rendering::GPU::ColorType getColorType() const { return this->colorType; }
        [[nodiscard]] const DataType& getData() const { return this->data; }
        [[nodiscard]] const SizeType& getSize() const { return this->size; }
        [[nodiscard]] unsigned int getWidth() const { return static_cast<unsigned int>(this->size.x); }
        [[nodiscard]] unsigned int getHeight() const { return static_cast<unsigned int>(this->size.y); }
        [[nodiscard]] size_t getPixelStride() const { return this->pixelStride; }
        [[nodiscard]] size_t getChannelCount() const;
        std::mutex& getDataMutex() { return this->dataMutex; }

    private:
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        SizeType size;
        BitDepthType bitDepth = 0;
        Rendering::GPU::ColorType colorType = Rendering::GPU::ColorType::G;
        DataType data;
        size_t pixelStride = 1;
        std::mutex dataMutex;

        friend std::ostream& operator<<(std::ostream& ostream, Image& image);
    };
}

#endif //QUAKE_IMAGE_HPP
