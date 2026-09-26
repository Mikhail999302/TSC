#pragma once
#include "StdAfx.h"
#include <C:\diploma\2012\TSC\Libs\TableManager\Entry.h>
#include <C:\diploma\2012\TSC\Libs\Formula\Entry.h>

namespace GroupedTSC
{
	//using namespace MathCalc;
	using namespace MathModels;
	typedef unsigned int uint;
	typedef std::vector<double> doubles;
	//typedef std::vector<uint> uints;
	typedef std::vector<MathString> MathStrings;
	typedef std::vector<CMathParameterValues> EstimatesArray;
	//typedef std::vector<MathCalc::MathString> CStrings;
	typedef std::pair<MathCalc::MathString,CMathParameterValue> CStrDb;
	typedef std::vector<CStrDb> CDict;
	typedef Tsc::TableManager::CTableManager<Tsc::Formula::CMathExpString<MathCalc::MathString> > TModelManager;
	typedef Tsc::TableManager::CTableManager<double> TSampleManager;

//Ёти функции должны покрывать все целые значени€ от 0 до своего максимального. 
//—корость не особенно критична. ¬ крайних случа€х можно просто сдвигать.
//	typedef uint*(*MergeStrategyDelegate)(int size,int &oSize);
}//namespace GroupedTSC