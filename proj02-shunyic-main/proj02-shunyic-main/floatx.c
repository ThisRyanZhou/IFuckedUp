#include "floatx.h"
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset

typedef union {
    double d;
    uint64_t u;
} DoubleBits;

floatx doubleToFloatx(double val, int totBits, int expBits) {
    if (val == 0.0) return 0; // Handle zero immediately

    int fracBits = totBits - 1 - expBits;        // fraction field bits
    int doubleExpBits = 11;                      // double has 11 exponent bits
    int doubleFracBits = 52;                     // double has 52 fraction bits
    int64_t doubleBias = (1 << (doubleExpBits - 1)) - 1;

    int64_t bias = (1 << (expBits - 1)) - 1;    // floatx bias

    DoubleBits db;
    db.d = val;

    // Extract double fields
    uint64_t sign = (db.u >> 63) & 0x1;
    uint64_t exp  = (db.u >> doubleFracBits) & 0x7ff;
    uint64_t frac = db.u & 0xfffffffffffff;

    floatx result = 0;

    // Handle special cases
    if (exp == 0x7ff) { // NaN or infinity
        uint64_t f = (frac != 0) ? 1 : 0;
        result = ((uint64_t)sign << (totBits - 1)) |
                 (((1ULL << expBits) - 1) << fracBits) |
                 (f ? (1ULL << (fracBits - 1)) : 0);
        return result;
    }

    if (exp == 0 && frac == 0) { // exact zero
        return (floatx)sign << (totBits - 1);
    }

    // Compute true exponent of double
    int64_t trueExp;
    uint64_t mantissa;

    if (exp == 0) { // subnormal double
        trueExp = 1 - doubleBias;
        mantissa = frac;
    } else { // normal double
        trueExp = exp - doubleBias;
        mantissa = (1ULL << doubleFracBits) | frac; // add implicit 1
    }

    // Adjust exponent to floatx bias
    int64_t fxExp = trueExp + bias;

    if (fxExp >= (1LL << expBits) - 1) { // overflow → infinity
        result = ((uint64_t)sign << (totBits - 1)) |
                 (((1ULL << expBits) - 1) << fracBits);
        return result;
    } else if (fxExp <= 0) { // subnormal floatx
        if (fxExp < -((int64_t)fracBits)) { // too small → zero
            return (floatx)sign << (totBits - 1);
        }
        // shift mantissa to make subnormal
        mantissa = mantissa >> (1 - fxExp);
        fxExp = 0;
    }

    // Truncate or extend fraction to fit floatx
    uint64_t fxFrac;
    if (fracBits >= doubleFracBits + 1) { // extend with 0s
        fxFrac = mantissa << (fracBits - (doubleFracBits + 1));
    } else { // truncate
        fxFrac = mantissa >> ((doubleFracBits + 1) - fracBits);
    }

    // Mask fraction to correct width
    fxFrac &= ((1ULL << fracBits) - 1);

    // Assemble floatx
    result = ((uint64_t)sign << (totBits - 1)) |
             ((uint64_t)fxExp << fracBits) |
             fxFrac;

    return result;
}
