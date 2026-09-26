#ifndef DIHOTOMYUFDD_H_
#define DIHOTOMYUFDD_H_
/*
 * Simple variant of FDD generation
 */
#include "../Interfaces/FiniteDiscreteDist.h"
namespace Tsc 
{
namespace Generator
{
//P([ksi]=i)=p_i Dihotomy Method
template <typename ValueType=double>
class CDihotomyUFDD:public CFiniteDiscreteDist<ValueType>
{
	ValueType curValue;
	CUniformDist<ValueType> alpha;
	std::vector<ValueType> s;
public:
	CDihotomyUFDD(unsigned long seed_,const std::vector<ValueType>& p_);
	virtual inline ValueType Next();
	virtual inline ValueType LookUp()const
			{return curValue;}
	virtual inline void SetDistribution(const std::vector<ValueType>& p_);
	virtual inline void SetSeed(unsigned long seed_)
			{alpha.SetSeed(seed_);}
	virtual inline unsigned long GetSeed()
			{return alpha.GetSeed();}
	virtual ~CDihotomyUFDD(){}
};
}// namespace Generation
}// namespace Tsc
#include "../Implementation/DihotomyUFDD.hpp"
#endif /*DIHOTOMYUFDD_H_*/
