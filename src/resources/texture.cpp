#include "texture.hpp"
#include "image.hpp"
#include "../device/gpu/gpu.hpp"

#include <boost/make_shared.hpp>

namespace Resources {
    Texture::Texture(Device::GPU::ColorType color_type, const glm::vec2& size, const void* data) :
            colorType(color_type),
            size(size) {
        id = Device::GPU::gpu.createTexture(color_type, static_cast<glm::uvec2>(size), data);
    }

    Texture::Texture(const boost::shared_ptr<Image>& image) :
            Texture(
                image->getColorType(),
                image->getSize(),
                image->getData().data()
            ) {}

    Texture::Texture(std::istream& istream) :
            Texture(boost::make_shared<Image>(istream)) {}

    Texture::~Texture() { Device::GPU::gpu.destroyTexture(id); }

    void Texture::set_size(const glm::vec2& _size) {
        if (_size == get_size()) return;
        Device::GPU::gpu.resizeTexture(shared_from_this(), static_cast<glm::uvec2>(_size));
        this->size = _size;
    }
}