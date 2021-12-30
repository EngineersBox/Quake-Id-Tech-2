#include "image.hpp"

#include <png.h>
#include <iostream>

namespace Resources {
    Image::Image(std::istream& istream) {
        //TODO: determine what the stream actually contains (don't assume PNG!)
        png_struct_def* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png_ptr == nullptr) throw std::exception();

        png_info_def* info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == nullptr) {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            throw std::exception();
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            throw std::exception();
        }

        png_set_read_fn(png_ptr, static_cast<png_voidp>(&istream), [](png_structp png_ptr, png_bytep data, png_size_t length) {
            void* png_io_ptr = png_get_io_ptr(png_ptr);
            static_cast<std::istream*>(png_io_ptr)->read(reinterpret_cast<char*>(data), length);
        });

        png_uint_32 sig_read = 0;
        png_set_sig_bytes(png_ptr, sig_read);
        png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, nullptr);

        int png_color_type;
        png_int_32 interlace_method;
        png_uint_32 width, height;

        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &png_color_type, &interlace_method, nullptr, nullptr);

        size.x = static_cast<float>(width);
        size.y = static_cast<float>(height);

        auto get_color_type = [](int png_color_type) -> Rendering::GPU::ColorType {
            switch (png_color_type) {
                case PNG_COLOR_TYPE_GRAY: return Rendering::GPU::ColorType::G;
                case PNG_COLOR_TYPE_RGB: return Rendering::GPU::ColorType::RGB;
                case PNG_COLOR_TYPE_PALETTE: return Rendering::GPU::ColorType::PALETTE;
                case PNG_COLOR_TYPE_GA: return Rendering::GPU::ColorType::GA;
                case PNG_COLOR_TYPE_RGBA: return Rendering::GPU::ColorType::RGBA;
                default: throw std::exception();
            }
        };

        color_type = get_color_type(png_color_type);
        unsigned long row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        pixel_stride = row_bytes / get_width();
        unsigned long data_length = row_bytes * get_height();
        data.resize(data_length);
        unsigned char** row_pointers = png_get_rows(png_ptr, info_ptr);

        for (unsigned int i = 0; i < size.y; ++i) {
            memcpy(data.data() + (row_bytes * (get_height() - 1 - i)), row_pointers[i], row_bytes);
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    }

    Image::Image(const SizeType& size, BitDepthType bit_depth, Rendering::GPU::ColorType color_type, const unsigned char* data_ptr, size_t data_size) :
            size(size),
            bit_depth(bit_depth),
            color_type(color_type) {
        data.resize(data_size);
        if (data_size <= data.size())
            memcpy(data.data(), data_ptr, data_size);
    }


    size_t Image::get_channel_count() const {
        switch (color_type) {
            case Rendering::GPU::ColorType::G:
            case Rendering::GPU::ColorType::PALETTE:
                return 1;
            case Rendering::GPU::ColorType::GA: return 2;
            case Rendering::GPU::ColorType::RGB: return 3;
            case Rendering::GPU::ColorType::RGBA: return 4;
            default: return 0;
        }
    }

    std::string png_error_message;
    std::ostream& operator<<(std::ostream& ostream, Image& image) {
        png_struct_def* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr) {
            throw std::exception();
        }

        png_info_def* info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, nullptr);
            throw std::exception();
        }

        std::exception exception;
        png_error_ptr error_fn = [](png_structp png_ptr, png_const_charp message) {
            png_error_message = message;
        };

        png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), error_fn, NULL);

        if (setjmp(png_jmpbuf(png_ptr))) {
            void* error_ptr = png_get_error_ptr(png_ptr);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            throw std::runtime_error(png_error_message.c_str());
        }

        auto get_png_color_type = [](Rendering::GPU::ColorType color_type) -> int {
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

        int png_color_type = get_png_color_type(image.get_color_type());
        png_set_IHDR(
            png_ptr,
            info_ptr,
            image.get_width(),
            image.get_height(),
            image.get_bit_depth(),
            png_color_type,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );

        std::unique_lock<std::mutex> image_data_lock(image.get_data_mutex());
        const unsigned char* data_ptr = image.get_data().data();
        png_bytepp rows = static_cast<png_bytepp>(png_malloc(png_ptr, image.get_height() * sizeof(png_bytep)));

        unsigned long bytes_per_pixel = image.get_bit_depth() * image.get_channel_count() / 8;
        bytes_per_pixel = bytes_per_pixel >= 1 ? bytes_per_pixel : 1;
        const unsigned long row_size = (image.get_width() * bytes_per_pixel);

        for (unsigned int y = 0; y < image.get_size().y; ++y) {
            auto row = static_cast<png_bytep>(png_malloc(png_ptr, row_size));
            for (unsigned int x = 0; x < image.get_size().x; ++x) {
                memcpy(row, data_ptr + (row_size * y), row_size);
            }
            rows[image.get_height() - y - 1] = row;
        }

        image_data_lock.unlock();

        png_set_rows(png_ptr, info_ptr, rows);
        png_set_write_fn(
            png_ptr,
            static_cast<png_voidp>(&ostream),
            [](png_structp png_ptr, png_bytep data, png_size_t size) -> void {
                static_cast<std::ostream*>(png_get_io_ptr(png_ptr))->write(reinterpret_cast<char*>(data), size);
            },
            nullptr
        );
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

        return ostream;
    }
}