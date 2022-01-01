#include "resource.hpp"

namespace Resources {
	Resource::Resource() :
            creationTime(ClockType::now()),
            lastAccessTime(ClockType::now()) {}
}
