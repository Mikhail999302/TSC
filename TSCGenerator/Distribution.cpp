#pragma once

#include "Distribution.h"
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////
//uniform Distribution
inline int TSCGenerator::CUniformDist::SetSuitableSeed(int seed_)
{
	UniformDistribution::rninit((unsigned long)seed_);
	return seed_;
}

inline double TSCGenerator::CUniformDist::Next()
{
	curValue=UniformDistribution::rnunif();
	return curValue;
}

////////////////////////////////////////////////////////////////////////////////////////////
//Finite Discrete Distrbution
TSCGenerator::CFiniteDiscreteDist::CFiniteDiscreteDist(const std::vector<double>& p_):
	p(p_)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
//DihotomyUFDD
TSCGenerator::CDihotomyUFDD::CDihotomyUFDD(int seed_,const std::vector<double>& p_):
	alpha(seed_),CFiniteDiscreteDist(p_)
{
	SetDistribution(p_);
	Next();
}

inline double TSCGenerator::CDihotomyUFDD::Next()
{
	int i=0,j=(int)p.size()-1;
	int k;
	alpha.Next();
	while(i!=j)
	{
		k=(int)floor((i+j)*0.5);
		if(alpha.LookUp()<(double)s[k])j=k;
		else i=k+1;
	}
	curValue=i;
	return curValue;
}

inline void TSCGenerator::CDihotomyUFDD::SetDistribution(const std::vector<double>& p_)
{
	p=p_;
	s=p;
	std::partial_sum(s.begin(),s.end(),s.begin());//s_i=sum(p_1+...p_i)
}

//////////////////////////////////////////////////////////////////////////////////////////////
//ChenUFDD

TSCGenerator::CChenUFDD::CChenUFDD(int seed_,const std::vector<double>&p_,int param_):
	alpha(seed_),CFiniteDiscreteDist(p_),param(param_),r(param_+1)
{
	s=p;
	std::partial_sum(s.begin(),s.end(),s.begin());//s_i=sum(p_1+...p_i)
	int i;
	i=0;
	double t=0;
	double revparam=1./param;
	for(int j=0;j<param;++j)
	{
		while(s[i]<=t)i++;
		r[j]=i;
		t+=revparam;
	}
	Next();
}

inline double TSCGenerator::CChenUFDD::Next()
{
	int j=(int)floor(param*alpha.Next());
	int i=r[j];
	while(alpha.LookUp()>=s[i])i++;
	curValue=i;
	return curValue;
}

inline void TSCGenerator::CChenUFDD::SetDistribution(const std::vector<double>&p_)
{
	p=p_;
	s=p;
	std::partial_sum(s.begin(),s.end(),s.begin());//s_i=sum(p_1+...p_i)
	int i;
	i=0;
	double t=0;
	SetParam((int)p.size());
	double revparam=1./param;
	for(int j=0;j<param;++j)
	{
		while(s[i]<=t)i++;
		r[j]=i;
		t+=revparam;
	}
}

inline void TSCGenerator::CChenUFDD::SetDistribution(const std::vector<double>&p_,int param_)
{
	p=p_;
	s=p;
	std::partial_sum(s.begin(),s.end(),s.begin());//s_i=sum(p_1+...p_i)
	int i;
	i=0;
	double t=0;
	SetParam(param_);
	double revparam=1./param;
	for(int j=0;j<param;++j)
	{
		while(s[i]<=t)i++;
		r[j]=i;
		t+=revparam;
	}
}
TSCGEN_DLLENTRY std::ostream& TSCGenerator::operator<<(std::ostream & fout, const std::vector<double> & obj)
{
	fout<<(unsigned int)obj.size()<<std::endl;
	for(std::vector<double>::const_iterator p=obj.begin();p!=obj.end();++p)
	{
		fout<<*p<<" ";
	}
	return fout;
}

TSCGEN_DLLENTRY std::istream& TSCGenerator::operator>>(std::istream & fin, std::vector<double> & obj)
{
	unsigned int size;
	double tmp;
	fin>>size;
	obj.clear();
	obj.reserve(size);
	for(int i=size;i-->0;)
	{
		fin>>tmp;
		obj.push_back(tmp);
	}
	return fin;
}

TSCGEN_DLLENTRY std::ostream& TSCGenerator::operator<<(std::ostream & fout, const std::vector<int> & obj)
{
	fout<<(unsigned int)obj.size()<<std::endl;
	for(std::vector<int>::const_iterator p=obj.begin();p!=obj.end();++p)
	{
		fout<<*p<<" ";
	}
	return fout;
}

TSCGEN_DLLENTRY std::istream& TSCGenerator::operator>>(std::istream & fin, std::vector<int> & obj)
{
	unsigned int size;
	int tmp;
	fin>>size;
	obj.clear();
	obj.reserve(size);
	for(int i=size;i-->0;)
	{
		fin>>tmp;
		obj.push_back(tmp);
	}
	return fin;
}

TSCGEN_DLLENTRY std::ostream& TSCGenerator::operator<<(std::ostream & fout, CDihotomyUFDD & obj)
{
	unsigned long int seed=obj.GetSeed();
	fout<<obj.p<<std::endl;
	fout<<obj.s<<std::endl;
	fout<<obj.curValue<<std::endl;
	fout<<seed<<std::endl;
	return fout;
}

TSCGEN_DLLENTRY std::istream& TSCGenerator::operator>>(std::istream & fin, CDihotomyUFDD & obj)
{
	unsigned long int seed;
	fin>>obj.p;
	fin>>obj.s;
	fin>>obj.curValue;
	fin>>seed;
	obj.SetSeed(seed);
	return fin;
}

TSCGEN_DLLENTRY std::ostream& TSCGenerator::operator<<(std::ostream & fout, CChenUFDD & obj)
{
	unsigned long int seed=obj.GetSeed();
	fout<<obj.p<<std::endl;
	fout<<obj.s<<std::endl;
	fout<<obj.r<<std::endl;
	fout<<obj.param<<std::endl;
	fout<<obj.curValue<<std::endl;
	fout<<seed<<std::endl;
	return fout;
}

TSCGEN_DLLENTRY std::istream& TSCGenerator::operator>>(std::istream & fin, CChenUFDD & obj)
{
	unsigned long int seed;
	fin>>obj.p;
	fin>>obj.s;
	fin>>obj.r;
	fin>>obj.param;
	fin>>obj.curValue;
	fin>>seed;
	obj.SetSeed(seed);
	return fin;
}