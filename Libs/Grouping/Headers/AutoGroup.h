#ifndef GROUPING_AUTOGROUP_H
#define GROUPING_AUTOGROUP_H
#include <functional>
#include <algorithm>
#include <vector>
#include "..\Interfaces\IMergeGrouping.h"
namespace Tsc {
namespace Grouping {
using namespace std;
template<class ReturnType=unsigned int, class ValueType=double> class AutoGroup :
	public IMergeGrouping<ReturnType, ValueType>
{
		typedef std::vector<ReturnType> ReturnTypes;
		typedef std::vector<ValueType> ValueTypes;
		
	void UnionWeights(const ValueTypes& _weightsValues, ReturnTypes & _weights, ReturnTypes& _groupping,
			int _minGroupNum, ValueType _level);
	void ShrinkGroupping(ReturnTypes& _groupping);
	bool ShrinkGroupping(ReturnTypes& _groupping, ReturnType _idx, ReturnType _step);
public:
	AutoGroup(int _identificator = 0):IMergeGrouping<ReturnType,ValueType>(_identificator){}
	virtual /*override*/ReturnTypes operator()()
	{
		ReturnTypes result(this->states.size());
		ReturnTypes weights(this->states.size());
		for (int i=0; i<(int)this->states.size(); ++i)
		{
			result[i] = i;
			weights[i] = i;
		}
		UnionWeights(this->states, weights, result,this->minSize, (ValueType)this->level/this->size);
		return result;
	}
};

template<class ReturnType=unsigned int, class ValueType=double> class CStatesOrder :
	public std::binary_function<ReturnType, ReturnType, bool>
{
	const std::vector<ValueType>& weights;
	std::vector<ReturnType>& groupping;
public:
	ValueType SumByIdx(ReturnType idx);
public:
	CStatesOrder(const std::vector<ValueType>& _weights, std::vector<ReturnType>& _groupping) :
		weights(_weights), groupping(_groupping)
	{
	}
	void SetGroupping(const std::vector<ReturnType>& _groupping)
	{
		groupping=_groupping;
	}
	bool operator()(ReturnType _lhs, ReturnType _rhs)
	{
		return SumByIdx(_lhs)<SumByIdx(_rhs);
	}
	virtual ~CStatesOrder(void)
	{
	}
};
}//namespace Grouping
}//namespace Tsc
#include "..\Implementation\AutoGroup.hpp"
#endif /*GROUPING_AUTOGROUP_H*/
