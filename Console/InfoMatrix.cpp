#include "..\TSCCalc\InfoMatrixEstimation.h"
void TestInfoMatrix()
{
	GroupedTSC::CInfoMatrixEstimation ime;
	const int size=3;
	Tsc::Grouping::NoneGroup<int,double> group;
	group.setStates(vector<double>(size*size));
	vector<double> pi(size-1,1./size);
	double pCh=0.3, pAdv=0.03;
	ime.setParameters(pi,pCh,pAdv);
	ime.setInfoMatrix(size,group);
	for (int iAdv=0; iAdv<100; ++iAdv)
	{
		for (int iCh=0; iCh<100; ++iCh)
		{
			for (int ip0=0; ip0<100; ++ip0)
			{
				pi[0]=0.2+ip0*0.002;
				pi[1]=0.3333;
				pCh=0.2+(iCh)*0.002;
				pAdv=(iAdv)*0.001;
				ime.setParameters(pi,pCh,pAdv);
				doubles d = ime.calcInverse();
				if(sqrt(d[d.size()-1]/d[0])>65)
				{
					for(size_t i=0; i<d.size(); ++i)
					{
						cout<<d[i]<<" ";
					}
					cout<<sqrt(d[d.size()-1]/d[0])<<"params:";
					for (size_t i=0; i<pi.size(); ++i)
					{
						cout<<pi[i]<<" ";
					}
					cout<<pCh<<" "<<pAdv<<endl;
				}
			}
		}
		//cout<<endl;
	}
}