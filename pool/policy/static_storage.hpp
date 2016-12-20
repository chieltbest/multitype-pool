//
// Created by chiel on 22/11/16.
//

#ifndef MULTITYPE_POOL_STATIC_STORAGE_HPP
#define MULTITYPE_POOL_STATIC_STORAGE_HPP

#include <functional>
#include "../../autosize/autosize.hpp"

namespace {

	struct base {
	};
	struct derived : public base {
	};

	template<typename container, typename data_t, unsigned MAX>
	struct call_init {
		constexpr call_init(container &data) {
			check<data_t>(data, derived{});
		}

		/// sfinae tester
		template<typename T>
		constexpr auto check(container& data, derived)
		-> decltype(&T::template init<MAX, container>, void()) {
			data_t::template init<MAX>(data);
		}
		template<typename T>
		constexpr auto check(container& data, base) -> void {
			// data_t has no member function init, no-op
		}
	};

} // namespace

/// provides a simple wrapper class for initializing an array as global
/// and using it as template parameter
template<template<typename /*ptr_t*/> class data_template, unsigned MAX>
class static_storage_array_data {
public:
	using index_t = autosize::auto_uint<MAX + 1>;

	class ptr_t {
	public:
		index_t idx;

		constexpr ptr_t(index_t idx)
			: idx{index_t(idx + 1)} {
		}

		constexpr ptr_t(std::nullptr_t = nullptr)
			: idx{0} {
			// make nullptr zero, because that is more efficient when checking for null
		}
	};

	using data_t = data_template<ptr_t>;

private:
	data_t data[MAX];

	template<typename static_storage_t, static_storage_t &data> friend
	class static_storage;


public:
	constexpr static unsigned max_elems = MAX;

	constexpr static_storage_array_data()
		: data{} {
		call_init<static_storage_array_data, data_t, MAX>{*this};
	}

	constexpr data_t &operator[](index_t idx) {
		return data[idx];
	}

	constexpr ptr_t at(index_t idx) {
		return {idx};
	}

};

/// wrapper policy for making the static storage available as a type
template<typename static_storage_t, static_storage_t &data>
class static_storage {
public:
	using index_t = typename static_storage_t::index_t;
	using data_t  = typename static_storage_t::data_t;

	class ptr_t : public static_storage_t::ptr_t {
		using static_storage_t::ptr_t::idx;

	public:
		constexpr ptr_t(typename static_storage_t::ptr_t &ptr)
			: static_storage_t::ptr_t{ptr} {
		}

		constexpr ptr_t(typename static_storage_t::ptr_t &&ptr)
			: static_storage_t::ptr_t{ptr} {
		}

		constexpr ptr_t(index_t idx)
			: static_storage_t::ptr_t{idx} {
		}

		constexpr ptr_t(std::nullptr_t null = nullptr) noexcept
			: static_storage_t::ptr_t{null} {
		}

		constexpr explicit operator bool() {
			return *this != nullptr;
		}

		constexpr data_t &operator*() const {
			return data[idx - 1];
		}

		constexpr data_t *operator->() const {
			return &data[idx - 1];
		};

		constexpr ptr_t &operator++() {
			++idx;
			return *this;
		}

		constexpr ptr_t operator+(index_t add) const {
			return {index_t(idx + add)};
		}

		constexpr ptr_t operator-(index_t sub) const {
			return {index_t(idx - sub)};
		}

		constexpr bool operator==(const ptr_t &lhs) const {
			return idx == lhs.idx;
		}

		constexpr bool operator!=(const ptr_t &lhs) const {
			return !(*this == lhs);
		}

		template<typename Stream>
		friend constexpr Stream &operator<<(Stream &lhs, const ptr_t &rhs) {
			return lhs << &*rhs;
		}

		struct hash {
			auto operator()(const ptr_t& ptr) const {
				return ptr.idx;
			}
		};

	};

	constexpr ptr_t at(index_t idx) const {
		return {idx};
	}

	constexpr ptr_t begin() const {
		return {index_t{0}};
	}

	constexpr ptr_t end() const {
		return {static_storage_t::max_elems};
	}

};

#endif // MULTITYPE_POOL_STATIC_STORAGE_HPP
