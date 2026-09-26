#ifndef GENERATOR_FINITEDISCRETEDIST_H_
#define GENERATOR_FINITEDISCRETEDIST_H_
/*
 * Generation of finite discrete distribution interface. Has two realizations Chen, Dihotomy
 */
#include <vector>
namespace Tsc 
{
namespace Generator 
{
template <typename ValueType=double>
class CFiniteDiscreteDist:public CDistribution<ValueType>
{
protected:
	std::vector<ValueType> p;
public:
	CFiniteDiscreteDist(const std::vector<ValueType>& p_):p(p_){}
	virtual ValueType Next()=0;
	virtual ValueType LookUp()const=0;
	virtual void SetDistribution(const std::vector<ValueType>& p_)=0;
	virtual void SetSeed(unsigned long seed_)=0;
	virtual unsigned long GetSeed()=0;
};
}  // namespace Generator
}  // namespace Tsc
#endif /*GENERATOR_FINITEDISCRETEDIST_H_*/
