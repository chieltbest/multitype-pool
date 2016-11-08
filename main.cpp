#include <iostream>
#include <thread>

#include "pool/pool.hpp"

static_pool::static_pool_data<char, 10> pool_data{};
static_pool::static_pool<char, 10, pool_data> pool{};
using pool_t = decltype(pool);
int main() {
	constexpr int loops = 10;
	pool_t::ptr_t ptrs[loops]{};
	for (int i = 0; i < loops; ++i) {
		ptrs[i] = pool.allocate(10+i);
	}
	for (int i = 0; i < loops; ++i) {
		std::cout << ptrs[i] << " " << +*ptrs[i] << std::endl;
	}
	for (int i = 0; i < loops; ++i) {
		pool.free(ptrs[i]);
	}

	return 0;
}
