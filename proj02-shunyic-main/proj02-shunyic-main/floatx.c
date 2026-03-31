#include "floatx.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "bitFields.h"

floatx doubleToFloatx(double val,int totBits,int expBits) {

    // -----------------------------
    // 0. Validate inputs
    // -----------------------------
    assert(totBits >= 3 && totBits <= 64);
    assert(expBits >= 1 && expBits <= totBits - 2);

    int fracBits = totBits - expBits - 1;

    int doubleBias = 1023;
    int floatxBias = (1 << (expBits - 1)) - 1;

    // -----------------------------
    // 1. Extract double fields
    // -----------------------------
    union {
        double d;
        unsigned long u;
    } conv;

    conv.d = val;

    unsigned long sign = (conv.u >> 63) & 1;
    unsigned long exp  = (conv.u >> 52) & 0x7FF;
    unsigned long frac = conv.u & 0xFFFFFFFFFFFFF;

    // -----------------------------
    // 2. Handle special cases
    // -----------------------------

    // Zero
    if (exp == 0 && frac == 0) {
        return sign << (totBits - 1);
    }

    // Inf / NaN
    if (exp == 0x7FF) {
        unsigned long fxExp = (1UL << expBits) - 1;
        unsigned long fxFrac = (frac == 0) ? 0 : 1;

        return (sign << (totBits - 1)) |
               (fxExp << fracBits) |
               fxFrac;
    }

    // -----------------------------
    // 3. Normalize exponent
    // -----------------------------
    int unbiasedExp;

    if (exp == 0) {
        // double subnormal
        unbiasedExp = 1 - doubleBias;
    } else {
        unbiasedExp = exp - doubleBias;
        frac |= (1UL << 52);  // restore implicit 1
    }

    int fxExp = unbiasedExp + floatxBias;

    // -----------------------------
    // 4. Handle overflow
    // -----------------------------
    if (fxExp >= (1 << expBits) - 1) {
        unsigned long fxExpAll1 = (1UL << expBits) - 1;
        return (sign << (totBits - 1)) |
               (fxExpAll1 << fracBits);
    }

    // -----------------------------
    // 5. Handle underflow → subnormal
    // -----------------------------
    if (fxExp <= 0) {

        int shift = 1 - fxExp;

        // too small → zero
        if (shift > 53) {
            return sign << (totBits - 1);
        }

        // shift mantissa
        frac >>= shift;

        unsigned long fxFrac;

        if (52 >= fracBits) {
            fxFrac = frac >> (52 - fracBits);
        } else {
            fxFrac = frac << (fracBits - 52);
        }

        return (sign << (totBits - 1)) | fxFrac;
    }

    // -----------------------------
    // 6. Normal case
    // -----------------------------
    unsigned long fxFrac;

    if (52 >= fracBits) {
        fxFrac = frac >> (52 - fracBits);
    } else {
        fxFrac = frac << (fracBits - 52);
    }

    return (sign << (totBits - 1)) |
           ((unsigned long)fxExp << fracBits) |
           fxFrac;
}
