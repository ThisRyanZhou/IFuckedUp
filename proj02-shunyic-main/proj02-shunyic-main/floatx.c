#include "floatx.h"
#include <stdint.h>
#include <math.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    if (val == 0.0) return 0;

    union {
        double d;
        uint64_t u;
    } uVal;
    uVal.d = val;

    int64_t sign = (uVal.u >> 63) & 1;
    int64_t exp  = (uVal.u >> 52) & 0x7FF;
    uint64_t frac = uVal.u & 0xFFFFFFFFFFFFFULL;

    int64_t doubleBias = 1023;
    int64_t floatxBias = (1 << (expBits-1)) - 1;

    int fracBits = totBits - 1 - expBits;

    floatx result = 0;
    result |= (sign << (totBits - 1));

    if (exp == 0) {
        // subnormal double
        uint64_t fxFrac = frac >> (52 - fracBits);
        result |= fxFrac;
        return result;
    }

    // normal double
    exp -= doubleBias;
    int64_t newExp = exp + floatxBias;

    if (newExp >= (1 << expBits) - 1) {
        // overflow → infinity
        newExp = (1 << expBits) - 1;
        result |= (newExp << fracBits);
        return result;
    }

    if (newExp <= 0) {
        // floatx subnormal
        frac |= 1ULL << 52; // implicit 1
        if (newExp < -fracBits) return result; // underflow → zero
        uint64_t fxFrac = frac >> (52 + 1 - fracBits - newExp);
        result |= fxFrac;
        return result;
    }

    result |= (newExp << fracBits);
    uint64_t fxFrac = frac >> (52 - fracBits); // truncate, no rounding
    result |= fxFrac;

    return result;
}
