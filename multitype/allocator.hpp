//          Copyright Chiel Douwes 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once

namespace detail {
	template<typename T>
	struct type_name {
		operator std::string() const {
			const char
				*name{__cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr)};
			std::string res{name}; // copy from name into string
			delete name;
			return res;
		}
	};
}
// end debug functions

template<typename T>
struct tag {
};
