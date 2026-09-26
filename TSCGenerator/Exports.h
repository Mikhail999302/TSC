#pragma once
#ifdef USE_SOURCE
#define TSCGEN_DLLENTRY
#define TSCGEN_DLLENTRYTEMPLATE 
#else
#ifdef TSCGENERATOR_EXPORTS
#define TSCGEN_DLLENTRY __declspec(dllexport)
#define TSCGEN_DLLENTRYTEMPLATE 
#else
#define TSCGEN_DLLENTRY __declspec(dllimport)
#define TSCGEN_DLLENTRYTEMPLATE extern
#endif
#endif

#include<vector>
#include<iostream>
#include<algorithm>
#include<exception>
#include <numeric>
#include<fstream>
#include <math.h>