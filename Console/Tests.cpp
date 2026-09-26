#include "..\TSCPLevel\plevel.h"
//#include "..\..\TSCCalc\Distributions\BoundedSample.h"

void TestMoments()
{
	//using namespace std;
	//using namespace Distributions;
	//auto_ptr<CSampleHolder> sample(new CSampleHolder());
	//auto_ptr<CMomentsGroupping> momentsGroup(new CMomentsGroupping(2));
	//sample->InsertValue(1);
	//sample->InsertValue(1);
	//sample->InsertValue(1);
	//auto_ptr<CSampleHolder> moments(sample->GetModification(momentsGroup.get()));
	//cout<<moments->GetAt(0)<<endl;
	//cout<<moments->GetAt(1);
}

void TestPLevel()
{
	/*const int numIntervals = 10;

	PLevel::doubles teoretic(numIntervals);
	int emp[] = {8,12,16,8,11,11,14,4,7,9};
	PLevel::doubles empiric(emp,emp+numIntervals);
	for (int i=0; i<numIntervals; ++i)
	{
		teoretic[i] = 1./numIntervals;
	}

	PLevel::CPLevel plevel;
	plevel.Init(teoretic,100,Tsc::Grouping::NoneGroup<unsigned int,double>());
	plevel.CalcChiValue(empiric);
	cout<<" "<<plevel.GetPLevelData().plevel;*/
}