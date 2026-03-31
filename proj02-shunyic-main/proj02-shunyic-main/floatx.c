#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <limits.h>
#include <assert.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    unsigned long result = 0;

    // --- Step 1: Sign bit ---
    if (val < 0) {
        setBitFld(totBits - 1, 1, 1, &result); // MSB is sign
        val = -val;
    }

    // --- Step 2: Handle special cases: 0, Inf, NaN ---
    if (val == 0.0) return result; // all bits already 0

    if (isinf(val)) {
        // exponent all 1s, fraction 0
        setBitFld(totBits - 2, expBits, (1UL << expBits) - 1, &result);
        setBitFld(0, totBits - 1 - expBits, 0, &result);
        return result;
    }

    if (isnan(val)) {
        // exponent all 1s, fraction != 0
        setBitFld(totBits - 2, expBits, (1UL << expBits) - 1, &result);
        setBitFld(0, totBits - 1 - expBits, 1, &result); // simple NaN
        return result;
    }

    // --- Step 3: Convert double to floatx ---
    int fracBits = totBits - 1 - expBits;
    int doubleExp;
    double frac = frexp(val, &doubleExp); // val = frac * 2^doubleExp, 0.5 <= frac < 1

    // --- Step 4: Re-bias exponent ---
    int biasFloatx = (1 << (expBits - 1)) - 1;
    int expFloatx = doubleExp - 1 + biasFloatx; // frexp returns 0.5 <= frac < 1

    if (expFloatx >= (1 << expBits) - 1) { // overflow → Inf
        setBitFld(totBits - 2, expBits, (1UL << expBits) - 1, &result);
        setBitFld(0, fracBits, 0, &result);
        return result;
    } else if (expFloatx <= 0) {
        // subnormal: shift fraction
        frac = ldexp(frac, expFloatx - 1); // scale fraction
        expFloatx = 0;
    }

    // --- Step 5: Fraction bits ---
    unsigned long fracBitsVal = (unsigned long)(frac * (1UL << fracBits));
    if (fracBitsVal >= (1UL << fracBits)) fracBitsVal = (1UL << fracBits) - 1; // clamp

    // --- Step 6: Set exponent and fraction ---
    setBitFld(totBits - 2, expBits, expFloatx, &result);
    setBitFld(0, fracBits, fracBitsVal, &result);

    return result;
}
