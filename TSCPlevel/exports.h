#pragma once 
#ifdef USE_SOURCE
	#define TSCAPP_DLLENTRY
	#define TSCAPP_DLLENTRYTEMPLATE 
#else
	#ifdef TSCPLEVEL_EXPORTS
		#define TSCAPP_DLLENTRY __declspec(dllexport)
		#define TSCAPP_DLLENTRYTEMPLATE 
	#else
		#define TSCAPP_DLLENTRYTEMPLATE extern
		#define TSCAPP_DLLENTRY __declspec(dllimport)
	#endif
#endif