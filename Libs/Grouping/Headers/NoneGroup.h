#ifndef GROUPING_NONEGROUP_H
#define GROUPING_NONEGROUP_H
#include <vector>
#include "..\Interfaces\IMergeGrouping.h"
namespace Tsc {
namespace Grouping {
using namespace std;
template<class ReturnType=unsigned int, class ValueType=double> class NoneGroup :
	public IMergeGrouping<ReturnType, ValueType>
{
		typedef std::vector<ReturnType> ReturnTypes;
		typedef std::vector<ValueType> ValueTypes;
public:
	NoneGroup(int _identificator = 0):IMergeGrouping<ReturnType,ValueType>(_identificator){}
	virtual /*override*/ReturnTypes operator()()
	{
		ReturnTypes result(this->states.size());
		for (typename ReturnTypes::iterator p=result.begin(); p!=result.end(); ++p)
			(*p)=(ReturnType)(p-result.begin());
		return result;
	}
};
}//namespace Grouping
}//namespace Tsc
#endif /*GROUPING_NONEGROUP_H*/
