#ifndef CACHE_HEADER_CACHER_H
#define CACHE_HEADER_CACHER_H
#include <map>
#include "..\Interfaces\ILengthyOperation.h"
#include "CacheComparer.h"
namespace Tsc {
namespace Cacher
{
template<typename ReturnType,typename ComparingStruct>
class Cacher
{
	std::map< ComparingStruct, ReturnType, CacheComparer<ComparingStruct> > mMap;
public:
	ReturnType GetCalculatedValue(ILengthyOperation<ReturnType,ComparingStruct> & _operation)
	{
		typename std::map<ComparingStruct, ReturnType, CacheComparer<ComparingStruct> >::iterator iter = 
				mMap.find(_operation.GetComparingStruct());
		if(iter!=mMap.end())
			return (*iter).second;
		else
			mMap.insert(make_pair(_operation.GetComparingStruct(),_operation.Calculate()));
		return mMap[_operation.GetComparingStruct()];
	}
};
}//namespace Cacher
}//namespace Tsc
#endif// CACHE_HEADER_CACHER_H
