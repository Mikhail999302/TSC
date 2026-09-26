//Main interface for distribution generation.
#ifndef GENERATOR_DISTRIBUTION_H
#define GENERATOR_DISTRIBUTION_H
namespace Tsc
{
namespace Generator
{
template<class ValType=double>
class CDistribution
{
public:
	//generate new value
	virtual ValType Next()=0;
	//get last generated value
	virtual ValType LookUp() const=0;
	//set new seed usually goes to Uniform distribution
	virtual void SetSeed(unsigned long seed_)=0;
	//Get current start seed
	virtual unsigned long GetSeed()=0;
	virtual ~CDistribution(){}
};
}//namespace Generator
}//namespace Tsc
#endif /*GENERATOR_DISTRIBUTION_H*/
