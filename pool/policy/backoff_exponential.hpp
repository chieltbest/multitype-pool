//
// Created by chiel on 17/01/17.
//

#ifndef MULTITYPE_POOL_BACKOFF_EXPONENTIAL_HPP
#define MULTITYPE_POOL_BACKOFF_EXPONENTIAL_HPP

#include <thread>

template <unsigned MinCycles, unsigned MaxCycles>
class backoff_exponential {
	int cur = MinCycles;

public:
	void operator()() {
		if (cur > MaxCycles) {
			std::this_thread::yield();
		} else {
			for (volatile int i = 0; i < cur; ++i) {
			}
			cur = cur << 1;
		}
	}
};

#endif //MULTITYPE_POOL_BACKOFF_EXPONENTIAL_HPP
