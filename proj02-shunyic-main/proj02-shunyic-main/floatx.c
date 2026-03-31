#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <assert.h>
#include <stdint.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - 1 - expBits;
    unsigned long result = 0;

    // Handle sign
    if (signbit(val)) {
        setBit(totBits - 1, 1, &result);
        val = -val;
    }

    // Handle special cases
    if (isnan(val)) {
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        setBitFld(0, fracBits, 1, &result);                           // fraction nonzero
        return result;
    }
    if (isinf(val)) {
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        setBitFld(0, fracBits, 0, &result);                           // fraction zero
        return result;
    }
    if (val == 0.0) return 0;

    // Normalized value
    int dblExp;
    double frac = frexp(val, &dblExp); // val = frac * 2^dblExp, 0.5 <= frac < 1
    int biasFx = (1 << (expBits - 1)) - 1;
    long fxExp = dblExp - 1 + biasFx;

    if (fxExp >= (1 << expBits) - 1) {
        // Overflow -> infinity
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result);
        setBitFld(0, fracBits, 0, &result);
        return result;
    } else if (fxExp <= 0) {
        // Subnormal
        frac = ldexp(frac, fxExp); // shift into fraction
        unsigned long fracVal = (unsigned long)(frac * (1UL << fracBits));
        if (fracVal > (1UL << fracBits) - 1) fracVal = (1UL << fracBits) - 1;
        setBitFld(0, fracBits, fracVal, &result);
        setBitFld(fracBits, expBits, 0, &result);
        return result;
    }

    // Normal floatx
    unsigned long fracVal = (unsigned long)((frac - 0.5) * 2 * (1UL << fracBits));
    if (fracVal > (1UL << fracBits) - 1) fracVal = (1UL << fracBits) - 1;
    setBitFld(0, fracBits, fracVal, &result);
    setBitFld(fracBits, expBits, fxExp, &result);

    return result;
}
