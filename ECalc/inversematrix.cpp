// InverseMatrix.cpp
//
// Calculation routine of inverse matrix
// using Jacobi's method
// 
// 22.04.2003 [Andrei Demski]
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <math.h>
#include <iostream>
#include "InverseMatrix.h"
#include "Outdefs.h"

template< class T >
class CCompatibleVector {
	T* pData;
public:
	CCompatibleVector( int nSize ): pData(new T[nSize]) {}
	~CCompatibleVector() { delete[] pData; }
	operator T*() { return pData; }
	T& operator[](int nIndex) { return pData[nIndex]; }
};

bool CalcInverseMatrix( const doubles2D& X, doubles2D& XInverse, doubles& eig )
{
	doubles2D XT;
	doubles2D XXT;
	MultiplyMatrices( X, TransposeMatrix(X, XT), XXT );

	int n = (int)X.size();
	CCompatibleVector<double> Matrix( (n+1)*n/2 );
	CCompatibleVector<double> Vectors( n*n );

	int i, j;
	int k = 0;
	for ( j = 0; j < n; j++ )
		for ( i = 0; i <= j; i++ )
			Matrix[k++] = XXT[i][j];

	eigen( Matrix, Vectors, n );

// Prepare sqrt(Eigen values) vector ----------------
	doubles EigenValuesSqrt(n);
	if(!eig.empty())
		eig.resize(n);
	for ( i = 0, k = 0; i < n; i++, k += i )
	{
		if(!eig.empty())
			eig[i] = Matrix[i+k];
		EigenValuesSqrt[i] = sqrt( Matrix[i + k] );
		//if ( (EigenValuesSqrt[i] = sqrt( Matrix[i + k] )) == 0 )
		//	return false;
	}
// Prepare UT ---------------------------------------
	doubles2D U(n);
	for ( j = 0; j < n; j++ )
		for ( i = 0; i < n; i++ )
			U[i][j] = Vectors[j*n + i];
	doubles2D UT;
	TransposeMatrix(U, UT);
// Prepare V ----------------------------------------
	doubles2D V;
	MultiplyMatrices( XT, U, V );
	for ( i = 0; i < n; i++ )
		for ( j = 0; j < n; j++ )
			V[i][j] /= EigenValuesSqrt[j];
// Prepare L ----------------------------------------
	doubles2D L(n);
	for ( j = 0; j < n; j++ )
		for ( i = 0; i < n; i++ )
			if ( i != j)
				L[i][j] = 0;
			else
				L[i][j] = 1 / EigenValuesSqrt[i];
// Calculate X inverse and free resourses -----------
	doubles2D VL;
	MultiplyMatrices( MultiplyMatrices( V, L, VL ), UT, XInverse );
	return true;
}

doubles2D& MultiplyMatrices( const doubles2D& matrix1, const doubles2D& matrix2, doubles2D& result )
{
	int n = (int)matrix1.size();
	result.resize2D(n);
	int i, j, k;
	for ( i = 0; i < n; i++ )
		for ( j = 0; j < n; j++ )
		{
			result[i][j] = 0;
			for ( k = 0; k < n; k++ )
				result[i][j] += matrix1[i][k] * matrix2[k][j];
		}
	return result;
}

doubles2D& TransposeMatrix( const doubles2D& matrix, doubles2D& result )
{
	int n = (int)matrix.size();
	result.resize2D(n);
	for ( int i = 0; i < n; i++ )
		for ( int j = 0; j < n; j++ )
			result[i][j] = matrix[j][i];
	return result;
}
