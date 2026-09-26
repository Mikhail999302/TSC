#ifndef UNIFORMDISTRIBUTION_H_
#define UNIFORMDISTRIBUTION_H_
/*
 * Main distribution
 * Uniform on [0,1]
 * Multiplicative variant
 */
#include "../Interfaces/distribution.h"
namespace Tsc
{
namespace Generator
{
template<class ValType=double>
class CUniformDist : public CDistribution<ValType>
{
	unsigned long iu, iuhold;
	static const unsigned long mult=663608941l;
	unsigned long seed;
	ValType curValue;
private:
	inline unsigned long SetSuitableSeed(unsigned long seed_);
	void rninit(unsigned long iufir);
	unsigned long rnlast(void);
	ValType rnunif(void);
public:
	CUniformDist(unsigned long seed_)
	{seed = SetSuitableSeed(seed_); Next();}
	virtual inline ValType Next();
	virtual ValType LookUp() const
	{return curValue;}
	virtual void SetSeed(unsigned long int seed_)
	{seed=seed_;rninit(seed);}
	virtual unsigned long GetSeed()
	{return rnlast();}
};
} // namespace Generator
} // namespace Tsc
#include "../implementation/UniformDistribution.hpp"
#endif /*UNIFORMDISTRIBUTION_H_*/
