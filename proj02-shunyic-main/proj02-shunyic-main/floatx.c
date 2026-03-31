#include "floatx.h"
#include <math.h>
#include <stdint.h>

floatx doubleToFloatx(double val, int totBits, int expBits) {
    if (totBits < 3 || expBits < 1 || expBits > totBits-2) return 0;

    int fracBits = totBits - 1 - expBits;
    uint64_t fx = 0;

    // Handle special cases
    if (isnan(val)) return 1ULL << (fracBits);          // any NaN pattern
    if (isinf(val)) return ((val < 0) ? (1ULL << (totBits-1)) : 0) | (((1ULL << expBits)-1) << fracBits);
    if (val == 0.0) return 0;

    // Determine sign
    uint64_t sign = (val < 0) ? 1ULL : 0ULL;
    if (val < 0) val = -val;

    // Extract double bits
    union {
        double d;
        uint64_t u;
    } u = { val };

    uint64_t doubleExp = (u.u >> 52) & 0x7FF;
    uint64_t doubleFrac = u.u & 0xFFFFFFFFFFFFF;

    int64_t doubleBias = 1023;
    int64_t fxBias = (1 << (expBits-1)) - 1;

    int64_t exp;
    uint64_t frac;

    if (doubleExp == 0) {
        // subnormal in double
        exp = 0;
        frac = doubleFrac;
    } else {
        // normal number: add implicit 1
        frac = (1ULL << 52) | doubleFrac;
        exp = (int64_t)doubleExp - doubleBias + fxBias;
    }

    // Handle overflow/underflow
    if (exp >= (1 << expBits) - 1) {
        // infinity
        exp = (1 << expBits) - 1;
        frac = 0;
    } else if (exp <= 0) {
        // subnormal in floatx
        if (exp < -fracBits) {
            // too small → zero
            exp = 0;
            frac = 0;
        } else {
            // shift fraction right to create subnormal
            frac = frac >> (1 - exp);
            exp = 0;
        }
    }

    // Truncate or extend fraction to fit fracBits
    if (fracBits < 52 + 1) {
        int shift = (52 + 1) - fracBits;
        frac = frac >> shift;
    } else if (fracBits > 52 + 1) {
        frac = frac << (fracBits - (52 + 1));
    }

    // Compose floatx
    fx = (sign << (totBits-1)) | (exp << fracBits) | (frac & ((1ULL << fracBits)-1));
    return fx;
}
