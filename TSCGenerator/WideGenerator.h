//Generates the Pii by p0,..,pn-2,pch, p'0,..,p'n-2
//for input use vector<double>
//////////////////////////////////////////

#pragma once
#include "IGenerator.h"
#include "distribution.h"

namespace TSCGenerator
{

//pij=pi*pch*p'j
//pii=pi(1-pch+pch*p'i)
	class TSCGEN_DLLENTRY CWideGenerator:public IGenerator
	{
	public:
		CWideGenerator(const doubles& PBefore_, double pch_, const doubles& PAfter_,int seed_):
		  alpha(new CUniformDist(seed_)), isInit(false),sample(PBefore_.size()*PBefore_.size()),PBefore(PBefore_),
			  PAfter(PAfter_),PBeforeGen(0),PAfterGen(0),pch(pch_)
		  {}
		  ~CWideGenerator(void){delete PAfterGen; delete alpha; delete PBeforeGen;}
		  CWideGenerator(const CWideGenerator& gen);
		  CWideGenerator& operator=(const CWideGenerator& gen);

		  // Generate pij using current settings but new sample size
		  inline void Generate(int size_){size=size_;Generate();}
		  void SetSize(int _size){size=_size;}
		  // Generate pij using current settings
		  inline void Generate(void);

		  //get output
		  const doubles& GetSample(void){return sample;}
		  void SaveSample(const char * fileName,bool isTtranspose=false);
		  const doubles& GetPBefore(void){return PBefore;}
		  const doubles& GetPAfter(void){return PAfter;}
		  const double getPCh(){return pch;}

	protected:
		inline void InitModel(void);
		inline void GenerateOne(void);
		void TransposeSample(void);

	private:
		bool isInit;
		doubles sample;//square matrix of pij as one long array n^2
		doubles PBefore;
		doubles PAfter;
		double pch;
		int size;//sample size
		int dim;//sample dimention;
		CUniformDist *alpha;
		CChenUFDD* PBeforeGen;
		CChenUFDD* PAfterGen;
	};

}//namespace TSCGenetator
