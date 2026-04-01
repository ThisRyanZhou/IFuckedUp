#include "floatx.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include "bitFields.h"

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - 1 - expBits;
    int maxExp = (1 << expBits) - 1;

    uint64_t fx = 0;

    // -------------------
    // Handle special cases using math.h
    // -------------------

    uint64_t sign = signbit(val) ? 1 : 0;

    if (isnan(val)) {
        setBitFld(fracBits, expBits, maxExp, &fx);
        setBitFld(0, fracBits, 1, &fx); // any nonzero fraction
        setBit(totBits - 1, sign, &fx);
        return fx;
    }

    if (isinf(val)) {
        setBitFld(fracBits, expBits, maxExp, &fx);
        setBit(totBits - 1, sign, &fx);
        return fx;
    }

    if (val == 0.0) {
        setBit(totBits - 1, sign, &fx);
        return fx;
    }

    // -------------------
    // Extract double bits
    // -------------------

    union {
        double d;
        uint64_t u;
    } u = { val };

    uint64_t dExp = getBitFld(52, 11, u.u);
    uint64_t dFrac = getBitFld(0, 52, u.u);

    int64_t dBias = 1023;
    int64_t fxBias = (1 << (expBits - 1)) - 1;

    int64_t exp;
    uint64_t frac;

    // -------------------
    // Normalize
    // -------------------

    if (dExp == 0) {
        // subnormal double
        frac = dFrac;
        exp = 1 - dBias;
    } else {
        frac = (1ULL << 52) | dFrac; // implicit 1
        exp = (int64_t)dExp - dBias;
    }

    // Re-bias
    exp += fxBias;

    // -------------------
    // Overflow → infinity
    // -------------------
    if (exp >= maxExp) {
        setBitFld(fracBits, expBits, maxExp, &fx);
        setBit(totBits - 1, sign, &fx);
        return fx;
    }

    // -------------------
    // Underflow → subnormal / zero
    // -------------------
    if (exp <= 0) {
        int shift = 1 - exp;

        if (shift >= 53) {
            frac = 0;
        } else {
            frac >>= shift;
        }

        exp = 0;
    }

    // -------------------
    // Adjust fraction size (truncate!)
    // -------------------

    int shift = 52 - fracBits;

    if (shift > 0) {
        frac >>= shift;  // truncate
    } else if (shift < 0) {
        frac <<= (-shift);
    }

    frac &= ((1ULL << fracBits) - 1);

    // -------------------
    // Assemble
    // -------------------

    setBitFld(0, fracBits, frac, &fx);
    setBitFld(fracBits, expBits, exp, &fx);
    setBit(totBits - 1, sign, &fx);

    return fx;
}
