/* ----------------------------------------------------------- */
/*       Вычисление собственных чисел и собственных            */
/*       векторов вещественной симметрической матрицы          */
/*       методом вращения Якоби                                */
/*                     ( 01.12.92 )                            */
/* ----------------------------------------------------------- */

#include <math.h>
#include "stdafx.h"
#include "outdefs.h"

#define sqr(x)  ((x)*(x))

static real range = 1.0e-10,
            sqrt2, anorm, anrmx,
            sinx, cosx, sinx2, cosx2, sincos,
	    x, y, thr;

static real   *rb,   *rb1,
        *rc,   *rc1,
        *ab,   *ac,
        *re;

static long int i, j, l, m, mq, lq, lm, ll, mm, iq, il, im, ilr, imr,
	   ind, ilq, imq;

void eigen (real   * a, real   * r, int n, int mv)
/* -------------------------------------------------------------------- */
/*   a - исходная матрица (верхний треугольник, записанный в одномерный */
/*       массив по столбцам или нижний по строкам), после вычислений на */
/*       диагонали помещаются собственные числа                         */
/*   r - результат вычислений - матрица собственных векторов по столб-  */
/*       цам, размещенная в одномерном массиве                          */
/*   n - размерность матриц a и r                                       */
/*   mv - входной параметр:                                             */
/*        0 - вычисляются собственные числа и собственные вектора       */
/*        1 - вычисляются только собственные числа                      */
/* -------------------------------------------------------------------- */
 {
   sqrt2 = sqrt (2.0);
   re = r+n*n;
   anorm = 0.0;
   ind = 0;

   /* ------ 1. Построение единичной матрицы в r [n*n] ------ */

   if (!mv)
     { memset (r, 0, n*n*sizeof(real));
       for (rb=r;  rb<re;  rb+=n+1)
         * rb = 1.0;
     }

   /* ------  2. Вычисление начальной и конечной норм  ------ */

   for (ab=a+1, i=1;  i<n;  ab++, i++)
     for (j=0; j<i;  j++, ab++)
       anorm += sqr (*ab);
   if (anorm==0.0) goto sort;
   anorm = sqrt2*sqrt (anorm);
   anrmx = anorm*range/n;

   /* ----- 3. Основной цикл по величине квадрата следа ----- */

   for (thr = anorm;  thr > anrmx; )
     { thr /= n;
       for (;; ind = 0)
         { for (l=0;  l<n-1;  l++)
             for (m=l+1;  m<n;  m++)
               {
   /* ---------- 4. Вычисление sin и cos поворота ----------- */
                 mq = m*(m+1)/2;  lq = l*(l+1)/2;  lm = l+mq;
                 if (fabs(a[lm]) < thr) continue;
                 ind = 1;
                 ll = l+lq;  mm = m+mq;
                 x = 0.5*(a[ll]-a[mm]);
                 y = -a[lm]/sqrt(sqr(a[lm])+sqr(x));
                 if (x<0.0) y = -y;
                 sinx = y/sqrt(2.0*(1.0+sqrt(1.0-sqr(y))));
                 sinx2 = sqr(sinx);
                 cosx = sqrt (cosx2 = 1.0-sinx2);
                 sincos = sinx*cosx;
   /* ------- 5. Вращение в плоскости столбцов l и m -------- */
                 ilq = n*l;  imq = n*m;
                 for (i=0;  i<n;  i++)
                   { iq = i*(i+1)/2;
                     if (i!=l && i!=m)
                       { im = (i<m ? i+mq : m+iq);
                         il = (i<l ? i+lq : l+iq);
                         x = a[il]*cosx - a[im]*sinx;
                         a[im] = a[il]*sinx + a[im]*cosx;
                         a[il] = x;
                       }
                     if (!mv)
                       { x = r[ilr = ilq+i]*cosx - r[imr = imq+i]*sinx;
                         r[imr] = r[ilr]*sinx + r[imr]*cosx;
                         r[ilr] = x;
                       }
                   }
                 x = 2.0*a[lm]*sincos;
                 y = a[ll]*cosx2 + a[mm]*sinx2 - x;
                 x += a[ll]*sinx2 + a[mm]*cosx2;
                 a[lm] = (a[ll] - a[mm])*sincos + a[lm]*(cosx2-sinx2);
                 a[ll] = y;  a[mm] = x;
               }
           if (!ind) break;
         }
     }

   sort :
   /* ----- 6. Сортировка собственных чисел и векторов ------ */

   for (ab=a, rb=r, i=1;  i<n;  ab+=++i, rb+=n)
     for (ac=ab+i+1, rc=rb+n, j=i+1;  j<=n;  ac+=++j, rc+=n)
       if (*ab < *ac)
         { x=*ab; *ab = *ac; *ac = x;
           if (!mv)
             for (rb1=rb, rc1=rc, l=0;  l<n;  rb1++, rc1++, l++)
               { x=*rb1; *rb1 = *rc1; *rc1=x; }
         }
 } /* ----- Конец процедуры eigen ----- */
