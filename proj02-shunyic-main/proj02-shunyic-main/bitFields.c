#include "bitFields.h"
#include <assert.h>

#define BIT(N) (1UL << (N))
#define MASK(n, pos) (((1UL << (n)) - 1) << (pos))
     
int getBit(int bn,unsigned long val){
  assert(bn >= 0 && bn < 64);
  return (val >> bn) & 1;
}
void clearBit(int bn,unsigned long * val){
  assert(bn >= 0 && bn < 64);
  *val &= ~BIT(bn);
}
void setBit(int bn,int new,unsigned long * val){
  assert(bn >= 0 && bn < 64);
  if (new) *val |= (1UL << bn);
  else clearBit(bn,val);
}
unsigned long getBitFld(int bl,int len,unsigned long val){
  assert(bl >= 0 && bl < 64);
  assert(len >= 0 && len + bl - 1 < 64);
  return (val >> bl) & ((1UL << len) - 1);
}
void clearBitFld(int bl,int len,unsigned long * val){
  assert(bl >= 0 && bl < 64);
  assert(len >= 0 && len + bl < 64);
  *val &= ~(MASK(len, bl));
}
void setBitFld(int bl,int len,unsigned long new,unsigned long * val){
  assert(bl >= 0 && bl < 64);
  assert(len >= 0 && len + bl - 1 < 64);
  clearBitFld(bl, len, val);
  new &= ((1UL << len)-1);
  *val |= (new << bl);
}
