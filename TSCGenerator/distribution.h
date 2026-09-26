//Definition of Distribution Generator
#pragma once

#include "Exports.h"
#include "R32m.h"

namespace TSCGenerator
{
typedef std::vector<double> doubles;

//Basic Class
template<class T> class TSCGEN_DLLENTRY CDistribution
{
public:
	typedef T ValType;

	virtual ValType Next()=0;
	virtual ValType LookUp() const=0;
	virtual void SetSeed(unsigned long int seed_)=0;
	virtual unsigned long int GetSeed()=0;
};

class TSCGEN_DLLENTRY CUniformDist : public CDistribution<double>
{
	unsigned long int seed;
	double curValue;
	inline int SetSuitableSeed(int seed_);
public:

	CUniformDist(int seed_){seed=SetSuitableSeed(seed_);Next();}
	virtual inline double Next();
	virtual double LookUp() const {return curValue;}
	virtual void SetSeed(unsigned long int seed_){seed=seed_;UniformDistribution::rninit(seed);}
	virtual unsigned long int GetSeed(){return UniformDistribution::rnlast();}
};

#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
TSCGEN_DLLENTRYTEMPLATE template class TSCGEN_DLLENTRY std::allocator< double >;
TSCGEN_DLLENTRYTEMPLATE template class TSCGEN_DLLENTRY std::vector< double >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
class TSCGEN_DLLENTRY CFiniteDiscreteDist:public CDistribution<double>
{
protected:
	doubles p;
public:
	CFiniteDiscreteDist(const doubles& p_);
	virtual double Next()=0;
	virtual double LookUp()const=0;
	virtual void SetDistribution(const doubles& p_)=0;
	virtual void SetSeed(unsigned long int seed_)=0;
	virtual unsigned long int GetSeed()=0;
};

//P([ksi]=i)=p_i Dihotomy Method
class TSCGEN_DLLENTRY CDihotomyUFDD:public CFiniteDiscreteDist
{
	double curValue;
	CUniformDist alpha;
	doubles s;
public:
	CDihotomyUFDD(int seed_,const doubles& p_);
	virtual inline double Next();
	virtual inline double LookUp()const{return curValue;};
	virtual inline void SetDistribution(const doubles& p_);
	virtual inline void SetSeed(unsigned long int seed_){alpha.SetSeed(seed_);}
	virtual inline unsigned long int GetSeed(){return alpha.GetSeed();}
	TSCGEN_DLLENTRY friend std::ostream& operator<<(std::ostream & fout, CDihotomyUFDD & obj);
	TSCGEN_DLLENTRY friend std::istream& operator>>(std::istream & fin, CDihotomyUFDD & obj);
};

//P([ksi]=i)=p_i Chen Method
class TSCGEN_DLLENTRY CChenUFDD:public CFiniteDiscreteDist
{
	double curValue;
	CUniformDist alpha;
	std::vector<double> s;
	int param;
	std::vector<int> r;
public:
	CChenUFDD(int seed_,const std::vector<double>& p_,int param_);
	virtual inline double Next();
	virtual inline double LookUp()const{return curValue;};
	virtual inline void SetDistribution(const std::vector<double>& p_);
	inline void SetDistribution(const std::vector<double>& p_,int param_);
	void SetParam(int param_){param=param_;r.resize(param);}
	int GetParam(){return param;}
	virtual inline void SetSeed(unsigned long int seed_){alpha.SetSeed(seed_);}
	virtual inline unsigned long int GetSeed(){return alpha.GetSeed();}
	TSCGEN_DLLENTRY friend std::ostream& operator<<(std::ostream & fout, CChenUFDD & obj);
	TSCGEN_DLLENTRY friend std::istream& operator>>(std::istream & fin, CChenUFDD & obj);
};

TSCGEN_DLLENTRY std::ostream& operator<<(std::ostream & fout, const std::vector<double> & obj);
TSCGEN_DLLENTRY std::istream& operator>>(std::istream & fin, std::vector<double> & obj);
TSCGEN_DLLENTRY std::ostream& operator<<(std::ostream & fout, const std::vector<int> & obj);
TSCGEN_DLLENTRY std::istream& operator>>(std::istream & fin, std::vector<int> & obj);
TSCGEN_DLLENTRY std::ostream& operator<<(std::ostream & fout, CDihotomyUFDD & obj);
TSCGEN_DLLENTRY std::istream& operator>>(std::istream & fin, CDihotomyUFDD & obj);
TSCGEN_DLLENTRY std::ostream& operator<<(std::ostream & fout, CChenUFDD & obj);
TSCGEN_DLLENTRY std::istream& operator>>(std::istream & fin, CChenUFDD & obj);
}//namespace TSCGenerator