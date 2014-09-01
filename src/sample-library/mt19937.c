/* 
   A C-program for MT19937: Real number version

   mt19937_generate_random_double() generates one pseudorandom 
   real number (double) which is uniformly distributed on 
   [0,1]-interval, for each call. 
   
   mt19937_seed_rng(seed) set initial values to the working area
   of 624 words. Before mt19937_generate_random_double(), 
   mt19937_seed_rng(seed) must be called once. (seed is any 
   32-bit integer except for 0).

   Coded by Takuji Nishimura, considering the suggestions by 
   Topher Cooper and Marc Rieffel in July-Aug. 1997.

   ------

   This library is free software; you can redistribute it and/or 
   modify it under the terms of the GNU Library General Public     
   License as published by the Free Software Foundation; either    
   version 2 of the License, or (at your option) any later         
   version.     
   
   This library is distributed in the hope that it will be useful, 
   but WITHOUT ANY WARRANTY; without even the implied warranty of  
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            

   See the GNU Library General Public License for more details.    
   You should have received a copy of the GNU Library General      
   Public License along with this library; if not, write to the    
   Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA    
   02111-1307  USA                                                 
   
   Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.       
   Any feedback is very welcome. For any question, comments,       
   see http://www.math.keio.ac.jp/matumoto/emt.html or email       
   matumoto@math.keio.ac.jp                                        
*/

#include "mt19937.h"

unsigned long mt[MT19937_N] = {0};
int mti = MT19937_N + 1;

/* initializing the array with a NONZERO seed */
void mt19937_seed_rng(unsigned long seed)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> mt19937_seed_rng()\n");
#endif

    /* 
       setting initial seeds to mt[N] using
       the generator Line 25 of Table 1 in
       [KNUTH 1981, The Art of Computer Programming 
       Vol. 2 (2nd Ed.), pp102]
    */
    mt[0]= seed & 0xffffffff;
    for (mti = 1; mti < MT19937_N; ++mti)
        mt[mti] = (69069 * mt[mti - 1]) & 0xffffffff;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> mt19937_seed_rng()\n");
#endif
}

double mt19937_generate_random_double()
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> mt19937_generate_random_double()\n");
#endif

    unsigned long y;
    static unsigned long mag01[2] = {0x0, MT19937_MATRIX_A};
    /* mag01[x] = x * MT19937_MATRIX_A  for x=0,1 */

    if (mti >= MT19937_N) 
        { 
            /* generate N words at one time */
            int kk;
            
            /* if mt19937_seed_rng() has not been called, a default initial seed is used */
            if (mti == MT19937_N + 1)   
                mt19937_seed_rng(4357); 

            for (kk = 0;kk < MT19937_N - MT19937_M; ++kk) 
                {
                    y = (mt[kk] & MT19937_UPPER_MASK) | (mt[kk+1] & MT19937_LOWER_MASK);
                    mt[kk] = mt[kk + MT19937_M] ^ (y >> 1) ^ mag01[y & 0x1];
                }
            
            for (;kk < MT19937_N - 1; ++kk) 
                {
                    y = (mt[kk] & MT19937_UPPER_MASK) | (mt[kk + 1] & MT19937_LOWER_MASK);
                    mt[kk] = mt[kk + (MT19937_M - MT19937_N)] ^ (y >> 1) ^ mag01[y & 0x1];
                }
            y = (mt[MT19937_N - 1] & MT19937_UPPER_MASK) | (mt[0] & MT19937_LOWER_MASK);
            mt[MT19937_N - 1] = mt[MT19937_M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

            mti = 0;
        }
  
    y = mt[mti++];
    y ^= MT19937_SHIFT_U(y);
    y ^= MT19937_SHIFT_S(y) & MT19937_MASK_B;
    y ^= MT19937_SHIFT_T(y) & MT19937_MASK_C;
    y ^= MT19937_SHIFT_L(y);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> mt19937_generate_random_double()\n");
#endif

    return (double) y / (unsigned long) 0xffffffff;
}

unsigned long mt19937_generate_random_ulong()
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> mt19937_generate_random_ulong()\n");
#endif

    unsigned long y;
    static unsigned long mag01[2] = {0x0, MT19937_MATRIX_A};
    /* mag01[x] = x * MT19937_MATRIX_A  for x=0,1 */

    if (mti >= MT19937_N) 
        { 
            /* generate N words at one time */
            int kk;
            
            /* if mt19937_seed_rng() has not been called, a default initial seed is used */
            if (mti == MT19937_N + 1)   
                mt19937_seed_rng(4357); 

            for (kk = 0;kk < MT19937_N - MT19937_M; ++kk) 
                {
                    y = (mt[kk] & MT19937_UPPER_MASK) | (mt[kk+1] & MT19937_LOWER_MASK);
                    mt[kk] = mt[kk + MT19937_M] ^ (y >> 1) ^ mag01[y & 0x1];
                }
            
            for (;kk < MT19937_N - 1; ++kk) 
                {
                    y = (mt[kk] & MT19937_UPPER_MASK) | (mt[kk + 1] & MT19937_LOWER_MASK);
                    mt[kk] = mt[kk + (MT19937_M - MT19937_N)] ^ (y >> 1) ^ mag01[y & 0x1];
                }
            y = (mt[MT19937_N - 1] & MT19937_UPPER_MASK) | (mt[0] & MT19937_LOWER_MASK);
            mt[MT19937_N - 1] = mt[MT19937_M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

            mti = 0;
        }
  
    y = mt[mti++];
    y ^= MT19937_SHIFT_U(y);
    y ^= MT19937_SHIFT_S(y) & MT19937_MASK_B;
    y ^= MT19937_SHIFT_T(y) & MT19937_MASK_C;
    y ^= MT19937_SHIFT_L(y);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> mt19937_generate_random_ulong()\n");
#endif

    return y;
}
