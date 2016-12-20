# multitype-pool

Pool implementation that is able to support multiple fixed types,
 and will be more efficient because of this information.

This pool optionally uses a statically allocated data store,
 so that the custom pointer type can use an automagically-sized index for referring to the object, thus saving memory

## examples

Create a single type pool with static memory allocation:
```cpp
#include <iostream>

#include "pool/policy/static_storage.hpp"
#include "pool/policy/intrusive_linked_stack.hpp"
#include "pool/policy/freelist_data_store.hpp"
#include "pool/pool.hpp"

// the stored type
struct foo {
	char bar[42];
}

constexpr int pool_size = 10;

// create a global pool data variable to store the elements in
// the template variables are the stored type, and the maximum number of allocations of that type
// we are using the freelist node from intrusive_linked_stack
using freelist_t = static_storage_array_data<
	freelist_stack_node< // freelist free block tracking
		foo // the data object
	>::node, // using ::node, as it is a partial applicaton that requires a pointer type 
	pool_size>; // the size of the storage
freelist_t data_storage{};

// create the actual pool object, with the reference to the data
 
inline void oom_exit() {
	std::cerr << "Out of memory!" << std::endl;
	std::exit(1);
}
 
using pool_t = pool<
	freelist_data_store< // freelist data adapter
		atomic_freelist_stack< // the actual stack to use (includes the stack head)
			static_storage< // type adapter for the data storage object
				freelist_t, // the type of the storage, all information about the data gets 
				            // pulled from this type
				data_storage>>>, // a reference to the actual data storage object
	oom_exit>; // the function that gets called when the pool is out of objects
pool_t pool{}; // the actual pool object

int main() {
	// allocate a single element, use auto for convinience (using the custom pool pointer type)
	auto ptr = pool.allocate();
	// the pointer type acts like a normal pointer, so you can print it and do pointer arithmetic 
	std::cout << ptr << ": " << *ptr << std::endl;
	// the pointer is only one byte
	std::cout << sizeof(ptr) << std::endl;
	pool.free(ptr); // free the pointer again
	
	// allocate can also take a custom type if the pool supports it
	// the default type is taken from the data storage type
	ptr = pool.allocate<foo>();
	pool.free(ptr);
}
```