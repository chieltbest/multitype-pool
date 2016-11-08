//
// Created by chiel on 26/10/16.
//

#ifndef MULTITYPE_POOL_AUTOSIZE_INT_HPP
#define MULTITYPE_POOL_AUTOSIZE_INT_HPP

#include <limits>

namespace detail {

	enum auto_int_idx {
		CHAR,
		SHORT,
		INT,
		LONG,
		LONG_LONG,
		BIG_NUM
	};

	constexpr auto_int_idx auto_int_generator(long long max) {
		return max <= std::numeric_limits<char>::max()
		       ? CHAR
		       : max <= std::numeric_limits<short>::max()
		         ? SHORT
		         : max <= std::numeric_limits<int>::max()
		           ? INT
		           : max <= std::numeric_limits<long>::max()
		             ? LONG
		             : max <= std::numeric_limits<long long>::max()
		               ? LONG_LONG
		               : BIG_NUM;
	}

	template<auto_int_idx>
	struct auto_int_impl {
		using r = long long; // TODO unsupported, assert
	};
	template<>
	struct auto_int_impl<CHAR> {
		using r = char;
	};
	template<>
	struct auto_int_impl<SHORT> {
		using r = short;
	};
	template<>
	struct auto_int_impl<INT> {
		using r = int;
	};
	template<>
	struct auto_int_impl<LONG> {
		using r = long;
	};
	template<>
	struct auto_int_impl<LONG_LONG> {
		using r = long long;
	};


	constexpr auto_int_idx auto_uint_generator(unsigned long long max) {
		return max <= std::numeric_limits<unsigned char>::max()
		       ? CHAR
		       : max <= std::numeric_limits<unsigned short>::max()
		         ? SHORT
		         : max <= std::numeric_limits<unsigned int>::max()
		           ? INT
		           : max <= std::numeric_limits<unsigned long>::max()
		             ? LONG
		             : max <= std::numeric_limits<unsigned long long>::max()
		               ? LONG_LONG
		               : BIG_NUM;
	}

	template<auto_int_idx>
	struct auto_uint_impl {
		using r = unsigned long long; // TODO unsupported, assert or use bignum
	};
	template<>
	struct auto_uint_impl<CHAR> {
		using r = unsigned char;
	};
	template<>
	struct auto_uint_impl<SHORT> {
		using r = unsigned short;
	};
	template<>
	struct auto_uint_impl<INT> {
		using r = unsigned int;
	};
	template<>
	struct auto_uint_impl<LONG> {
		using r = unsigned long;
	};
	template<>
	struct auto_uint_impl<LONG_LONG> {
		using r = unsigned long long;
	};

}

template<long long MAX>
using auto_int = typename detail::auto_int_impl<detail::auto_int_generator(MAX)>::r;

template<unsigned long long MAX>
using auto_uint = typename detail::auto_uint_impl<detail::auto_uint_generator(MAX)>::r;

#endif //MULTITYPE_POOL_AUTOSIZE_INT_HPP
