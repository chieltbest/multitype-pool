//
// Created by chiel on 10/11/16.
//

#ifndef MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP
#define MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP

#include <atomic>
#include <iostream>
#include <unordered_set>

#include "../../autosize/autosize.hpp"
#include "backoff_exponential.hpp"

// use an anonymous namespace to hide the implementation from other translation
// units
namespace {
	/// a node in the
	/// \tparam StorageDataPolicy
	template <template <typename> class data_t, typename ptr_t>
	union freelist_stack_node_impl {
	public:
		data_t<ptr_t> data;
		// free stack pointers don't have to be atomic as they are
		// never accessed by multiple threads at once
		ptr_t next;

		operator data_t<ptr_t> &() {
			return data;
		}

		constexpr freelist_stack_node_impl(const ptr_t ptr = nullptr)
		    : next{ptr} {
		}

		/// initializes the data in the data storage
		/// \tparam size the number of elements to be initialized
		/// \tparam container the type of the container that the data is stored in
		/// \param data reference to the data that should be initialized
		template <unsigned size, typename container>
		constexpr static void init(container &data) {
			for (int i = 0; i != size - 1; ++i) { // TODO change this to a range
				data[i].next = data.at(i + 1);
			}
			// the pointers are default constructed as nullptr, so the last element
			// doesn't have
			// to be assigned
		};
	};
} // namespace

template <template <typename> class data_t>
struct freelist_stack_node {
	template <typename ptr_t>
	using node = freelist_stack_node_impl<data_t, ptr_t>;
};

/// implements the pool freelist data policy
/// \tparam StoragePolicy the storage class
/// \tparam Atomic the type of atomic that the stack should use
/// \tparam Backoff the backoff policy to be used when the atomic fails
///                 a backoff object is made at every function call, which then
///                 gets it's
///                 operator() called for every failed attempt
/// \tparam AllocCounter the type of the counter int that the atomic head should incorporate.
///                      the counter should be at least 16 bits, as with less
///                      bits the chance of
///                      an aba occurring will increase.
template <typename StoragePolicy, template <typename> class Atomic = std::atomic,
          typename Backoff      = backoff_exponential<1024, 16384>, // TODO test out some values
          typename AllocCounter = uint32_t>
// the counter type must be unsigned, as
// overflow is only defined for unsigned types
class atomic_freelist_stack {
public:
	using data_t = decltype(StoragePolicy::data_t::data);
	using ptr_t  = typename StoragePolicy::ptr_t;

private:
	StoragePolicy data;

	struct head_t {
		AllocCounter count;
		ptr_t ptr;
	};

	union head_t_int_union {
		head_t data;
		autosize::int_sizeof<sizeof(head_t)> num;
	};

	Atomic<autosize::int_sizeof<sizeof(head_t)>> head;

public:
	constexpr atomic_freelist_stack()
	    : data{}
	    , head{head_t_int_union{.data = {.ptr = data.begin(), .count = 0}}.num} {
		static_assert(sizeof(head_t_int_union) <= sizeof(long), "head is probably not atomic");
	}

	void push(ptr_t ptr) {
		head_t_int_union old_head = head_t_int_union{.num = head.load(std::memory_order_acquire)};
		bool result;
		Backoff backoff{};
		do {
			ptr->next = old_head.data.ptr;

			result = head.compare_exchange_weak(
			        old_head.num,
			        head_t_int_union{
			                .data = {.ptr = ptr, .count = AllocCounter(old_head.data.count + 1)}}
			                .num,
			        std::memory_order_release, std::memory_order_relaxed);
			if (!result) {
				backoff();
			}
		} while (!result);
	}

	template <bool HandleOutOfMemory = true>
	ptr_t pop() {
		head_t_int_union old_head = head_t_int_union{.num = head.load(std::memory_order_acquire)};
		bool result;
		Backoff backoff;
		do {
			// if the head is not valid then just return it,
			// as there is no sense in getting the next element
			if (HandleOutOfMemory && !old_head.data.ptr) {
				break;
			}

			result = head.compare_exchange_weak(
			        old_head.num,
			        head_t_int_union{.data = {.ptr   = old_head.data.ptr->next,
			                                  .count = AllocCounter(old_head.data.count + 1)}}
			                .num,
			        std::memory_order_acquire, std::memory_order_relaxed);
			if (!result) {
				backoff();
			}
		} while (!result);

		return old_head.data.ptr;
	}

	template <typename Ostream>
	auto get_free(Ostream &err = std::cerr) {
		std::unordered_set<ptr_t, typename ptr_t::hash> seen;
		std::size_t free = 0;
		ptr_t cur_ptr = head_t_int_union{.num = head.load()}.data.ptr;
		while (cur_ptr != nullptr) {
			auto found = seen.insert(cur_ptr);
			if (!std::get<1>(found)) {
				// there is a loop in the stack (that's bad)
				err << "Stack corruption detected!" << std::endl;
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
template <typename StoragePolicy>
class freelist_stack {
public:
	using data_t = decltype(StoragePolicy::data_t::data);
	using ptr_t  = typename StoragePolicy::ptr_t;

private:
	StoragePolicy data;
	ptr_t head;

public:
	constexpr freelist_stack()
	    : data{}
	    , head{data.begin()} {
	}

	void push(ptr_t ptr) {
		ptr->next = head;
		head      = ptr;
	}

	template <bool HandleOutOfMemory = true>
	ptr_t pop() {
		ptr_t old_head = head;
		if (!HandleOutOfMemory || old_head) {
			head = std::move(old_head->next);
		}
		return old_head;
	}

	auto get_free() {
		std::unordered_set<ptr_t, typename ptr_t::hash> seen;
		std::size_t free = 0;
		ptr_t cur_ptr    = head;
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

// TODO make convenience wrapper

#endif // MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP
