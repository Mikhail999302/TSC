#pragma once
#include <exception>
namespace Exceptions
{
class SampleSizeException:public std::exception
	{
		virtual const char* what(){return "Sample size are not correct";}
	};
class NotInitialisedException:public std::exception
	{
		virtual const char* what(){return "Model isn't initialised.";}
	};
}//namespace Exceptions