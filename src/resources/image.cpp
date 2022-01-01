#include "image.hpp"

#include <png.h>
#include <iostream>

namespace Resources {
    Image::Image(std::istream& istream) {
        //TODO: determine what the stream actually contains (don't assume PNG!)
        png_struct_def* pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (pngPtr == nullptr) throw std::runtime_error("Could not create PNG read struct");

        png_info_def* infoPtr = png_create_info_struct(pngPtr);
        if (infoPtr == nullptr) {
            png_destroy_read_struct(&pngPtr, nullptr, nullptr);
            throw std::runtime_error("Could not create PNG info struct");
        }

        if (setjmp(png_jmpbuf(pngPtr))) {
            png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
            throw std::runtime_error("Could not destroy PNG read struct");
        }

        png_set_read_fn(pngPtr, static_cast<png_voidp>(&istream), [](png_structp png_ptr, png_bytep _data, png_size_t length) {
            void* pngIoPtr = png_get_io_ptr(png_ptr);
            static_cast<std::istream*>(pngIoPtr)->read(reinterpret_cast<char*>(_data), length);
        });

        png_uint_32 sigRead = 0;
        png_set_sig_bytes(pngPtr, sigRead);
        png_read_png(pngPtr, infoPtr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, nullptr);

        int pngColorType;
        png_int_32 interlaceMethod;
        png_uint_32 width, height;

        png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &pngColorType, &interlaceMethod, nullptr, nullptr);

        this->size.x = static_cast<float>(width);
        this->size.y = static_cast<float>(height);

        auto getColorType = [](int png_color_type) -> Rendering::GPU::ColorType {
            switch (png_color_type) {
                case PNG_COLOR_TYPE_GRAY: return Rendering::GPU::ColorType::G;
                case PNG_COLOR_TYPE_RGB: return Rendering::GPU::ColorType::RGB;
                case PNG_COLOR_TYPE_PALETTE: return Rendering::GPU::ColorType::PALETTE;
                case PNG_COLOR_TYPE_GA: return Rendering::GPU::ColorType::GA;
                case PNG_COLOR_TYPE_RGBA: return Rendering::GPU::ColorType::RGBA;
                default: throw std::exception();
            }
        };

        this->colorType = getColorType(pngColorType);
        unsigned long rowBytes = png_get_rowbytes(pngPtr, infoPtr);
        this->pixelStride = rowBytes / getWidth();
        unsigned long dataLength = rowBytes * getHeight();
        this->data.resize(dataLength);
        unsigned char** rowPointers = png_get_rows(pngPtr, infoPtr);

        for (unsigned int i = 0; i < size.y; ++i) {
            memcpy(data.data() + (rowBytes * (getHeight() - 1 - i)), rowPointers[i], rowBytes);
        }

        png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
    }

    Image::Image(const SizeType& size, BitDepthType bitDepth, Rendering::GPU::ColorType colorType, const unsigned char* dataPtr, size_t dataSize) :
            size(size),
            bitDepth(bitDepth),
            colorType(colorType) {
        data.resize(dataSize);
        if (dataSize <= data.size())
            memcpy(data.data(), dataPtr, dataSize);
    }


    size_t Image::getChannelCount() const {
        switch (colorType) {
            case Rendering::GPU::ColorType::G:
            case Rendering::GPU::ColorType::PALETTE:
                return 1;
            case Rendering::GPU::ColorType::GA: return 2;
            case Rendering::GPU::ColorType::RGB: return 3;
            case Rendering::GPU::ColorType::RGBA: return 4;
            default: return 0;
        }
    }

    std::string pngErrorMessage;
    std::ostream& operator<<(std::ostream& ostream, Image& image) {
        png_struct_def* pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!pngPtr) {
            throw std::runtime_error("Could not create PNG write struct");
        }

        png_info_def* info_ptr = png_create_info_struct(pngPtr);
        if (!info_ptr) {
            png_destroy_write_struct(&pngPtr, nullptr);
            throw std::runtime_error("Could not create PNG info struct");
        }

        std::exception exception;
        png_error_ptr errorFn = [](png_structp png_ptr, png_const_charp message) {
            pngErrorMessage = message;
        };

        png_set_error_fn(pngPtr, png_get_error_ptr(pngPtr), errorFn, NULL);

        if (setjmp(png_jmpbuf(pngPtr))) {
            void* errorPtr = png_get_error_ptr(pngPtr);
            png_destroy_write_struct(&pngPtr, &info_ptr);
            throw std::runtime_error(pngErrorMessage.c_str());
        }

        auto getPngColorType = [](Rendering::GPU::ColorType color_type) -> int {
            switch (color_type) {
                case Rendering::GPU::ColorType::G: return PNG_COLOR_TYPE_GRAY;
                case Rendering::GPU::ColorType::RGB: return PNG_COLOR_TYPE_RGB;
                case Rendering::GPU::ColorType::PALETTE: return PNG_COLOR_TYPE_PALETTE;
                case Rendering::GPU::ColorType::GA: return PNG_COLOR_TYPE_GA;
                case Rendering::GPU::ColorType::RGBA: return PNG_COLOR_TYPE_RGBA;
                case Rendering::GPU::ColorType::DEPTH_STENCIL: return PNG_COLOR_TYPE_RGBA;
                default: throw std::exception();
            }
        };

        int pngColorType = getPngColorType(image.getColorType());
        png_set_IHDR(
            pngPtr,
            info_ptr,
            image.getWidth(),
            image.getHeight(),
            image.getBitDepth(),
            pngColorType,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );

        std::unique_lock<std::mutex> imageDataLock(image.getDataMutex());
        const unsigned char* dataPtr = image.getData().data();
        png_bytepp rows = static_cast<png_bytepp>(png_malloc(pngPtr, image.getHeight() * sizeof(png_bytep)));

        unsigned long bytesPerPixel = image.getBitDepth() * image.getChannelCount() / 8;
        bytesPerPixel = bytesPerPixel >= 1 ? bytesPerPixel : 1;
        const unsigned long rowSize = (image.getWidth() * bytesPerPixel);

        for (unsigned int y = 0; y < image.getSize().y; ++y) {
            auto row = static_cast<png_bytep>(png_malloc(pngPtr, rowSize));
            for (unsigned int x = 0; x < image.getSize().x; ++x) {
                memcpy(row, dataPtr + (rowSize * y), rowSize);
            }
            rows[image.getHeight() - y - 1] = row;
        }

        imageDataLock.unlock();

        png_set_rows(pngPtr, info_ptr, rows);
        png_set_write_fn(
            pngPtr,
            static_cast<png_voidp>(&ostream),
            [](png_structp png_ptr, png_bytep data, png_size_t size) -> void {
                static_cast<std::ostream*>(png_get_io_ptr(png_ptr))->write(reinterpret_cast<char*>(data), size);
            },
            nullptr
        );
        png_write_png(pngPtr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

        return ostream;
    }
}