#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <assert.h>
#include <float.h>

typedef unsigned long floatx;

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - expBits - 1;  // 1 bit for sign
    floatx result = 0;

    // Handle NaN
    if (isnan(val)) {
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        setBit(result, 1, &result); // set first fraction bit to 1
        return result;
    }

    // Handle +Inf and -Inf
    if (isinf(val)) {
        if (val < 0) setBit(totBits - 1, 1, &result); // sign bit
        setBitFld(fracBits, expBits, (1UL << expBits) - 1, &result); // exponent all 1s
        return result;
    }

    // Handle zero
    if (val == 0.0) {
        if (signbit(val)) setBit(totBits - 1, 1, &result);
        return result;
    }

    // Get sign
    if (val < 0) {
        setBit(totBits - 1, 1, &result);
        val = -val;
    }

    int e;
    double frac = frexp(val, &e); // val = frac * 2^e, 0.5 <= frac < 1.0

    int bias = (1 << (expBits - 1)) - 1;
    long exp = e - 1 + bias; // biased exponent

    if (exp <= 0) { // subnormal
        frac = ldexp(frac, e - 1 + fracBits); // shift fraction
        exp = 0;
    } else if (exp >= (1 << expBits) - 1) { // overflow -> inf
        exp = (1 << expBits) - 1;
        frac = 0;
    } else {
        frac = ldexp(frac - 0.5, fracBits + 1); // remove implicit 1
    }

    // Encode exponent
    setBitFld(fracBits, expBits, exp, &result);

    // Encode fraction
    unsigned long fracInt = (unsigned long)(frac + 0.5); // round to nearest
    if (fracInt >= (1UL << fracBits)) fracInt = (1UL << fracBits) - 1; // clamp
    setBitFld(0, fracBits, fracInt, &result);

    return result;
}
