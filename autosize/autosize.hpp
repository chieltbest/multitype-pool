//
// Created by chiel on 26/10/16.
//

#ifndef MULTITYPE_POOL_AUTOSIZE_INT_HPP
#define MULTITYPE_POOL_AUTOSIZE_INT_HPP

#include <limits>

namespace autosize {

	namespace {

		enum auto_int_idx {
			I8, I16, I32, I64, BIG_NUM
		};

		constexpr auto_int_idx auto_int_generator(long long max) {
			return max <= std::numeric_limits<int_least8_t>::max()
			       ? I8
			       : max <= std::numeric_limits<int_least16_t>::max()
			         ? I16
			         : max <= std::numeric_limits<int_least32_t>::max()
			           ? I32
			           : max <= std::numeric_limits<int_least64_t >::max()
			             ? I64
			             : BIG_NUM;
		}

		template<auto_int_idx>
		struct auto_int_impl {
			using r = int_least64_t; // TODO: unsupported, assert
		};
		template<>
		struct auto_int_impl<I8> {
			using r = int_least8_t;
		};
		template<>
		struct auto_int_impl<I16> {
			using r = int_least16_t;
		};
		template<>
		struct auto_int_impl<I32> {
			using r = int_fast32_t;
		};
		template<>
		struct auto_int_impl<I64> {
			using r = int_least64_t;
		};


		constexpr auto_int_idx auto_uint_generator(unsigned long long max) {
			return max <= std::numeric_limits<uint_least8_t>::max()
			       ? I8
			       : max <= std::numeric_limits<uint_least16_t>::max()
			         ? I16
			         : max <= std::numeric_limits<uint_least32_t>::max()
			           ? I32
			           : max <= std::numeric_limits<uint_least64_t>::max()
			             ? I64
			             : BIG_NUM;
		}

		template<auto_int_idx>
		struct auto_uint_impl {
			using r = uint_least64_t; // TODO: unsupported, assert or use bignum
		};
		template<>
		struct auto_uint_impl<I8> {
			using r = uint_least8_t;
		};
		template<>
		struct auto_uint_impl<I16> {
			using r = uint_least16_t;
		};
		template<>
		struct auto_uint_impl<I32> {
			using r = uint_least32_t;
		};
		template<>
		struct auto_uint_impl<I64> {
			using r = uint_least64_t;
		};


		template<std::size_t size>
		struct int_sizeof_impl {
			using r = typename int_sizeof_impl<size + 1>::r;
		};
		template<>
		struct int_sizeof_impl<1> {
			using r = int8_t;
		};
		template<>
		struct int_sizeof_impl<2> {
			using r = int16_t;
		};
		template<>
		struct int_sizeof_impl<4> {
			using r = int32_t;
		};
		template<>
		struct int_sizeof_impl<8> {
			using r = int64_t;
		};


		template<std::size_t size>
		struct uint_sizeof_impl {
			using r = typename uint_sizeof_impl<size + 1>::r;
		};
		template<>
		struct uint_sizeof_impl<1> {
			using r = uint8_t;
		};
		template<>
		struct uint_sizeof_impl<2> {
			using r = uint16_t;
		};
		template<>
		struct uint_sizeof_impl<4> {
			using r = uint32_t;
		};
		template<>
		struct uint_sizeof_impl<8> {
			using r = uint64_t;
		};

	} // namespace

	template<long long MAX>
	using auto_int = typename auto_int_impl<auto_int_generator(MAX)>::r;

	template<unsigned long long MAX>
	using auto_uint = typename auto_uint_impl<auto_uint_generator(MAX)>::r;


	template<unsigned size>
	using int_sizeof = typename int_sizeof_impl<size>::r;

	template<unsigned size>
	using uint_sizeof = typename uint_sizeof_impl<size>::r;

} // namespace autosize

#endif //MULTITYPE_POOL_AUTOSIZE_INT_HPP
