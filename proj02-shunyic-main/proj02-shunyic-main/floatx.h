#ifndef FLOATX_H
#define FLOATX_H
typedef unsigned long floatx;

floatx doubleToFloatx(double val,int totBits,int expBits);
// double floatxToDouble(floatx val,int totBits,int expBits);
#endif