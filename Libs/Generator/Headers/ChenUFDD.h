#ifndef CHENUFDD_H_
#define CHENUFDD_H_
/*
 * Most efficient variant for generation of FDD
 */
#include "../Headers/UniformDistribution.h"
#include "../Headers/DihotomyUFDD.h"
namespace Tsc 
{
namespace Generator 
{
//P([ksi]=i)=p_i Chen Method
template <typename ValueType=double>
class CChenUFDD:public CFiniteDiscreteDist<ValueType>
{
	ValueType curValue;
	CUniformDist<ValueType> alpha;
	std::vector<ValueType> s;
	int param;
	std::vector<int> r;
public:
	CChenUFDD(unsigned long seed_,const std::vector<ValueType>& p_, int param_);
	virtual inline ValueType Next();
	virtual inline ValueType LookUp()const
		{return curValue;}
	virtual inline void SetDistribution(const std::vector<ValueType>& p_);
	inline void SetDistribution(const std::vector<ValueType>& p_,int param_);
	void SetParam(int param_)
		{param=param_;r.resize(param);}
	int GetParam()
		{return param;}
	virtual inline void SetSeed(unsigned long int seed_)
		{alpha.SetSeed(seed_);}
	virtual inline unsigned long GetSeed()
		{return alpha.GetSeed();}
	virtual ~CChenUFDD(){}
};
}  // namespace Generator
}  // namespace Tsc
#include "../implementation/ChenUFDD.hpp"
#endif /*CHENUFDD_H_*/
