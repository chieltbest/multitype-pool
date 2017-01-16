#include <iostream>
#include <thread>

#include "pool/policy/static_storage.hpp"
#include "pool/policy/freelist_bitmap.hpp"
#include "pool/policy/freelist_stack.hpp"
#include "pool/policy/freelist_data_store.hpp"
#include "pool/pool.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

constexpr int loops = 1'000'000'000, poolsize = 10;

inline void oom_exit();

/*using freelist_t = static_storage_array_data<
	freelist_stack_node<int>::node,
	poolsize>;
freelist_t data_storage{};

using pool_t = pool<
	freelist_data_store<
		atomic_freelist_stack<
			static_storage<
				freelist_t,
				data_storage>>>,
	oom_exit>;
pool_t pool_obj{};

inline void oom_exit() {
	std::cerr << "Out of memory!" << std::endl;
	for (int i = 0; i < data_storage.max_elems; ++i) {
		std::cout << data_storage[i] << std::endl;
	}
	std::exit(1);
}

int main() {
	decltype(pool_obj.get_free()) free;
	do {
		auto allocator_thread = []() {
			for (int i = 0; i < loops; ++i) {
				auto ptr = pool_obj.allocate();
//				std::cout << ptr << " " << *ptr << std::endl;
				pool_obj.free(ptr);
			}
		};
		auto start_time = system_clock::now();

		std::thread threads[4];
		for (auto&& thread : threads) {
			thread = std::thread{allocator_thread};
		}
		for (auto&& thread : threads) {
			thread.join();
		}

		auto total_time = std::chrono::system_clock::now() - start_time;

		free = pool_obj.get_free();
		std::cout << free
		          << " (took " << duration_cast<milliseconds>(total_time).count() << "ms)"
		          << std::endl;
	} while (free == poolsize);

	return 0;
}*/

template<typename ptr_t>
struct foo {};

using freelist_t = static_storage_array_data<
	foo,
	poolsize>;
freelist_t data_storage{};

using pool_t = pool<
	freelist_data_store<
		atomic_freelist_bitmap<
			static_storage<
				freelist_t,
				data_storage>>>
	/*oom_exit*/>;
pool_t pool_obj{};

inline void oom_exit() {
	std::cerr << "Out of memory!" << std::endl;
	for (int i = 0; i < data_storage.max_elems; ++i) {
//		std::cout << data_storage[i] << std::endl;
	}
	std::exit(1);
}

int main() {
	decltype(pool_obj.get_free()) free = poolsize;
	do {
		auto start_time = system_clock::now();

		for (int i = 0; i < loops; ++i) {
			auto ptr = pool_obj.allocate();
//			std::cout << ptr << " " << *ptr << std::endl;
			pool_obj.free(ptr);
		}

		auto total_time = std::chrono::system_clock::now() - start_time;

//		free = pool_obj.get_free();
		std::cout /*<< free*/
		          << " (took " << duration_cast<milliseconds>(total_time).count() << "ms)"
		          << std::endl;
	} while (free == poolsize);

	return 0;
}