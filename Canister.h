#ifndef CANISTER_H
#define CANISTER_H

/*
 * Very lightweight container implementation using
 * arrays with fixed maximum size.
 */

namespace Knobs {

	template <typename T, int N>
	class Canister {

		private:
			T *_reservoir[ N ];
			int _fill;
			int _current;

		public:
			/**
			 * Add one item to end of list
			 * returns false if the list is full.
			 */
			inline bool add( T& item ) {

				if( _fill == N ) return false;

				_reservoir[ _fill++ ] = &item;

				return true;
			}

			/**
			 * In order to save space this class has the "iterator" integrated.
			 */
			inline T *next() {

				if( _current >= _fill ) return (T*)0;

				return _reservoir[ _current++ ];
			}

			inline T *first() {

				_current = 0;

				return next();
			}

			inline int fill() {

				return _fill;
			}

	};

	//#include "Canister.tpp"
}

#endif
