#include <iostream>
using namespace std;
#include "../TSCGenerator/WideGenerator.h"
using namespace TSCGenerator;
void TestWideGenerator()
{
	TSCGenerator::doubles before(2,0.5);
	TSCGenerator::doubles after(2,0.5);
	double pch = 1;
	TSCGenerator::CWideGenerator gen(before,pch,after,1);
	gen.Generate(1000000);
	TSCGenerator::doubles sample(gen.GetSample());
	for(int i=0; i<4; ++i) 
	{
		cout<<sample[i]<<" ";
		if (!((i+1)%(int)before.size()))
			cout<<endl;
	}
}