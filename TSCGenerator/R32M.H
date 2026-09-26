#pragma once
/*
 *  random.h - генераторы случайных чисел
 *  (C) Вермос, 1991
 */

namespace UniformDistribution
{
void  rninit   ( unsigned long iufir );
void  rnrest   ( void );
void  rnconst  ( unsigned long iufir );
void  rnconfix ( unsigned nmb );

unsigned long rnfirst (void);
unsigned long rnlast  (void);
unsigned long rnconrd (void);

double  rnunif ( void );
double  rnexp  ( void );
double  rnnorm ( void );

static unsigned long int iu, iuhold, mult=663608941l;
}//namespace UniformDistribution
