//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/// this file defines the segmented storage allocator
/// the allocator allocates storage by allocating large blocks in a secondary allocator
#pragma once

#include <cxxabi.h>

#include "bitmap_alloc.hpp"
#include "../pool/policy/dynamic_storage.hpp"

namespace detail {
	template <typename T>
	struct type_name {
		operator std::string() const {
			const char *name{
			        __cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr)};
			std::string res{name}; // copy from name into string
			delete name;
			return res;
		}
	};
}
// end debug functions

template <typename T, unsigned N>
class seg_alloc_segment {
public:
	using data_t = T;

	seg_alloc_segment<T, N> *next_segment, *prev_segment;
	bitmap_alloc<T, N> alloc{};
};
template<typename SectionType>
struct segmented_allocator {
	template<typename AllocType>
	class type {
		AllocType &alloc;
		using section_t = SectionType;
		section_t *front{nullptr};

	public:
		using data_t = typename SectionType::data_t;

		constexpr type(AllocType &alloc)
			: alloc{alloc} {
		}

		/// free has to first find the corresponding allocator to free the data from, end then free
		/// the data from that allocator, and possibly free the segment from the underlying segment allocator
		/// finding the allocator has to be done by polling the address of the segment from the
		/// segment allocator, which might be a recursive function
		constexpr void free(data_t *ptr) {
			section_t &section = alloc.lookup_type((char *) ptr);
			auto &sect_alloc = section.alloc;
			//if the section is completely full it has to be re-added to the available list
			bool was_full = sect_alloc.full();
			// free the pointer from the allocator in the section
			sect_alloc.free(ptr);
			if (was_full) {
				std::cout << std::string(detail::type_name<section_t>{}) << " readd" << std::endl;
				if (!sect_alloc.empty()) {
					section.next_segment = front;
					front = &section;
				}
			} else if (sect_alloc.empty()) {
				std::cout << std::string(detail::type_name<section_t>{}) << " free" << std::endl;
				// remove the section from the free section list
				if (section.next_segment) {
					section.next_segment->prev_segment = section.prev_segment;
				}
				if (section.prev_segment) {
					section.prev_segment->next_segment = section.next_segment;
				}
				// free the section in the upper allocator
				alloc.free(&section);
			}
		}

		template<typename T, typename... Args>
		constexpr std::enable_if_t<std::is_same<T, data_t>::value, data_t>::type *allocate(
			Args &&... args) {
			if (!front) {
				std::cout << std::string(detail::type_name<section_t>{}) << " alloc" << std::endl;
				front = alloc.allocate<section_t>();
			}
			auto ptr = front->alloc.allocate(std::forward<Args>(args)...);
			// if the the segment is filled by this allocation
			if (front->alloc.full()) {
				std::cout << std::string(detail::type_name<section_t>{}) << " overflow"
				          << std::endl;
				// remove the segment from the available list
				front = front->next_segment;
				if (front) {
					front->prev_segment = nullptr;
				}
			}
			return ptr;
		}

		data_t &lookup_type(const char *ptr) {
			// use the lookup in the allocator of the segment
			return alloc.lookup_type(ptr).alloc.lookup_type(ptr);
		}
	};
};
