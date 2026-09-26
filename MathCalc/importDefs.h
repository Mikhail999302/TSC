#ifndef IMPORTDEFS_H_
#define IMPORTDEFS_H_
#ifdef USE_MATHCALC_SOURCE	// use MathCalc sources as a part of a project
#	define MATHCALC_API
#	define EXPIMP_TEMPLATE
#else						// use MathCalc as DLL
#	ifdef MATHCALC_EXPORTS
#		define MATHCALC_API __declspec(dllexport)
#		define EXPIMP_TEMPLATE
#	else
#		define MATHCALC_API __declspec(dllimport)
#		define EXPIMP_TEMPLATE extern
#	endif
#endif
#endif /*IMPORTDEFS_H_*/
