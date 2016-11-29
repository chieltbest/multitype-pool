//
// Created by chiel on 10/11/16.
//

#ifndef MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP
#define MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP

#include <atomic>

/// a node in the
/// \tparam StorageDataPolicy
template<typename data_t, typename ptr_t>
union freelist_stack_node_impl {
public:
	data_t data;
	// free stack pointers don't have to be atomic as they are never accessed by multiple threads at once
	ptr_t next;

	operator data_t&() {
		return data;
	}

	freelist_stack_node_impl(const ptr_t ptr = nullptr)
		: next{ptr} {
	}

};

template<typename data_t>
struct freelist_stack_node {
	template<typename ptr_t>
	using node = freelist_stack_node_impl<data_t, ptr_t>;
};

/// implements the pool freelist data policy
/// \tparam ptr_t the pointer type that the pool implements
/// \tparam T the data type
/// \tparam atomic_t the atomic type that should be used
template<typename StoragePolicy, template<typename> class atomic_t = std::atomic>
class intrusive_stack {
public:
	using ptr_t = typename StoragePolicy::ptr_t;

private:
//	using atomic_t<ptr_t>::compare_exchange_weak;
//	using atomic_t<ptr_t>::load;

	StoragePolicy data;
	atomic_t<ptr_t> head;

public:
	constexpr intrusive_stack()
		: data{},
		  head{data.begin()} {
		for (ptr_t iter = data.begin(); iter != data.end() - 1; ++iter) {
			iter->next = iter + 1;
		}
		(data.end() - 1)->next = nullptr;
	}

	void push(ptr_t ptr) {
		ptr_t old_head{head.load()}; // TODO memory ordering
		do {
			(*ptr).next = old_head;
			// TODO ABA problem here, figure out how to mitigate
		} while (!head.compare_exchange_weak(old_head, ptr)); // TODO figure out memory ordering
	}

	ptr_t pop() {
		ptr_t old_head{head.load()}; // TODO memory ordering
		// if the head is nullptr then just return that, as there is no sense in getting the next element
		if (old_head != nullptr) {
			// compare exchange weak is used here, as it may seem that it is a trivial exchange, but the dereference is not
			while (!head.compare_exchange_weak(old_head, old_head->next)); // TODO memory ordering
		}
		return old_head;
	}

};

#endif //MULTITYPE_POOL_INTRUSIVE_LINKED_STACK_HPP
