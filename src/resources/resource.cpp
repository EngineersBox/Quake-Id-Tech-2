#include "resource.hpp"

namespace Resources {
	Resource::Resource() :
        creation_time(ClockType::now()),
		last_access_time(ClockType::now()) {}
}
