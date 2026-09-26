#ifndef CACHE_HEADERS_LENGTHYOPERATION_H
#define CACHE_HEADERS_LENGTHYOPERATION_H
#include <functional>
#include "..\Interfaces\ILengthyOperation.h"
//Comparison of two ComparingStruct for predicate
namespace Tsc {
namespace Cacher
{
template<typename ComparingStruct>
class CacheComparer:public std::binary_function<ComparingStruct,ComparingStruct,bool>
{
public:
	bool operator()(const ComparingStruct& _lhs, const ComparingStruct& _rhs)const
	{
		return _lhs<_rhs;
	}
};
}//namespace Cache
}//namespace Tsc
#endif// CACHE_HEADERS_LENGTHYOPERATION_H
