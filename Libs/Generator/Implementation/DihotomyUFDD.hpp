#ifndef DIHOTOMYUFDD_HPP_
#define DIHOTOMYUFDD_HPP_
#include<numeric>
#include "../Headers/DihotomyUFDD.h"
template <typename ValueType>
Tsc::Generator::CDihotomyUFDD<ValueType>::CDihotomyUFDD(unsigned long seed_,const std::vector<ValueType>& p_):
	CFiniteDiscreteDist<ValueType>(p_),alpha(seed_)
{
	SetDistribution(p_);
	Next();
}

template<typename ValueType>
ValueType Tsc::Generator::CDihotomyUFDD<ValueType>::Next()
{
	int i=0,j=(int)this->p.size()-1;
	int k;
	alpha.Next();
	while(i!=j)
	{
		k=(int)floor((i+j)*0.5);
		if(alpha.LookUp()<(ValueType)s[k])j=k;
		else i=k+1;
	}
	curValue=(ValueType)i;
	return curValue;
}

template<typename ValueType>
void Tsc::Generator::CDihotomyUFDD<ValueType>::SetDistribution(const std::vector<ValueType>& p_)
{
	this->p=p_;
	s=this->p;
	std::partial_sum(s.begin(),s.end(),s.begin());//s_i=sum(p_1+...p_i)
}
#endif /*DIHOTOMYUFDD_HPP_*/
