#pragma once

#include <chrono>
#include <type_traits>
#include <string>

namespace Resources {
    struct Resource {
		typedef std::chrono::system_clock ClockType;
		typedef ClockType::time_point TimePointType;

		std::string name;
		TimePointType lastAccessTime;

		[[nodiscard]] const TimePointType& getCreationTime() const { return this->creationTime; }

    protected:
		Resource();

    private:
		TimePointType creationTime;

		Resource(const Resource&) = delete;
		Resource& operator=(const Resource&) = delete;
    };

    template<typename T, typename Enable = void>
    struct IsResource : std::false_type { };

    template<typename T>
	struct IsResource<T, typename std::enable_if<std::is_base_of<Resource, T>::value>::type> : std::true_type{};
}
