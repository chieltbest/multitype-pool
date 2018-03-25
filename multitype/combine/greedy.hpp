//          Copyright Chiel Douwes 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "combine.hpp"

/// find the combination of elements that results in the least fragmentation in a list of elements
template <typename SegAlloc, typename UnionAlloc, typename C = kvasir::mpl::identity>
using find_optimal = kvasir::mpl::combinations<
        kvasir::mpl::uint_<2>,
        kvasir::mpl::push_back<
                SegAlloc, kvasir::mpl::push_back<UnionAlloc, kvasir::mpl::cfe<least_combined>>>,
        kvasir::mpl::listify,
        kvasir::mpl::extreme<
                kvasir::mpl::if_<kvasir::mpl::transform<kvasir::mpl::unpack<kvasir::mpl::at0<>>,
                                                        kvasir::mpl::cfe<effective_usage_less>>,
                                 kvasir::mpl::at0<>, kvasir::mpl::at1<>>,
                kvasir::mpl::unpack<C>>>;

template <typename SegAlloc, typename UnionAlloc, typename C = kvasir::mpl::identity>
struct greedy_combine;

namespace detail {
	template <typename... Ts>
	struct greedy_combine_impl {
		template <typename SegAlloc, typename UnionAlloc, typename C>
		using f = kvasir::mpl::call<
		        find_optimal<SegAlloc, UnionAlloc, greedy_combine<SegAlloc, UnionAlloc, C>>, Ts...>;
	};

	template <typename T>
	struct greedy_combine_impl<T> {
		template <typename SegAlloc, typename UnionAlloc, typename C>
		using f = kvasir::mpl::call<C, T>;
	};
}

template <typename SegAlloc, typename UnionAlloc, typename C>
struct greedy_combine {
	template <typename... Ts>
	using f = typename detail::greedy_combine_impl<Ts...>::template f<SegAlloc, UnionAlloc, C>;
};
