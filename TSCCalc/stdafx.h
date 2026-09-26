#pragma once
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#pragma warning(disable : 4290)

#ifdef USE_SOURCE
	#define TSCCALC_DLLENTRY
	#define TSCCALC_DLLENTRYTEMPLATE 
#else
	#ifdef TSCCALC_EXPORTS
		#define TSCCALC_DLLENTRY __declspec(dllexport)
		#define TSCCALC_DLLENTRYTEMPLATE 
	#else
		#define TSCCALC_DLLENTRYTEMPLATE extern
		#define TSCCALC_DLLENTRY __declspec(dllimport)
	#endif
#endif

#include<vector>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<exception>
//#include <atlstr.h>
#include"Exceptions.h"
#include "..\\ECalc\\ECalc.h"
#include "..\\TSCGenerator\\Generator.h"
#include "..\\TSCPLevel\\PLevel.h"
