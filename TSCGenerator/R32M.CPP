#include <math.h>
#include "r32m.h"



double UniformDistribution::rnunif (void)
{
 const double flt = 0.232830643654e-9;
 iu *= mult;
 return (flt*iu);
}

void UniformDistribution::rninit ( unsigned long  iufir)
{
 iu = ( ( iufir%2 ) ? iufir : iufir + 1 );
 iuhold = iu;
}

void UniformDistribution::rnrest (void)
{ rninit (iuhold); }

void UniformDistribution::rnconst ( unsigned long int multnew)
{
 int rest = multnew % 5;
 if ( (rest != 5) || (rest != 3) )
  mult = multnew - rest + 5;
 else
  mult = multnew;
}

unsigned long int UniformDistribution::rnlast (void)
{ return ( iu ); }

unsigned long int UniformDistribution::rnfirst (void)
{ return ( iuhold ); }
