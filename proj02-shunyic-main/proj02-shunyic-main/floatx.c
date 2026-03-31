#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <limits.h>
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
        // All exponent 1s, fraction nonzero (we use 1)
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        setBit(result, 0, &result); // any nonzero fraction
        setBitFld(0, fracBits, 1, &result); // fraction=1
        return result;
    }

    if (isinf(val)) {
        // All exponent 1s, fraction 0
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        setBitFld(0, fracBits, 0, &result);
        return result;
    }

    if (val == 0.0) {
        // zero
        return 0;
    }

    // Normalized value
    int dblExp;
    double frac = frexp(val, &dblExp); // val = frac * 2^dblExp, 0.5 <= frac < 1
    int bias64 = 1023;  // double exponent bias
    int biasFx = (1 << (expBits - 1)) - 1;
    long fxExp = dblExp - 1 + biasFx;

    if (fxExp >= (1 << expBits) - 1) {
        // Overflow -> infinity
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        setBitFld(0, fracBits, 0, &result);                           // fraction=0
        return result;
    } else if (fxExp <= 0) {
        // Subnormal number
        frac = ldexp(frac, fxExp); // shift into fraction
        unsigned long fracVal = (unsigned long)(frac * (1UL << fracBits) + 0.5);
        setBitFld(0, fracBits, fracVal, &result);
        setBitFld(fracBits, expBits, 0, &result);
        return result;
    }

    // Normal floatx
    unsigned long fracVal = (unsigned long)((frac - 0.5) * 2 * (1UL << fracBits) + 0.5);
    setBitFld(0, fracBits, fracVal, &result);
    setBitFld(fracBits, expBits, fxExp, &result);

    return result;
}
