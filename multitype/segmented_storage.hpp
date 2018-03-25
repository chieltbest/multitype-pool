//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/// this file defines the segmented storage allocator
/// the allocator allocates storage by allocating large blocks in a secondary allocator
#pragma once

#include <cxxabi.h>

#include "allocator.hpp"
#include "root/bitmap_alloc.hpp"
#include "identity_alloc.hpp"

template <unsigned N, typename T>
struct seg_alloc_segment {
	using data_t = T;

	// pointers are always pointer-aligned, so put those first
	seg_alloc_segment<N, T> *next_segment, *prev_segment;
	bitmap_alloc<N, identity_allocator<T>> alloc;
};

template <typename N, typename T>
struct segmented_allocator {

	using data_t = seg_alloc_segment<N::value, typename T::data_t>;

	constexpr static float usage = T::usage * N::value;

	template <typename Alloc, bool check_oom, bool check_free_null>
	class type : public T::template type<type<Alloc, check_oom, check_free_null>, check_oom,
	                                     check_free_null> {
		using base_t = typename T::template type<type<Alloc, check_oom, check_free_null>, check_oom,
		                                         check_free_null>;
		using t_data_t = typename T::data_t;

		data_t *front{nullptr};

	public:
		constexpr type()
		    : base_t{} {
		}

		/// free has to first find the corresponding allocator to free the data from, end then free
		/// the data from that allocator, and possibly free the segment from the underlying segment allocator
		/// finding the allocator has to be done by polling the address of the segment from the
		/// segment allocator, which might be a recursive function
		constexpr void free(t_data_t *ptr) {
			if (check_free_null && !ptr) {
				return;
			}

			data_t &section  = ((Alloc *)this)->lookup_type((const char *)ptr);
			auto &sect_alloc = section.alloc;
			//if the section is completely full it has to be re-added to the available list
			bool was_full = sect_alloc.full();
			// free the pointer from the allocator in the section
			sect_alloc.free(ptr);
			if (was_full) {
				std::cout << std::string(detail::type_name<data_t>{}) << " readd" << std::endl;
				if (!sect_alloc.empty()) {
					section.next_segment = front;
					front                = &section;
				}
			} else if (sect_alloc.empty()) {
				std::cout << std::string(detail::type_name<data_t>{}) << " free" << std::endl;
				// remove the section from the free section list
				if (front == &section) {
					front = section.next_segment;
					if (front) {
						front->prev_segment = nullptr;
					}
				} else {
					// if the section is not front then there must be a previous section
					section.prev_segment->next_segment = section.next_segment;
					if (section.next_segment) {
						section.next_segment->prev_segment = section.prev_segment;
					}
				}
				// free the section in the upper allocator
				((Alloc *)this)->free(&section);
			}
		}
		using base_t::free;

		template <typename... Args>
		constexpr t_data_t *allocate(tag<t_data_t>, Args &&... args) {
			std::cout << std::string(detail::type_name<t_data_t>{}) << " alloc >" << std::endl;
			if (!front) {
				front = ((Alloc *)this)->template allocate(tag<data_t>{});
				if (check_oom && !front) {
					return nullptr;
				}
			}
			auto ptr = front->alloc.allocate(tag<t_data_t>{}, std::forward<Args>(args)...);
			// if the the segment is filled by this allocation
			if (front->alloc.full()) {
				std::cout << std::string(detail::type_name<data_t>{}) << " overflow" << std::endl;
				// remove the segment from the available list
				front = front->next_segment;
				if (front) {
					front->prev_segment = nullptr;
				}
			}
			std::cout << std::string(detail::type_name<t_data_t>{}) << " alloc <" << std::endl;
			return ptr;
		}
		using base_t::allocate;

		t_data_t &lookup_type(const char *ptr) const {
			// use the lookup in the allocator of the segment
			return ((Alloc *)this)->lookup_type(ptr).alloc.lookup_type(ptr);
		}
	};
};
