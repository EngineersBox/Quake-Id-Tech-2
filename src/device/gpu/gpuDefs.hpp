#pragma once

#include <glm/glm.hpp>

#include "../../scene/structure/rectangle.hpp"

namespace Device::GPU {
    template<typename Value, Value Default>
    struct ValueWrapper {
        typedef Value ValueType;
        typedef ValueWrapper<ValueType, Default> Type;

        ValueWrapper() = default;
        ValueWrapper(const ValueType& value) :
                value(value) {}

        operator Value() const { return this->value; }

        Type& operator=(ValueType _value) {
            this->value = _value;
            return *this;
        }

        ValueType* operator&() { return &this->value; }

    private:
        ValueType value = Default;
    };

    typedef ValueWrapper<unsigned int, 0>	GpuId;
    typedef ValueWrapper<int, -1>	GpuLocation;
    typedef ValueWrapper<unsigned int, 0>	GpuIndex;

    typedef Scene::Structure::Rectangle<float> GpuViewportType;
    typedef unsigned char GpuClearFlagType;
    typedef glm::vec2 GpuFrameBufferSizeType;
    typedef unsigned char GpuFrameBufferTypeFlagsType;

    enum : GpuFrameBufferTypeFlagsType {
        GPU_FRAME_BUFFER_TYPE_FLAG_COLOR = (1 << 0),
        GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH = (1 << 1),
        GPU_FRAME_BUFFER_TYPE_FLAG_STENCIL = (1 << 2)
    };

    enum class GpuFrameBufferType : GpuFrameBufferTypeFlagsType {
        COLOR = (GPU_FRAME_BUFFER_TYPE_FLAG_COLOR),
        COLOR_DEPTH = (GPU_FRAME_BUFFER_TYPE_FLAG_COLOR | GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH),
        COLOR_DEPTH_STENCIL = (GPU_FRAME_BUFFER_TYPE_FLAG_COLOR | GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH | GPU_FRAME_BUFFER_TYPE_FLAG_STENCIL),
        DEPTH = (GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH),
        DEPTH_STENCIL = (GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH | GPU_FRAME_BUFFER_TYPE_FLAG_STENCIL)
    };

    enum class GpuDataTypes {
        BYTE,
        UNSIGNED_BYTE,
        SHORT,
        UNSIGNED_SHORT,
        INT,
        UNSIGNED_INT,
        FLOAT,
        DOUBLE
    };

    template<typename T>
    struct GpuDataType;

    template<>
    struct GpuDataType<char> {
        static const auto VALUE = GpuDataTypes::BYTE;
    };

    template<>
    struct GpuDataType<unsigned char> {
        static const auto VALUE = GpuDataTypes::UNSIGNED_BYTE;
    };

    template<>
    struct GpuDataType<short> {
        static const auto VALUE = GpuDataTypes::SHORT;
    };

    template<>
    struct GpuDataType<unsigned short> {
        static const auto VALUE = GpuDataTypes::UNSIGNED_SHORT;
    };

    template<>
    struct GpuDataType<int> {
        static const auto VALUE = GpuDataTypes::INT;
    };

    template<>
    struct GpuDataType<unsigned int> {
        static const auto VALUE = GpuDataTypes::UNSIGNED_INT;
    };

    template<>
    struct GpuDataType<float> {
        static const auto VALUE = GpuDataTypes::FLOAT;
    };

    template<>
    struct GpuDataType<double> {
        static const auto VALUE = GpuDataTypes::DOUBLE;
    };
}