#pragma once

#ifdef USE_SOURCE
	#define DLLDIST
	#define DLLDISTTEMPLATE 
#else
	#ifdef DISTRIBUTIONS_EXPORTS
		#define DLLDIST __declspec(dllexport)
		#define DLLDISTTEMPLATE 
	#else
		#define DLLDISTTEMPLATE extern
		#define DLLDIST __declspec(dllimport)
	#endif
#endif

#include <vector>
#include <numeric>
#include <algorithm>
#include <math.h>
#include <tchar.h>
