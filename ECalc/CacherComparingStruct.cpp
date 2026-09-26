#include "CacherComparingStruct.h"

bool ECALC_API MathModels::operator <(const CCacherComparingStruct& _lhs, const CCacherComparingStruct& _rhs)
{
	if (_lhs.mDim <_rhs.mDim)
	{
		return true;
	}
	else if(_lhs.mDim == _rhs.mDim)
	{
		return _lhs.mType < _rhs.mType;
	}
	else
		return false;
}
