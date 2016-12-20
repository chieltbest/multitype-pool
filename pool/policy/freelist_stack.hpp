//
// Created by chiel on 10/11/16.
//

#ifndef MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP
#define MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP

#include <atomic>
#include <iostream>
#include <unordered_set>
#include <cassert>

#include "../../autosize/autosize.hpp"

// use an anonymous namespace to hide the implementation from other translation units
namespace {
	/// a node in the
	/// \tparam StorageDataPolicy
	template<typename data_t, typename ptr_t>
	union freelist_stack_node_impl {
	public:
		data_t data;
		// free stack pointers don't have to be atomic as they are
		// never accessed by multiple threads at once
		ptr_t next;

		operator data_t &() {
			return data;
		}

		constexpr freelist_stack_node_impl(const ptr_t ptr = nullptr)
			: next{ptr} {
		}

		/// initializes the data in the data storage
		/// \tparam size the number of elements to be initialized
		/// \tparam container the type of the container that the data is stored in
		/// \param data reference to the data that should be initialized
		template<unsigned size, typename container>
		constexpr static void init(container& data) {
			for (int i = 0; i != size - 1; ++i) { // TODO change this to a range
				data[i].next = data.at(i + 1);
			}
			// the pointers are default constructed as nullptr, so the last element doesn't have
			// to be assigned
		};

	};
}

template<typename data_t>
struct freelist_stack_node {
	template<typename ptr_t> using node = freelist_stack_node_impl<data_t, ptr_t>;
};

/// implements the pool freelist data policy
/// \tparam StoragePolicy the storage class
/// \tparam atomic_t the type of atomic that the stack should use
/// \tparam alloc_counter the type of the counter int that the atomic head should incorporate.
///                       the counter should be at least 16 bits, as with less bits the chance of
///                       an aba occurring will increase.
template <typename StoragePolicy, template <typename> class atomic_t = std::atomic,
          typename alloc_counter = uint32_t>
// the counter type must be unsigned, as
// overflow is only defined for unsigned types
class atomic_freelist_stack {
public:
	using data_t = decltype(StoragePolicy::data_t::data);
	using ptr_t = typename StoragePolicy::ptr_t;

private:
	StoragePolicy data;

	struct head_t {
		alloc_counter count;
		ptr_t ptr;
	};

	union head_t_int_union {
		head_t data;
		autosize::int_sizeof<sizeof(head_t)> num;
	};

	atomic_t<autosize::int_sizeof<sizeof(head_t)>> head;

public:
	constexpr atomic_freelist_stack()
		: data{},
		  head{head_t_int_union{
			  .data = {
				  .ptr = data.begin(),
				  .count = 0}
		  }.num} {
		static_assert(sizeof(head_t_int_union) <= sizeof(long), "head is probably not atomic");
	}

	void push(ptr_t ptr) {
		head_t_int_union old_head = head_t_int_union{.num = head.load(std::memory_order_acquire)};
		bool result;
		do {
			ptr->next = old_head.data.ptr;

			result = head.compare_exchange_weak(old_head.num,
			                                    head_t_int_union{
				                                    .data = {.ptr = ptr,
					                                         .count = alloc_counter(
						                                         old_head.data.count + 1)}
			                                    }.num,
			                                    std::memory_order_release,
			                                    std::memory_order_relaxed);
			if (!result) {
				// TODO make a better back off
				std::this_thread::yield();
			}
		} while (!result);
	}

	ptr_t pop() {
		head_t_int_union old_head = head_t_int_union{.num = head.load(std::memory_order_acquire)};
		bool result;
		// if the head is not valid then just return it,
		// as there is no sense in getting the next element
		do {
			if (!old_head.data.ptr) {
				break;
			}

			result = head.compare_exchange_weak(old_head.num,
			                                    head_t_int_union{
				                                    .data = {.ptr = old_head.data.ptr->next,
				                                             .count = alloc_counter(
					                                             old_head.data.count + 1)}
			                                    }.num,
			                                    std::memory_order_acquire,
			                                    std::memory_order_relaxed);
			if (!result) {
				// TODO make a better back off
				std::this_thread::yield();
			}
		} while (!result);

		return old_head.data.ptr;
	}

	auto get_free() {
		std::unordered_set<ptr_t, typename ptr_t::hash> seen;
		std::size_t free = 0;
		ptr_t cur_ptr = head_t_int_union{.num = head.load()}.data.ptr;
		while (cur_ptr != nullptr) {
			auto found = seen.insert(cur_ptr);
			if (!std::get<1>(found)) {
				// there is a loop in the stack (that's bad)
				std::cerr << "Stack corruption detected!" << std::endl;
				return free;
			}
			free++;
			cur_ptr = cur_ptr->next;
		}
		return free;
	}

};

/// non-atomic stack, to be used when it is sure that there is
/// only ever one thread accessing the pool object
template<typename StoragePolicy>
class freelist_stack {
public:
	using data_t = decltype(StoragePolicy::data_t::data);
	using ptr_t = typename StoragePolicy::ptr_t;

private:
	StoragePolicy data;
	ptr_t head;

public:
	constexpr freelist_stack()
		: data{},
		  head{data.begin()} {
	}

	void push(ptr_t ptr) {
		ptr->next = head;
		head = ptr;
	}

	ptr_t pop() {
		ptr_t old_head = head;
		if (old_head) {
			head = std::move(old_head->next);
		}
		return old_head;
	}

	auto get_free() {
		std::unordered_set<ptr_t, typename ptr_t::hash> seen;
		std::size_t free = 0;
		ptr_t cur_ptr = head;
		while (cur_ptr != nullptr) {
			auto found = seen.find(cur_ptr);
			if (found != seen.end()) {
				// there is a loop in the stack (that's bad)
				std::cerr << "Stack corruption detected!" << std::endl;
				return free;
			}
			free++;
			cur_ptr = cur_ptr->next;
		}
		return free;
	}

};

#endif // MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP
