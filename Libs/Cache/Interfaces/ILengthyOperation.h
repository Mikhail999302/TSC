#ifndef CACHE_INTERFACES_ILENGTHYOPERATION_H
#define CACHE_INTERFACES_ILENGTHYOPERATION_H
//This interface defines the lengthy operation whose result should be cached.
//The interface depends on two instansiation parameters:
//ReturnType - result of lengthy operation. No additional conditions except copying.
//ComparingStruct - the struct that defines the equality of two lengthy operations
//		the < operation should be defined.

namespace Tsc {
namespace Cacher
{
template<typename ReturnType,typename ComparingStruct>
class ILengthyOperation
{
public:
	virtual ~ILengthyOperation(){}
	virtual ReturnType Calculate()=0;
	virtual ComparingStruct GetComparingStruct()=0;
};
}//namespace Cacher
}//namespace Tsc
#endif //CACHE_INTERFACES_ILENGTHYOPERATION_H
