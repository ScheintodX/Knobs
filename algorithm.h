#ifndef ALGORITHM_H
#define ALGORITHM_H

// Some commonly used algorithms which can't be
// used from stdlib because conflicts with arduino

#undef min
#undef max

namespace Math {

	template <typename T>
	static inline T& min( T &one, T &two ){
		return one < two ? one : two;
	}

	template <typename T>
	static inline T& max( T &one, T &two ){
		return one > two ? one : two;
	}

	template <typename T>
	static inline T abs( T val ){
		return val >= 0 ? val : -val;
	}

}

#endif

