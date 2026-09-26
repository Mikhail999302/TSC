#ifndef UNIFORMDISTRIBUTION_HPP_
#define UNIFORMDISTRIBUTION_HPP_
#include <math.h>
#include "../Headers/UniformDistribution.h"

///////////////////////////////////////////////////////////////////////////////////////////
//uniform Distribution
template <typename ValType>
inline unsigned long Tsc::Generator::CUniformDist<ValType>::SetSuitableSeed(unsigned long seed_)
{
	rninit(seed_);
	return seed_;
}

template <typename ValType>
inline ValType Tsc::Generator::CUniformDist<ValType>::Next()
{
	return curValue = rnunif();
}

template <typename ValType>
inline void Tsc::Generator::CUniformDist<ValType>::rninit(unsigned long iufirst)
{
	iu = ( (iufirst%2 ) ? iufirst : iufirst + 1 );
	iuhold = iu;
}

template <typename ValType>
unsigned long Tsc::Generator::CUniformDist<ValType>::rnlast(void)
{
	return iu;
}

template <typename ValType>
ValType Tsc::Generator::CUniformDist<ValType>::rnunif(void)
{
	const double flt = 0.232830643654e-9;
	iu *= mult;
	return (flt*iu);
}

#endif /*UNIFORMDISTRIBUTION_HPP_*/
