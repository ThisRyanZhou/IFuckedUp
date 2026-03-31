#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <assert.h>
#include <stdint.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - expBits - 1;
    floatx fx = 0;

    // Handle sign
    if (val < 0.0) {
        setBit(totBits - 1, 1, &fx);
        val = -val;
    }

    // Handle zero
    if (val == 0.0) return fx;

    // Constants for bias
    int bias = (1 << (expBits - 1)) - 1;
    int doubleBias = 1023;

    // Extract double bits
    union {
        double d;
        uint64_t u;
    } uval;
    uval.d = val;

    int64_t doubleExp = ((uval.u >> 52) & 0x7FF) - doubleBias;
    uint64_t doubleFrac = uval.u & 0xFFFFFFFFFFFFFULL; // 52 bits

    long long fxExp = 0;
    uint64_t fxFrac = 0;

    if (doubleExp == 1024) { // Infinity or NaN in double
        fxExp = (1 << expBits) - 1;
        fxFrac = (doubleFrac != 0) ? 1 : 0;
    } else if (doubleExp <= -bias) { // subnormal in floatx
        // shift fraction to fit into fracBits
        double shift = doubleExp + 1 + fracBits;
        if (shift >= 0)
            fxFrac = (uint64_t)((1.0 + (doubleFrac / pow(2.0,52))) * pow(2.0, shift) + 0.5);
        fxExp = 0;
    } else { // normal
        fxExp = doubleExp + bias;
        // shift fraction to fit fracBits
        fxFrac = (uint64_t)((1.0 + (doubleFrac / pow(2.0,52))) * (1ULL << fracBits) - (1ULL << fracBits) + 0.5);
        if (fxFrac == (1ULL << fracBits)) { // rounding overflow
            fxFrac = 0;
            fxExp += 1;
            if (fxExp >= (1 << expBits) - 1) { // infinity
                fxExp = (1 << expBits) - 1;
                fxFrac = 0;
            }
        }
    }

    // Write exponent and fraction
    setBitFld(fracBits, expBits, fxExp, &fx);
    setBitFld(0, fracBits, fxFrac, &fx);

    return fx;
}
