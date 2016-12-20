//
// Created by chiel on 20/11/16.
//

#ifndef MULTITYPE_POOL_STATIC_DATA_HPP
#define MULTITYPE_POOL_STATIC_DATA_HPP

template<typename FreelistPolicy>
class freelist_data_store {
	FreelistPolicy freelist;

public:
	using default_data_t = typename FreelistPolicy::data_t;

	/// wrapper class for converting the raw data/freelist
	/// stack pointer into the requested data type
	template<typename T>
	class ptr_t : public FreelistPolicy::ptr_t {
	public:
		template<typename... Args>
		constexpr ptr_t(Args &&... args)
			: FreelistPolicy::ptr_t{args...} {
		}

		constexpr T &operator*() {
			// TODO make a better error message for allocating from a wrong type
			return static_cast<T &>(FreelistPolicy::ptr_t::operator*());
		}

		constexpr T *operator->() {
			return FreelistPolicy::ptr_t::operator->();
		};

		constexpr ptr_t &operator++() {
			return FreelistPolicy::ptr_t::operator++();
		}

		template<typename int_t>
		constexpr ptr_t operator+(int_t add) {
			return FreelistPolicy::ptr_t::operator+(add);
		}

		template<typename int_t>
		constexpr ptr_t operator-(int_t sub) {
			return FreelistPolicy::ptr_t::operator-(sub);
		}

		constexpr bool operator==(const ptr_t &lhs) const {
			return FreelistPolicy::ptr_t::operator==(lhs);
		}

		constexpr bool operator!=(const ptr_t &lhs) const {
			return FreelistPolicy::ptr_t::operator!=(lhs);
		}

		template<typename Stream>
		friend constexpr Stream &operator<<(Stream &lhs, const ptr_t &rhs) {
			return lhs << static_cast<const typename FreelistPolicy::ptr_t &>(rhs);
		}
	};

	constexpr freelist_data_store()
		: freelist{} {
	}

	template<typename T>
	ptr_t<T> allocate() {
		return freelist.pop();
	}

	template<typename T>
	void free(ptr_t<T> ptr) {
		freelist.push(ptr);
	}

	auto get_free() {
		return freelist.get_free();
	}

};

#endif // MULTITYPE_POOL_STATIC_DATA_HPP
