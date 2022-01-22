#pragma once

#include <type_traits>
#include <concepts>

namespace Scene::Structure {
	template <typename T>
	concept arithmetic = std::is_arithmetic_v<T>;
}