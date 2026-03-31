#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <limits.h>  // for CHAR_BIT
#include <assert.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - expBits - 1;  // number of fraction bits
    unsigned long result = 0;

    // Handle special cases first
    if (isnan(val)) {
        // NaN: exponent all 1s, fraction != 0
        setBits(&result, fracBits + expBits, totBits - 1, 1); // exponent all 1s
        setBits(&result, 1, 0, fracBits - 1);                 // fraction != 0 (set LSB)
        return result;
    }

    if (isinf(val)) {
        // Infinity: exponent all 1s, fraction 0
        if (val < 0) result |= 1UL << (totBits - 1);         // sign
        setBits(&result, (1UL << expBits) - 1, fracBits + expBits, totBits - 2);
        return result;
    }

    if (val == 0.0) {
        // Zero: all bits 0 except sign if negative
        if (signbit(val)) result |= 1UL << (totBits - 1);
        return result;
    }

    // Extract sign
    if (val < 0) {
        result |= 1UL << (totBits - 1);
        val = -val;
    }

    // Decompose double
    union {
        double d;
        unsigned long long u;
    } dbl;
    dbl.d = val;

    int dblExp = (int)((dbl.u >> 52) & 0x7FF) - 1023;  // unbiased double exponent
    unsigned long long dblFrac = dbl.u & 0xFFFFFFFFFFFFFULL; // 52-bit fraction

    // Determine floatx bias
    int fxBias = (1 << (expBits - 1)) - 1;
    int fxExp = dblExp + fxBias;

    // Determine max/min exponent for floatx
    int maxExp = (1 << expBits) - 2;   // all 1s reserved for Inf/NaN
    int minExp = 1;

    unsigned long fxFraction = 0;

    // Handle normal floatx
    if (fxExp >= minExp && fxExp <= maxExp) {
        // Normalized: leading 1 implied
        // Map double fraction to floatx fraction
        // Shift double fraction to align with floatx fraction
        int shift = 52 - fracBits;
        if (shift >= 0) {
            fxFraction = (unsigned long)(dblFrac >> shift);
        } else {
            fxFraction = (unsigned long)(dblFrac << (-shift));
        }

        // Set exponent and fraction
        setBits(&result, fxExp, fracBits + expBits, totBits - 2);
        setBits(&result, fxFraction, 0, fracBits - 1);
        return result;
    }

    // Handle subnormal floatx
    if (fxExp < minExp && fxExp > -fracBits) {
        // Shift fraction to produce subnormal
        int shift = (minExp - fxExp);
        fxFraction = ((1ULL << 52) | dblFrac) >> (52 - fracBits + shift);
        setBits(&result, 0, fracBits + expBits, totBits - 2); // exponent all 0
        setBits(&result, fxFraction, 0, fracBits - 1);
        return result;
    }

    // Handle overflow (too large): set to infinity
    if (fxExp > maxExp) {
        setBits(&result, (1UL << expBits) - 1, fracBits + expBits, totBits - 2); // exponent all 1s
        setBits(&result, 0, 0, fracBits - 1);                                     // fraction 0
        return result;
    }

    // Handle underflow (too small): set to zero
    return result; // already 0 with correct sign
}
