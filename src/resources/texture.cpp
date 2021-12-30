#include "texture.hpp"
#include "image.hpp"
#include "../rendering/gpu/gpu.hpp"

#include <boost/make_shared.hpp>

namespace Resources {
    Texture::Texture(Rendering::GPU::ColorType color_type, const glm::vec2& size, const void* data) :
            color_type(color_type),
            size(size) {
        id = Rendering::GPU::gpu.create_texture(color_type, static_cast<glm::uvec2>(size), data);
    }

    Texture::Texture(const boost::shared_ptr<Image>& image) :
            Texture(image->get_color_type(),
                    image->get_size(),
                    image->get_data().data()) {}

    Texture::Texture(std::istream& istream) :
            Texture(boost::make_shared<Image>(istream)) {}

    Texture::~Texture() { Rendering::GPU::gpu.destroy_texture(id); }

    void Texture::set_size(const glm::vec2& size) {
        if (size == get_size()) return;
        Rendering::GPU::gpu.resize_texture(shared_from_this(), static_cast<glm::uvec2>(size));
        this->size = size;
    }
}