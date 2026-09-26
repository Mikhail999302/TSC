// InverseMatrix.h
//////////////////////////////////////////////////////////////////////
#pragma once
#include "ECalcMisc.h"

#include <vector>
template< class T >
class vector2D : public std::vector< T > 
{
public:
	vector2D() {}
	vector2D(typename std::vector<T>::size_type N, bool use_resize2D = true ) { use_resize2D ? resize2D(N) : resize(N); }
	void resize2D(typename std::vector<T>::size_type N ) 
	{ 
		// resizes to square matrix N * N
		resize( N );
		for (typename std::vector< T >::iterator it = std::vector< T >::begin(); it != std::vector< T >::end(); ++it )
			it->resize(N);
	}
};

typedef std::vector<double> doubles;
typedef vector2D<doubles> doubles2D;

bool ECALC_API CalcInverseMatrix( const doubles2D& X, doubles2D& XInverse, doubles& eig);

doubles2D& MultiplyMatrices( const doubles2D& matrix1, const doubles2D& matrix2, doubles2D& result );
doubles2D& TransposeMatrix( const doubles2D& matrix, doubles2D& result );
