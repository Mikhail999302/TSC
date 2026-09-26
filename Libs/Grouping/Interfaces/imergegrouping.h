/*
 Interface for different types of grouppings.
 Set parameters that is actual for your type of groupping and call operator() to get a vector of states that
 should be groupped. The states that should be groupped, should be marked with same integer value from zero to max
 number without gaps.
 see AutoGroup, StandardGroup, NoneGroup, ShrinkGroup
 */

#ifndef GROUPING_IMERGEGROUPING_H
#define GROUPING_IMERGEGROUPING_H
#include <vector>
namespace Tsc {
namespace Grouping {
template<class ReturnType=unsigned int, class ValueType=double> class IMergeGrouping
{
	int mGroupingIdent;
protected:
	std::vector<ValueType> states;
	int size;
	ValueType level;
	int minSize;
	std::vector<ReturnType> groups;

	typedef std::vector<ReturnType> ReturnTypes;
	typedef std::vector<ValueType> ValueTypes;
	
public:
		
	IMergeGrouping(int _identificator = 0):mGroupingIdent(_identificator){}
	virtual ~IMergeGrouping(){}
	virtual ReturnTypes operator()()=0;
	
	int getIdentificator(){return mGroupingIdent;}
	void setIdentificator(int _identificator){mGroupingIdent=_identificator;}
	void setSize(int _size)
	{
		size=_size;
	}
	void setStates(const ValueTypes & _states)
	{
		states=_states;
	}
	void setLevel(ValueType _level)
	{
		level=_level;
	}
	void setMinSize(int _minSize)
	{
		minSize=_minSize;
	}
};
}//namespace Grouping
}//namespace Tsc
#endif /*GROUPING_IMERGEGROUPPING_H*/
