//Generates the Pii by p0,..,pn-2,pch,padv
//for input use vector<double>
//////////////////////////////////////////

#pragma once
#include "IGenerator.h"
#include "distribution.h"

namespace TSCGenerator
{
	class /*TSCGEN_DLLENTRY*/ CGeneratorException:public std::exception
	{
		virtual const char *what( ) const throw( ){return "Uninitialized!";};
	};
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	TSCGEN_DLLENTRYTEMPLATE template class TSCGEN_DLLENTRY std::allocator< double >;
	TSCGEN_DLLENTRYTEMPLATE template class TSCGEN_DLLENTRY std::vector< double >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA



	class TSCGEN_DLLENTRY CGenerator: public IGenerator
	{
	public:
		CGenerator(const doubles& distribution_,int seed_):
		  distribution(distribution_),alpha(new CUniformDist(seed_)),
			  poly(0),isInit(false)
		  {sample.resize((distribution.size())*(distribution.size()));}
		  virtual ~CGenerator(void){delete poly;delete alpha;}
		  CGenerator(const CGenerator& gen);
		  virtual CGenerator& operator=(const CGenerator& gen);

		  // Generate pij using current settings but new size
		  virtual inline void Generate(int size_){size=size_;Generate();}
		  void SetSize(int _size){size=_size;}
		  // Generate pij using current settings
		  virtual inline void Generate(void);

		  //get output
		  virtual const doubles& GetSample(void){return sample;}
		  virtual void SaveSample(const char * fileName,bool isTtranspose=false);
		  virtual const doubles& GetDistribution(void){return distribution;}
		  //edited 24.09
		  //virtual void SetSample(doubles u_sample);

	protected:
		virtual inline void InitModel(void){};
		virtual inline void GenerateOne(void){};
		virtual void TransposeSample(void);

	protected:
		bool isInit;
		doubles distribution;//p0,pn-2,pch,padv as vector watch the order
		doubles sample;//square matrix of pij as one long array n^2

		int size;//sample size
		int dim;//sample dimention;
		CUniformDist *alpha;
		CChenUFDD *poly;
	};


	//****************************************
	//******  class CGenerator_1Pos   ********
	//модель с одним рекламируемым продуктом**
	//****************************************
	class TSCGEN_DLLENTRY CGenerator_1Pos: public CGenerator
	{
	public:
		CGenerator_1Pos(const doubles& distribution_,int seed_):CGenerator(distribution_, seed_)
		{sample.resize((distribution.size()-1)*(distribution.size()-1));}
		virtual ~CGenerator_1Pos(void){/*delete poly;delete alpha;*/}
		CGenerator_1Pos(const CGenerator_1Pos& gen);
		CGenerator_1Pos& operator=(const CGenerator_1Pos& gen);

		//// Generate pij using current settings but new size
		//inline void Generate(int size_){size=size_;Generate();}
		//void SetSize(int _size){size=_size;}
		//// Generate pij using current settings
		//inline void Generate(void);

		////get output
		//const doubles& GetSample(void){return sample;}
		//void SaveSample(const char * fileName,bool isTtranspose=false);
		//const doubles& GetDistribution(void){return distribution;}

		const doubles& GetP(void){if(isInit)return pi;throw CGeneratorException();}

	protected:
		virtual inline void InitModel(void);
		virtual inline void GenerateOne(void);
		//void TransposeSample(void);

	protected:
		//bool isInit;
		//doubles distribution;//p0,pn-2,pch,padv as vector watch the order
		//doubles sample;//square matrix of pij as one long array n^2
		doubles pi;
		double padv,pch;
		//int size;//sample size
		//int dim;//sample dimention;
		//CUniformDist *alpha;
		//CChenUFDD *poly;
	};

	//****************************************
	//******  class CGenerator_1Pos   ********
	//модель с одним рекламируемым продуктом для новой модели**
	//****************************************

	class TSCGEN_DLLENTRY CGenerator_1Pos_mine : public CGenerator_1Pos
	{
	public:
		CGenerator_1Pos_mine(const doubles& distribution_, int seed_) :CGenerator_1Pos(distribution_, seed_) {};
		virtual ~CGenerator_1Pos_mine(void) {/*delete poly;delete alpha;*/ }
		CGenerator_1Pos_mine(const CGenerator_1Pos_mine& gen);
		CGenerator_1Pos_mine& operator=(const CGenerator_1Pos_mine& gen);

		//// Generate pij using current settings but new size
		//inline void Generate(int size_){size=size_;Generate();}
		//void SetSize(int _size){size=_size;}
		//// Generate pij using current settings
		//inline void Generate(void);

		////get output
		//const doubles& GetSample(void){return sample;}
		//void SaveSample(const char * fileName,bool isTtranspose=false);
		//const doubles& GetDistribution(void){return distribution;}

		//const doubles& GetP(void) { if (isInit)return pi; throw CGeneratorException(); }

	protected:
		//virtual inline void InitModel(void);
		virtual inline void GenerateOne(void) override;
		//void TransposeSample(void);

	protected:
		//bool isInit;
		//doubles distribution;//p0,pn-2,pch,padv as vector watch the order
		//doubles sample;//square matrix of pij as one long array n^2
		//doubles pi;
		//double padv, pch;
		//int size;//sample size
		//int dim;//sample dimention;
		//CUniformDist *alpha;
		//CChenUFDD *poly;
	};

	//****************************************
	//******  class CGenerator_2Pos   ********
	//модель с двумя положит.рекламами********
	//****************************************

	class TSCGEN_DLLENTRY CGenerator_2Pos: public CGenerator
	{
	public:
		CGenerator_2Pos(const doubles& distribution_,int seed_):CGenerator(distribution_, seed_)
		{sample.resize((distribution.size()-2)*(distribution.size()-2));}//2 - (это число доп. пар. (кроме pi) - 1)
		virtual ~CGenerator_2Pos(void){/*delete poly;delete alpha;*/}
		CGenerator_2Pos(const CGenerator_2Pos& gen);
		CGenerator_2Pos& operator=(const CGenerator_2Pos& gen);

		const doubles& GetP(void){if(isInit)return pi;throw CGeneratorException();}

	protected:
		inline void InitModel(void);
		inline void GenerateOne(void);
		//void TransposeSample(void);

	protected:
		doubles pi;
		double pch, padv, px;
	};


	//****************************************
	//******  class CGenerator_1Neg   ********
	//модель с одной отрицательной рекламой **
	//****************************************
	class TSCGEN_DLLENTRY CGenerator_1Neg: public CGenerator
	{
	public:
		CGenerator_1Neg(const doubles& distribution_,int seed_):CGenerator(distribution_, seed_)
		{sample.resize((distribution.size()-1)*(distribution.size()-1));}
		virtual ~CGenerator_1Neg(void){/*delete poly;delete alpha;*/}
		CGenerator_1Neg(const CGenerator_1Neg& gen);
		CGenerator_1Neg& operator=(const CGenerator_1Neg& gen);
		
		const doubles& GetP(void){if(isInit)return pi;throw CGeneratorException();}
	protected:
		virtual inline void InitModel(void);
		virtual inline void GenerateOne(void);

	protected:
		doubles pi;
		double pch, padv;
	};


	//*******************************************
	//******  class CGenerator_2NegPos   ********
	//модель с двумя рекламами: "-" и "+" *******
	//*******************************************
	class TSCGEN_DLLENTRY CGenerator_2NegPos: public CGenerator
	{
	public:
		CGenerator_2NegPos(const doubles& distribution_,int seed_):CGenerator(distribution_, seed_)
		{sample.resize((distribution.size()-2)*(distribution.size()-2));}
		virtual ~CGenerator_2NegPos(void){/*delete poly;delete alpha;*/}
		CGenerator_2NegPos(const CGenerator_2NegPos& gen);
		CGenerator_2NegPos& operator=(const CGenerator_2NegPos& gen);
		
		const doubles& GetP(void){if(isInit)return pi;throw CGeneratorException();}
	protected:
		virtual inline void InitModel(void);
		virtual inline void GenerateOne(void);

	protected:
		doubles pi;
		double pch, padv, px;
	};

}//namespace TSCGenetator
