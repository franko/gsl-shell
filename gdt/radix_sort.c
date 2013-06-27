
extern void RadixSort11(double *farray, double *sorted, unsigned int elements);

// Radix.cpp: a fast floating-point radix sort demo
//
//   Copyright (C) Herf Consulting LLC 2001.  All Rights Reserved.
//   Use for anything you want, just tell me what you do with it.
//   Code provided "as-is" with no liabilities for anything that goes wrong.
//

typedef int int32;
typedef unsigned int uint32;

// ================================================================================================
// flip a float for sorting
//  finds SIGN of fp number.
//  if it's 1 (negative float), it flips all bits
//  if it's 0 (positive float), it flips the sign only
// ================================================================================================
inline void double_flip(const uint32 msp, const uint32 lsp, uint32* fmsp, uint32* flsp)
{
    const uint32 mask = -(int32) (msp >> 31);
    *fmsp = msp ^ (mask | 0x80000000);
    *flsp = lsp ^ mask;
}

// ================================================================================================
// flip a float back (invert FloatFlip)
//  signed was flipped from above, so:
//  if sign is 1 (negative), it flips the sign bit back
//  if sign is 0 (positive), it flips all bits back
// ================================================================================================
inline void double_flip_inv(const uint32 msp, const uint32 lsp, uint32 *fmsp, uint32 *flsp)
{
    const uint32 mask = (msp >> 31) - 1;
    *fmsp = msp ^ (mask | 0x80000000);
    *flsp = lsp ^ mask;
}

// ---- utils for accessing 11-bit quantities
#define _0(x)   (x & 0x7FF)
#define _1(x)   (x >> 11 & 0x7FF)
#define _2(x)   (x >> 22 )

// ================================================================================================
// Main radix sort
// ================================================================================================
void RadixSort11(double *farray, double *sorted, unsigned int elements)
{
    uint32 i;
    uint32 *sort = (uint32*)sorted;
    uint32 *array = (uint32*)farray;

    // 6 histograms on the stack:
    const uint32 kHist = 2048;
    uint32 b0[kHist * 6];

    uint32 *b1 = b0 + kHist;
    uint32 *b2 = b1 + kHist;
    uint32 *b3 = b2 + kHist;
    uint32 *b4 = b3 + kHist;
    uint32 *b5 = b4 + kHist;

    for (i = 0; i < kHist * 6; i++) {
        b0[i] = 0;
    }

    // 1.  parallel histogramming pass
    //
    for (i = 0; i < elements; i++) {
        uint32 fim, fil;
        double_flip(array[2*i+1], array[2*i], &fim, &fil);
        b0[_0(fil)] ++;
        b1[_1(fil)] ++;
        b2[_2(fil)] ++;
        b3[_0(fim)] ++;
        b4[_1(fim)] ++;
        b5[_2(fim)] ++;
    }
    
    // 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
    {
        uint32 sum0 = 0, sum1 = 0, sum2 = 0;
        uint32 sum3 = 0, sum4 = 0, sum5 = 0;
        uint32 tsum;
        for (i = 0; i < kHist; i++) {

            tsum = b0[i] + sum0;
            b0[i] = sum0 - 1;
            sum0 = tsum;

            tsum = b1[i] + sum1;
            b1[i] = sum1 - 1;
            sum1 = tsum;

            tsum = b2[i] + sum2;
            b2[i] = sum2 - 1;
            sum2 = tsum;

            tsum = b3[i] + sum3;
            b3[i] = sum3 - 1;
            sum3 = tsum;

            tsum = b4[i] + sum4;
            b4[i] = sum4 - 1;
            sum4 = tsum;

            tsum = b5[i] + sum5;
            b5[i] = sum5 - 1;
            sum5 = tsum;
        }
    }

    // byte 0: floatflip entire value, read/write histogram, write out flipped
    for (i = 0; i < elements; i++) {

        uint32 fim, fil;
        double_flip(array[2*i+1], array[2*i], &fim, &fil);
        uint32 pos = _0(fil);
        uint32 j = ++b0[pos];
        sort[2*j  ] = fil;
        sort[2*j+1] = fim;
    }

    // byte 1: read/write histogram, copy
    //   sorted -> array
    for (i = 0; i < elements; i++) {
        uint32 sim = sort[2*i+1];
        uint32 sil = sort[2*i];
        uint32 pos = _1(sil);
        uint32 j = ++b1[pos];
        array[2*j  ] = sil;
        array[2*j+1] = sim;
    }

    // byte 2: read/write histogram, copy & flip out
    //   array -> sorted
    for (i = 0; i < elements; i++) {
        uint32 aim = array[2*i+1];
        uint32 ail = array[2*i];
        uint32 pos = _2(ail);
        // pf2(array);
        uint32 j = ++b2[pos];
        sort[2*j  ] = ail; // IFloatFlip(ai);
        sort[2*j+1] = aim; // IFloatFlip(ai);
    }

    // byte 3: floatflip entire value, read/write histogram, write out flipped
    for (i = 0; i < elements; i++) {

        uint32 sim = sort[2*i+1];
        uint32 sil = sort[2*i];
        uint32 pos = _0(sim);
        uint32 j = ++b3[pos];
        array[2*j  ] = sil;
        array[2*j+1] = sim;
    }

    // byte 4: read/write histogram, copy
    //   sorted -> array
    for (i = 0; i < elements; i++) {
        uint32 aim = array[2*i+1];
        uint32 ail = array[2*i];
        uint32 pos = _1(aim);
        uint32 j = ++b4[pos];
        sort[2*j  ] = ail;
        sort[2*j+1] = aim;
    }

    // byte 5: read/write histogram, copy & flip out
    //   array -> sorted
    for (i = 0; i < elements; i++) {
        uint32 sim = sort[2*i+1];
        uint32 sil = sort[2*i];
        uint32 pos = _2(sim);
        uint32 j = ++b5[pos];
        double_flip_inv(sim, sil, &sim, &sil);
        array[2*j  ] = sil;
        array[2*j+1] = sim;
    }
}
