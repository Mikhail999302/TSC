#ifndef GROUPING_AUTOGROUP_HPP
#define GROUPING_AUTOGROUP_HPP
#include "..\Headers\AutoGroup.h"
template <class ReturnType, class ValueType>
ValueType Tsc::Grouping::CStatesOrder<ReturnType, ValueType>::SumByIdx(ReturnType idx)
{
	ValueType rez=0;
	for(typename std::vector<ReturnType>::const_iterator p=groupping.begin(); p!=groupping.end(); ++p)
	{
		if(*p==idx)
			rez+=*(weights.begin()+(p-groupping.begin()));
	}
	return rez;
}

template <class ReturnType, class ValueType> 
bool Tsc::Grouping::AutoGroup<ReturnType, ValueType>::ShrinkGroupping(ReturnTypes& _groupping, ReturnType _idx, ReturnType _step)
{
	bool isAny=true;
	for(typename ReturnTypes::iterator iter= _groupping.begin(); iter != _groupping.end(); ++iter)
	{
		if(*iter==_idx)
		{
			*iter -= _step;
			isAny = false;
		}
	}
	return isAny;
}

template <class ReturnType, class ValueType> 
void Tsc::Grouping::AutoGroup<ReturnType, ValueType>::ShrinkGroupping(ReturnTypes& _groupping)
{
	ReturnType maxIdx=*std::max_element(_groupping.begin(), _groupping.end());
	ReturnType minIdx=*std::min_element(_groupping.begin(), _groupping.end());
	ReturnType step=minIdx;
	for(int i=(int)minIdx; i<=(int)maxIdx; ++i)
	{
		if(ShrinkGroupping(_groupping, i, step))
		{
			++step;
		}
	}
}

template <class ReturnType, class ValueType> 
void Tsc::Grouping::AutoGroup<ReturnType, ValueType>::UnionWeights(const ValueTypes& _weightsValues, ReturnTypes & _weights, ReturnTypes& _groupping, int _minGroupNum, ValueType _level)
{
	CStatesOrder<ReturnType, ValueType> statesOrder(_weightsValues,_groupping);
	sort(_weights.begin(),_weights.end(),statesOrder);
	while((int)_weights.size() > _minGroupNum && (_level<=0 || statesOrder.SumByIdx(_weights[0]) < _level))
	{
		ReturnType min=_weights[0]>_weights[1]?_weights[1]:_weights[0];
		for(int i=0;i<(int)_groupping.size();++i)
		{
			if(_groupping[i]==_weights[0]||_groupping[i]==_weights[1])
				_groupping[i]=min;
		}
		_weights[0] = _weights[1] = min;
		_weights.erase(unique(_weights.begin(), _weights.end()), _weights.end());
		statesOrder.SetGroupping(_groupping);
		sort(_weights.begin(), _weights.end(), statesOrder);
	}
	ShrinkGroupping(_groupping);
}

#endif /*GROUPING_AUTOGROUP_HPP*/
