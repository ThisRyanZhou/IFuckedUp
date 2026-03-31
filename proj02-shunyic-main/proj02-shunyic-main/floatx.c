#include "floatx.h"
#include "bitFields.h"
#include <math.h>
#include <assert.h>

typedef unsigned long floatx;

floatx doubleToFloatx(double val, int totBits, int expBits) {
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - expBits - 1;
    floatx result = 0;

    // Sign
    if (val < 0.0) {
        setBit(totBits - 1, 1, &result);
        val = -val;
    }

    // Zero
    if (val == 0.0) return result;

    // Decompose val
    int e;
    double frac = frexp(val, &e); // val = frac * 2^e, 0.5 <= frac < 1.0

    int bias = (1 << (expBits - 1)) - 1;
    long exp = e - 1 + bias;

    unsigned long fracInt;

    if (exp <= 0) { // subnormal
        frac = ldexp(frac, e - 1 + fracBits);
        fracInt = (unsigned long)(frac + 0.5); // round to nearest
        exp = 0;
    } else if (exp >= (1 << expBits) - 1) { // overflow, clamp
        exp = (1 << expBits) - 1;
        fracInt = 0;
    } else { // normal
        frac = ldexp(frac - 0.5, fracBits + 1); // remove implicit 1
        fracInt = (unsigned long)(frac + 0.5); // round to nearest
        if (fracInt >= (1UL << fracBits)) { // handle carry
            fracInt = 0;
            exp += 1;
            if (exp >= (1 << expBits) - 1) { // max
                exp = (1 << expBits) - 1;
            }
        }
    }

    // Set exponent and fraction
    setBitFld(fracBits, expBits, exp, &result);
    setBitFld(0, fracBits, fracInt, &result);

    return result;
}
