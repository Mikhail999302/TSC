#ifndef CHENUFDD_HPP_
#define CHENUFDD_HPP_
#include "../Headers/ChenUFDD.h"

template <typename ValueType> 
Tsc::Generator::CChenUFDD<ValueType>::CChenUFDD(unsigned long seed_,
		const std::vector<ValueType>&p_, int param_) :
			CFiniteDiscreteDist<ValueType>(p_), alpha(seed_), param(param_), r(param_+1)
{
	s=this->p;
	std::partial_sum(s.begin(), s.end(), s.begin());//s_i=sum(p_1+...p_i)
	int i;
	i=0;
	ValueType t=0;
	ValueType revparam=1./param;
	for (int j=0; j<param; ++j)
	{
		while (s[i]<=t)
			i++;
		r[j]=i;
		t+= revparam;
	}
	Next();
}

template <typename ValueType>
ValueType Tsc::Generator::CChenUFDD<ValueType>::Next()
{
	int j=(int)floor(param*alpha.Next());
	int i=r[j];
	while (alpha.LookUp()>=s[i])
		i++;
	curValue=i;
	return curValue;
}

template <typename ValueType>
inline void Tsc::Generator::CChenUFDD<ValueType>::SetDistribution(const std::vector<ValueType>&p_)
{
	this->p=p_;
	s=this->p;
	std::partial_sum(s.begin(), s.end(), s.begin());//s_i=sum(p_1+...p_i)
	int i;
	i=0;
	ValueType t=0;
	SetParam((int)this->p.size());
	ValueType revparam=1./param;
	for (int j=0; j<param; ++j)
	{
		while (s[i]<=t)
			i++;
		r[j]=i;
		t+=revparam;
	}
}

template <typename ValueType>
void Tsc::Generator::CChenUFDD<ValueType>::SetDistribution(const std::vector<ValueType>&p_, int param_)
{
	this->p=p_;
	s=this->p;
	std::partial_sum(s.begin(), s.end(), s.begin());//s_i=sum(p_1+...p_i)
	int i;
	i=0;
	ValueType t=0;
	SetParam(param_);
	ValueType revparam=1./param;
	for (int j=0; j<param; ++j)
	{
		while (s[i]<=t)
			i++;
		r[j]=i;
		t+=revparam;
	}
}
#endif /*CHENUFDD_HPP_*/
