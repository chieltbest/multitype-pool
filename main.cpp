#include <iostream>

#include "pool/pool.hpp"
#include "pool/policy/intrusive_linked_stack.hpp"
#include "pool/policy/static_storage.hpp"
#include "pool/policy/freelist_data_store.hpp"

void oomComm() {
	std::cerr << "Out of memory!" << std::endl;
};

using freelist_t = static_storage_array_data<freelist_stack_node<char>::node, 10>;
freelist_t data_storage{};
using pool_t = pool<freelist_data_store<intrusive_stack<static_storage<freelist_t, data_storage>>>, char, oomComm>;
pool_t pool_obj{};

int main() {
//	auto ptr = pool_obj.allocate('a');
//	std::cout << ptr << " " << +*ptr << std::endl;
	constexpr int loops = 11;
	pool_t::ptr_t<char> ptrs[loops]{};
	for (int i = 0; i < loops; ++i) {
		ptrs[i] = pool_obj.allocate(10+i);
		std::cout << ptrs[i] << " " << +*ptrs[i] << std::endl;
	}
	for (int i = 0; i < loops; ++i) {
		pool_obj.free(ptrs[i]);
	}

	return 0;
}
