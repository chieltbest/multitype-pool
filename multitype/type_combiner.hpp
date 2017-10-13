//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once
namespace mpl {
	namespace detail {
		constexpr unsigned combinations_select(unsigned n) {
			return n >= 2 ? 2 : 0;
		}

		template <unsigned>
		struct combinations_impl;

		template <>
		struct combinations_impl<0> {
			template <typename C, typename... Ts>
			using f = mpl::detail::rlist_tail_of_8;
		};

		template <>
		struct combinations_impl<2> {
			template <typename C, typename T, typename... Ts>
			using f = mpl::detail::rlist<mpl::list<mpl::call<C, T, Ts>...>,
			                             typename combinations_impl<combinations_select(
			                                     sizeof...(Ts))>::template f<C, Ts...>>;
		};
	}

	template <typename Comb, typename C>
	struct combinations {
		template <typename... Ts>
		using f = mpl::call<mpl::detail::recursive_join<C>,
		                    typename detail::combinations_impl<detail::combinations_select(
		                            sizeof...(Ts))>::template f<Comb, Ts...>>;
	};
}
