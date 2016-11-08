//
// Created by chiel on 26/10/16.
//

#ifndef MULTITYPE_POOL_POOL_HPP
#define MULTITYPE_POOL_POOL_HPP

#include <memory>
#include <cassert>
#include "../autosize/autosize_int.hpp"

namespace static_pool {

	template<typename T, unsigned MAX>
	class static_pool_data;

	namespace detail {

		template<unsigned MAX>
		using index = auto_uint<MAX>;
		template<typename T, unsigned MAX>
		union data_ptr_union;
		template<typename T, unsigned MAX>
		class intrusive_stack_node;

		template<typename T, unsigned MAX>
		class internal_ptr_t {
		protected:
			index<MAX> idx;

		public:
			constexpr internal_ptr_t(const index<MAX> idx = ~static_cast<index<MAX>>(0))
				: idx{idx} {
			}

			/// nullptr to internal_ptr conversion
			constexpr internal_ptr_t(std::nullptr_t null)
					// nullptr is one past the maximum index stored by index<MAX>
					: internal_ptr_t{} {
			}

			template<static_pool_data<T, MAX> &data>
			T& get_data() const {
				return data.data[idx].data;
			}

			template<static_pool_data<T, MAX> &data>
			intrusive_stack_node<T, MAX>& get_freelist() const {
				return data.data[idx].free_list;
			};

			template<typename ostream>
			friend auto& operator<<(ostream &rhs, const internal_ptr_t &lhs) {
				return rhs << +lhs.idx;
			}

		};

		template<typename T, unsigned MAX>
		class intrusive_stack_node {
		public:
			internal_ptr_t<T, MAX> next;

			constexpr intrusive_stack_node(internal_ptr_t<T, MAX> next = nullptr)				: next{next} {
			}
		};

		template<typename T, unsigned MAX>
		class intrusive_stack : intrusive_stack_node<T, MAX> {
		public:
			constexpr intrusive_stack(index<MAX> idx = nullptr)
					: intrusive_stack_node<T, MAX>{idx} {
			}

			template<static_pool_data<T, MAX> &data>
			void push(internal_ptr_t<T, MAX> node) {
				node.template get_freelist<data>() = *this;
				this->next = node;
				// TODO make atomic
			}

			template<static_pool_data<T, MAX> &data>
			internal_ptr_t<T, MAX> pop() {
				const internal_ptr_t<T, MAX> temp{this->next};
				this->next = this->next.template get_freelist<data>().next;
				return temp;
				// TODO make atomic
			}
		};

		template<typename T, unsigned MAX>
		union data_ptr_union {
			// we have to explicitly initialise the free list, to set the active data member
			constexpr data_ptr_union() : free_list{} {
			}

			T data;
			intrusive_stack_node<T, MAX> free_list;
		};

	}

	template<typename T, unsigned MAX>
	class static_pool_data {
	public:
		constexpr static_pool_data()
				: free_elems{0},
				  data{} {
			// pre-initialise the free list, as the static pool cannot access the data here in constexpr time
			for (detail::index<MAX> i = 0; i < MAX - 1; ++i) {
				data[i].free_list = {static_cast<detail::index<MAX>>(i + 1)};
			}
			data[MAX - 1].free_list = {nullptr};
		}

		// free_elems is defined first, so that there is less chance of a cache miss/slow lookup
		detail::intrusive_stack<T, MAX> free_elems;
		detail::data_ptr_union<T, MAX> data[MAX];
	};

	template<typename T,
			unsigned MAX,
			static_pool_data<T, MAX> &data>
	class static_pool {
	public:


		class ptr_t : public detail::internal_ptr_t<T, MAX> {
		public:
			constexpr ptr_t(detail::internal_ptr_t<T, MAX> ptr = {nullptr})
				: detail::internal_ptr_t<T, MAX>{ptr} {
			}

			constexpr ptr_t(std::nullptr_t ptr)
					: ptr_t{} {
			}

			constexpr T* operator->() const {
				return this->operator*();
			}

			constexpr T& operator*() const {
				return detail::internal_ptr_t<T, MAX>::template get_data<data>();
			}

			constexpr bool operator==(const ptr_t &lhs) const {
				return this->idx == lhs.idx;
			}

			constexpr bool operator!=(const ptr_t &lhs) const {
				return this->idx != lhs.idx;
			}
		};

		template<typename ...Args>
		ptr_t allocate(Args... args) {
			ptr_t new_elem{data.free_elems.template pop<data>()};
			assert(new_elem != nullptr);

			new (&(*new_elem)) T(args...);
			return new_elem;
		}

		void free(ptr_t ptr) {
			assert(ptr != nullptr);

			(*ptr).~T();
			data.free_elems.template push<data>(ptr);
		}

	};

}

#endif //MULTITYPE_POOL_POOL_HPP
