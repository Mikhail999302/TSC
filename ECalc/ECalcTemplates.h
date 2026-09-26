// ECalcTemplates.h: misc templates
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>

namespace MathModels {

	template< class T >
	class vector2D : public std::vector<T> {
	public:
		vector2D() {}
		explicit vector2D(typename std::vector<T>::size_type N, bool use_resize2D = true ) 
			{ use_resize2D ? resize2D(N) : resize(N); }
		void resize2D(typename std::vector<T>::size_type N ) { // resizes to square matrix N * N
			resize( N );
			for (typename std::vector<T>::iterator it = std::vector<T>::begin(); it != std::vector<T>::end(); ++it )
				it->resize(N);
		}
	};

} // MathModels

namespace DataProcessing {

	template< class Vec, class Element >
	void SetElementAtGrowIndex( Vec& vec, int index, const Element& element, int grow_by = 5 )
	{
		if ( vec.size() <=(size_t) index ) {
			if ( vec.capacity() == vec.size() ) 
				vec.reserve( vec.size() + grow_by );
			vec.push_back( element );
		}
		else
			vec[index] = element;
	}

} // DataProcessing

