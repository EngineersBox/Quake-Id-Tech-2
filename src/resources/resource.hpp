#pragma once

#include <chrono>
#include <type_traits>
#include <string>
#include <concepts>

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

	template<typename T>
	concept IsResource = std::derived_from<T, Resource>;
}
