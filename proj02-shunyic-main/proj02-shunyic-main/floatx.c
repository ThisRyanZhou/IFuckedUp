#include "floatx.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "bitFields.h"

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits > 1);
    assert(expBits > 0 && expBits < totBits);

    if (val == 0.0) return 0;

    union {
        double d;
        unsigned long u;
    } uVal;
    uVal.d = val;

    int fracBits = totBits - 1 - expBits;

    // Extract fields using bitFields helpers
    int sign = getBitFld(63, 1, uVal.u);
    int exp  = getBitFld(52, 11, uVal.u);
    unsigned long frac = getBitFld(0, 52, uVal.u);

    int doubleBias = 1023;
    int floatxBias = (1 << (expBits - 1)) - 1;

    floatx result = 0;

    // Set sign
    setBitFld(totBits - 1, 1, sign, &result);

    if (exp == 0) {
        // subnormal double
        unsigned long fxFrac = frac >> (52 - fracBits);
        setBitFld(0, fracBits, fxFrac, &result);
        return result;
    }

    int trueExp = exp - doubleBias;
    int newExp  = trueExp + floatxBias;

    if (newExp >= (1 << expBits) - 1) {
        // overflow → infinity
        setBitFld(fracBits, expBits, (1 << expBits) - 1, &result);
        return result;
    }

    if (newExp <= 0) {
        // subnormal floatx
        frac |= BIT(52); // restore implicit 1

        if (newExp < -fracBits) {
            // underflow → zero
            return result;
        }

        unsigned long fxFrac =
            frac >> (52 + 1 - fracBits - newExp);

        setBitFld(0, fracBits, fxFrac, &result);
        return result;
    }

    // normal case
    setBitFld(fracBits, expBits, newExp, &result);

    unsigned long fxFrac = frac >> (52 - fracBits);
    setBitFld(0, fracBits, fxFrac, &result);

    return result;
}
