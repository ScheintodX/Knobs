#ifndef ALGORITHM_H
#define ALGORITHM_H

// Some commonly used algorithms which can't be
// used from stdlib because conflicts with arduino

namespace Math {

	template <typename T>
	static inline T& kmin( T &one, T &two ){
		return one < two ? one : two;
	}

	template <typename T>
	static inline T& kmax( T &one, T &two ){
		return one > two ? one : two;
	}

	template <typename T>
	static inline T kabs( T val ){
		return val >= 0 ? val : -val;
	}

}

#endif
