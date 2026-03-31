#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <assert.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    unsigned long result = 0;
    int fracBits = totBits - 1 - expBits;

    // --- Sign bit ---
    if (val < 0) {
        setBitFld(totBits - 1, 1, 1, &result);
        val = -val;
    }

    // --- Handle special cases ---
    if (val == 0.0) return result;
    if (isinf(val)) {
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result);
        return result;
    }
    if (isnan(val)) {
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result);
        if (fracBits > 0) setBitFld(0, 1, 1, &result); // fraction != 0
        return result;
    }

    // --- Normalize ---
    int doubleExp;
    double frac = frexp(val, &doubleExp); // val = frac * 2^doubleExp, 0.5 <= frac < 1

    int biasFloatx = (1 << (expBits - 1)) - 1;
    int expFloatx = doubleExp - 1 + biasFloatx;

    // --- Handle overflow / underflow ---
    if (expFloatx >= (1 << expBits) - 1) { // Infinity
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result);
        return result;
    } else if (expFloatx <= 0) { // Subnormal
        frac = ldexp(frac, expFloatx - 1);
        expFloatx = 0;
    }

    // --- Fraction ---
    unsigned long fracVal = (unsigned long)(frac * (1UL << fracBits) + 0.5); // round nearest
    if (fracVal >= (1UL << fracBits)) fracVal = (1UL << fracBits) - 1;

    // --- Set exponent and fraction ---
    if (expBits > 0) setBitFld(fracBits, expBits, expFloatx, &result);
    if (fracBits > 0) setBitFld(0, fracBits, fracVal, &result);

    return result;
}
