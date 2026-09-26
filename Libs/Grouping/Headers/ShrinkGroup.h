#ifndef GROUPING_SHRINKGROUP_H
#define GROUPING_SHRINKGROUP_H
#include <vector>
#include <math.h>
#include "..\Interfaces\IMergeGrouping.h"
namespace Tsc {
namespace Grouping {
using namespace std;
template<class ReturnType=unsigned int, class ValueType=double> class ShrinkGroup :
	public IMergeGrouping<ReturnType, ValueType>
{
		typedef std::vector<ReturnType> ReturnTypes;
		typedef std::vector<ValueType> ValueTypes;
public:
	ShrinkGroup(int _identificator = 0):IMergeGrouping<ReturnType,ValueType>(_identificator){}
	virtual /*override*/ReturnTypes operator()()
	{
		int len=(int)sqrt((double)this->states.size());
		ReturnTypes rez(len);
		for (int i=0; i<(int)rez.size(); ++i)
		{
			if (!i)
				rez[0]=0;
			else
				rez[i]=2-(i%2);
		}
		return rez;
	}
};
}//namespace Grouping
}//namespace Tsc
#endif /*GROUPING_SHRINKGROUP_H*/
