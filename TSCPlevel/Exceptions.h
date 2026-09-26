#pragma once
namespace PLevel
{
	namespace Exceptions
	{
		class /*TSCAPP_DLLENTRY*/ IncorrectSizeException:public std::exception
		{
			virtual const char *what() const{return "Input arrays are different sizes.";}
		};

		class /*TSCAPP_DLLENTRY*/ DivideByZeroException:public std::exception
		{
			virtual const char *what() const{return "Divide By Zero";}
		};

		class /*TSCAPP_DLLENTRY*/ NotInitializedException:public std::exception
		{
			virtual const char *what() const{return "Plevel hasn't been initialized";}
		};
	}//namespace Exceptions
}//namespace PLevel