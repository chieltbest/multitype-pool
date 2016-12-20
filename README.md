# multitype-pool

Pool implementation that is able to support multiple fixed types,
 and will be more efficient because of this information.

This pool optionally uses a statically allocated data store,
 so that the custom pointer type can use an automagically-sized index for referring to the object, thus saving memory

## examples

Create a single type pool with static memory allocation:
```cpp
#include <iostream>
#include "static_pool.hpp"

// the stored type
struct foo {
	char bar[42];
}

// create a global pool data variable to store the elements in
// the template variables are the stored type, and the maximum number of allocations of that type
static_pool::static_pool_data<foo, 16> pool_data{};
// create the actual pool object, with the reference to the data
static_pool::static_pool<foo, 16, pool_data> pool{};

int main() {
	// allocate a single element, use auto for convinience (using the custom pool pointer type)
	auto ptr = pool.allocate();
	// print out the index of the pointer, and the pointer contents
	std::cout << ptr << ": " << *ptr << std::endl;
	pool.free(ptr);
}
```