#include "floatx.h"
#include <assert.h>
#include <limits.h> // for CHAR_BIT - number of bits per byte
#include <math.h> // for isinf and isnan
#include "bitFields.h"

floatx doubleToFloatx(double val,int totBits,int expBits) {

/*-----------------------------------------------------------------------------------------------
	From the README: 	At a high level, doing the conversion requires several manipulations:
	1. Extracting the sign bit from the double value, and inserting it into the floatx value at
		the correct position.
	2. Extract the biased exponent from the double value.
			Check to see if the double value is sub-normal.
			Check to make sure the floatx exponent won't overflow or underflow.
			                If so, handle as a special case.
			If not, rebias the exponent using the floatx bias (which depends on the number of exponent bits)
			and write the result to the correct location in the floatx result.
	3. Extract the fraction bits from the double value.
			Determine how many bits are available for the fraction in the floatx value,
			and truncate or extend the original value,
			and write the resulting bits to the floatx result.
	4. Handle special cases, such as infinity, or not-a-number.
	5. Handle the cases where the floatx value is subnormal.
	                Adjust the fraction bits as needed.
	6. Return the floatx result.
----------------------------------------------------------------------------------------------------*/

	// First, make some assertions to ensure the totBits and expBits parameters are OK
	// Then, implement the algorithm

	return 0; // Remove this when you are done.
}
