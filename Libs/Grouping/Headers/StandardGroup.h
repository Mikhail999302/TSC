#ifndef GROUPING_STANDARDGROUP_H
#define GROUPING_STANDARDGROUP_H
#include <vector>
#include <math.h>
#include "..\Interfaces\IMergeGrouping.h"
namespace Tsc {
namespace Grouping {
using namespace std;
template<class ReturnType=unsigned int, class ValueType=double> class StandardGroup :
	public IMergeGrouping<ReturnType, ValueType>
{
		typedef std::vector<ReturnType> ReturnTypes;
		typedef std::vector<ValueType> ValueTypes;
		
public:
	StandardGroup(int _identificator = 0):IMergeGrouping<ReturnType,ValueType>(_identificator){}
	virtual /*override*/ ReturnTypes operator()()
	{
		int len=(int) sqrt((double)this->states.size());
		ReturnTypes rez(len*len);
		for (int i=0; i<len; ++i)
			for (int j=0; j<len; ++j)
			{
				if (i==j)
					rez[i*len+j]=i;
				else if (!j)
					rez[i*len+j]=len;
				else if (!i)
					rez[i*len+j]=len+1;
				else
					rez[i*len+j]=len+2;
			}
		return rez;
	}
};
}//namespace Grouping
}//namespace Tsc
#endif /*GROUPING_STANDARDGROUP_H*/

