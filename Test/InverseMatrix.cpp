#include <vector>
using namespace std;
#include "..\ECalc\InverseMatrix.h"
void TestInverseMatrix()
{
	int size=2;
	doubles2D matrix(2);
	doubles2D inverse(2);
	matrix[0][0]=1.;
	matrix[1][1]=1.;
	CalcInverseMatrix(matrix, inverse);
	for(int i=0; i<size; ++i)
	{
		for(int j=0; j<size; ++j)
		{
			if(j)
				cout<<" ";
			cout<<matrix[i][j];
		}
		cout<<endl;
	}
}