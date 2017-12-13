//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <type_traits>

namespace detail {
	template <typename... Ts>
	union var_union {};

	template <typename T, typename... Ts>
	union var_union<T, Ts...> {
		T car;
		var_union<Ts...> cdr;
	};

	template <typename Get, typename... Ts>
	constexpr Get &get_union(var_union<Get, Ts...> &u) {
		return u.car;
	};

	template <typename Get, typename T, typename... Ts>
	constexpr auto get_union(var_union<T, Ts...> &u) -> decltype(get_union<Get>(u.cdr)) {
		return get_union<Get>(u.cdr);
	};

	// if the element cannot be found then SFINAE

	template <typename Alloc, typename data_t, bool check_oom, bool check_null_free,
	          typename... Types>
	class type_impl
	        : public identity_allocator<data_t>::template type<Alloc, check_oom, check_null_free> {
	};

	template <typename Alloc, typename data_t, bool check_oom, bool check_null_free,
	          typename SubAlloc, typename... Types>
	class type_impl<Alloc, data_t, check_oom, check_null_free, SubAlloc, Types...>
	        : public type_impl<Alloc, data_t, check_oom, check_null_free, Types...>,
	          public SubAlloc::template type<
	                  type_impl<Alloc, data_t, check_oom, check_null_free, SubAlloc, Types...>,
	                  check_oom, check_null_free> {
		using next_base_t = type_impl<Alloc, data_t, check_oom, check_null_free, Types...>;
		using base_t      = typename SubAlloc::template type<
		        type_impl<Alloc, data_t, check_oom, check_null_free, SubAlloc, Types...>, check_oom,
		        check_null_free>;

		Alloc &alloc;

	public:
		constexpr type_impl(Alloc &alloc)
		    : next_base_t{alloc}
		    , base_t{*this}
		    , alloc{alloc} {
		}

		constexpr void free(typename SubAlloc::data_t *ptr) {
			alloc.free(&alloc.lookup_type((const char *)ptr));
		}
		using next_base_t::free;
		using base_t::free;

		template <typename... Args>
		constexpr typename SubAlloc::data_t *allocate(tag<typename SubAlloc::data_t>,
		                                              Args &&... args) {
			std::cout << std::string(detail::type_name<typename SubAlloc::data_t>{}) << " ualloc >"
			          << std::endl;
			typename SubAlloc::data_t *ptr = &detail::get_union<typename SubAlloc::data_t>(
			        *alloc.template allocate(tag<data_t>{}));
			new (ptr) typename SubAlloc::data_t(args...);
			std::cout << std::string(detail::type_name<typename SubAlloc::data_t>{}) << " ualloc <"
			          << std::endl;
			return ptr;
		};
		using next_base_t::allocate;
		using base_t::allocate;

		constexpr typename SubAlloc::data_t &lookup_type(const char *ptr) {
			return detail::get_union<typename SubAlloc::data_t>(alloc.lookup_type(ptr));
		}
	};
}

template <typename... Types>
struct union_alloc {

	using data_t = detail::var_union<typename Types::data_t...>;

	// pass self into lower allocators
	template <typename Alloc, bool check_oom, bool check_null_free>
	struct type : public detail::type_impl<Alloc, data_t, check_oom, check_null_free, Types...> {
		constexpr type(Alloc &alloc)
		    : detail::type_impl<Alloc, data_t, check_oom, check_null_free, Types...>{alloc} {
		}
	};
};
