#pragma once
#include <C:\diploma\2012\TSC\Libs\Grouping/Entry.h>
#include "Typedefs.h"
#include "exports.h"
#include "Exceptions.h"
#include "ProbDist.h"
#include "..\Dumper\Dump.h"


namespace PLevel
{
struct TSCAPP_DLLENTRY PLevelData
{
	double criterion_value;
	int df;
	double plevel;
	double minNPi;
};

class TSCAPP_DLLENTRY CPLevel
{
	bool isInit;
	doubles fullTheoretic;
	doubles theoretic;
	doubles empiric;
	uints mergeStrategy;
	int size;
	PLevelData chiData;
public:

	static void SumByIdx(const doubles& from,const uints& with,doubles& to);
public:
	const static double eps;
	CPLevel(void){}
	~CPLevel(void);
	inline void Init(const doubles& fullTheoretic_, int size_ , Tsc::Grouping::IMergeGrouping<uint,double>& Strategy);
	inline void CalcChiValue(const doubles& fullEmpiric,bool isCheckSize=false);
	inline void CalcChiValue(const doubles& fullEmpiric,int df,bool isCheckSize=false);
	inline const PLevelData& GetPLevelData(void){return chiData;}
};
}//namespace PLevel