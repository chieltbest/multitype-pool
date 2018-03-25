//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <kvasir/mpl/mpl.hpp>

#include "../root/single_alloc.hpp"

// common functions for algorithms that build allocators

namespace detail {
	template <unsigned seg1_left, unsigned seg2_left, bool seg1_greater>
	struct least_combined_impl;

	template <unsigned seg2_left>
	struct least_combined_impl<0, seg2_left, false> {
		template <template <unsigned> class Seg1, template <unsigned> class Seg2,
		          template <typename...> class Union, unsigned N1, unsigned N2, typename C,
		          typename... Results>
		using f = kvasir::mpl::call<C, Results..., Union<Seg1<N1>, Seg2<N2>>>;
	};

	template <unsigned seg1_left>
	struct least_combined_impl<seg1_left, 0, true> {
		template <template <unsigned> class Seg1, template <unsigned> class Seg2,
		          template <typename...> class Union, unsigned N1, unsigned N2, typename C,
		          typename... Results>
		using f = kvasir::mpl::call<C, Results..., Union<Seg1<N1>, Seg2<N2>>>;
	};

	template <unsigned seg1_left, unsigned seg2_left>
	struct least_combined_impl<seg1_left, seg2_left, false> {
		template <template <unsigned> class Seg1, template <unsigned> class Seg2,
		          template <typename...> class Union, unsigned N1, unsigned N2, typename C,
		          typename... Results>
		using f = typename least_combined_impl<(seg1_left - 1), (seg2_left),
		                                       (sizeof(typename Seg1<(N1 + 1)>::data_t) >
		                                        sizeof(typename Seg2<(N2)>::data_t))>::
		        template f<Seg1, Seg2, Union, (N1 + 1), (N2), C, Results...,
		                   Union<Seg1<(N1)>, Seg2<(N2)>>>;
	};

	template <unsigned seg1_left, unsigned seg2_left>
	struct least_combined_impl<seg1_left, seg2_left, true> {
		template <template <unsigned> class Seg1, template <unsigned> class Seg2,
		          template <typename...> class Union, unsigned N1, unsigned N2, typename C,
		          typename... Results>
		using f = typename least_combined_impl<(seg1_left), (seg2_left - 1),
		                                       (sizeof(typename Seg1<(N1)>::data_t) >
		                                        sizeof(typename Seg2<(N2 + 1)>::data_t))>::
		        template f<Seg1, Seg2, Union, (N1), (N2 + 1), C, Results...,
		                   Union<Seg1<(N1)>, Seg2<(N2)>>>;
	};
}

template <typename T, typename Alloc>
struct seg_alloc_wrap {
	template <unsigned N>
	using f = typename Alloc::template f<kvasir::mpl::uint_<N>, T>;
};

template <typename T1, typename T2>
using effective_usage_less = kvasir::mpl::bool_<((T1::usage / sizeof(typename T1::data_t)) <
                                                 (T2::usage / sizeof(typename T2::data_t)))>;

/// find the configuration with the least fragmentation in a union allocator of two segmented
/// allocators using the least common denominator algorithm
template <typename T1, typename T2, typename SegT, typename Union,
          typename C = kvasir::mpl::identity>
using least_combined = typename detail::least_combined_impl<
        std::numeric_limits<unsigned long>::digits, std::numeric_limits<unsigned long>::digits,
        false>::
        template f<seg_alloc_wrap<T1, SegT>::template f, seg_alloc_wrap<T2, SegT>::template f,
                   Union::template f, 0, 0,
                   kvasir::mpl::extreme<kvasir::mpl::if_<kvasir::mpl::cfe<effective_usage_less>,
                                                         kvasir::mpl::at1<>, kvasir::mpl::at0<>>,
                                        C>>;
