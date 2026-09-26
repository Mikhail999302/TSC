#include "MathParameterList.h"
using namespace MathModels;
namespace 
{
	inline void _SetContextValue( const CMathParameterValue& ParamValue ) 
	{
		ParamValue.SetContextValue();
	}
}//namespace anonim

/*inline */void CMathParameterValues::SetContextValues() const
{
	std::for_each( begin(), end(), _SetContextValue );
}
