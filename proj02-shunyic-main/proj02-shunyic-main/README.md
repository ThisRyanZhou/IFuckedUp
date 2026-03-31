# proj02 - Extended Floating Point

## Introduction

The goal of this assignment is to:

- Learn about IEEE floating point numbers
- Learn about manipulating bits ("bit twiddling") in C
- Understand number type conversions

We have discussed the internal representation of IEEE floating point numbers (float and double) in lecture.

When the IEEE organization made the floating point (754) specification, they defined the bit width of all three fields, sign, exponent, and fraction by trading off data range versus precision. The bit widths chosen by IEEE are very convenient, but sometimes, it would be nice to be more flexible about the bit widths of the fields. For example, you might be working on an application where data precision is very important, but a big range is not required. In this case, you might want more fraction bits, and fewer exponent bits.

This project asks you implement a generalized floating point number capability; one which uses an arbitrary number of bits, `expBits`, to represent the exponent of a floating point number, and an arbitrary number of bits to represent the fractional part of the number.  We will call this generalized floating point number a "floatx" number.

## Floatx Specifications

A floatx number is a parameterized floating point number in which the total bit width and the bit-width of the exponent field are the two parameters. For example one instance of a floatx number might have a total width of 12 bits and an exponent field that is 5 bits wide; floatx(12,5). Since floatx numbers always have a sign bit, then the width of the fraction field must be total bits - (expoent field size + 1). In a float(12,5) value, the fraction field must be 12-(5+1)=6 bits wide.

In concrete terms, we will keep a floatx number in a 64 bit unsigned integer (an unsigned long). However, we will only be using the rightmost bits (bit positions totBits-1 to zero) for our floatx value.

For example, a floatx(12,5) number with 12 total bits and 5 exponent bits would use the following bits in a long unsigned value...

<table>
<tr><td>63</td><td>...</td><td>12</td><td>11</td><td>10</td><td>9</td><td>8</td><td>7</td><td>6</td><td>5</td><td>4</td><td>3</td><td>2</td><td>1</td><td>0</td></tr>
<tr><th colspan=3>Padding</th><th>S</th><th colspan=5>Exponent</th><th colspan=6>Fraction</th><tr>
</table>

Therefore, the maximum total bits for a floatx number is 64. There must be at least 1 exponent bit and at least 1 fraction bit, so the smallest floatx number is floatx(3,1). The floatx number with the biggest exponent field is floatx(64,62), and the floatx number with the biggest fraction field is floatx(64,1).

Notice that the IEEE `float` specification is floatx(32,8), and the IEEE `double` specification is floatx(64,11).

Other than changing the number of bits for the sub-fields, all the rest of the IEEE conventions for floating point numbers must be followed:

- The left-most (most significant) bit is a sign bit; 0 for positive, 1 for negative
- The next *expBits* bits represent an exponent field, biased by $2^{(expBits-1)} - 1$
- Exponent bits of all ones represents special values like infinity (if fraction bits are all zero), or "Not a Number" (NaN) (if fraction bits are not all zero.)
- The remaining bits represent a fraction field, `F`.  For most numbers, there is an implied "1." in front of the fraction so we interpret it as $1.F$.  
- If the exponent field and the fraction field are all zeroes, that represents the number 0.0.
- If the exponent field is all zeroes, but the fraction field is not zero, then we treat the number as a "de-normalized" number, and assume it has the value of $0.F \times 2^{-bias+1}$
- When increasing the number of fraction bits, it is valid to pad on the right with 0 bits.
- When decreasing the number of fraction bits, truncate the rightmost bits. This has the effect of rounding towards zero which the simplest of the valid rounding strategies for floating point numbers.

## Implementing Floatx Numbers

Obviously, working with floatx numbers will require a lot of bit manipulation. The purpose of Lab04 was to provide bit manipulation tools that you can use in this project. I have found that using bitFields.c and bitFields.h from Lab04 significantly reduces the time required to complete this project, and have included a correct implementation of bitFields.c in this repo.

We will create a floatx number from a double-precision floating point value. The Wikipedia [Double-precision floating-point](https://en.wikipedia.org/wiki/Double-precision_floating-point_format) article does a good job of describing the details of a double value.

For the project, you will need to implement a single function, the `doubleToFloatx` function that takes a double precision floating point input argument, along with the total number of bits and the number of exponent bits for the floatx version of the value. The function needs to return the floatx version of the number.

At a high level, doing the conversion requires several manipulations:

1. Extracting the sign bit from the double value, and inserting it into the floatx value at the correct position.
2. Extract the biased exponent from the double value. Check to see if the double value is sub-normal. If so, handle it (as discussed during lecture). Check to make sure the floatx exponent won't overflow (or underflow). If so, handle as a special case. If not, re-bias the exponent using the floatx bias (which depends on the number of exponent bits) and write the result to the correct location in the floatx result.
3. Extract the fraction bits from the double value. Determine how many bits are available for the fraction in the floatx value, and truncate or extend the original value, and write the resulting bits to the floatx result.
4. Handle special cases, such as 0.0, infinity, or not-a-number.
5. Return the floatx result.

## Provided to you

The proj02 repository already contains some infrastructure, in addition to the bit field utilities in bitFields.c, to get you started, as follows:

### floatx.h and floatx.c

The floatx.h file declares the single floatx function, `doubleToFloatx`, we will be working on for this project.

[It would also make sense to work on a `floatxToDouble` function if this were a real library, but doing so adds more work to an already difficult project, and doesn't contribute much more to the educational value of the project, so I've left that out.]

 The floatx.h file also contains a typedef statement to define floatx as an unsigned long (64 bit) integer. You should not have to edit floatx.h.

Your main job will be to complete the definition of the `doubleToFloatx` function in floatx.c. This is the function that takes a double precision floating point value, and returns the floatx representation of that value.

### testFloatx.c

This is a program that uses command line arguments to specify 1) the total number of bits, and 2) the number of exponent bits in a floatx format. It then reads standard input, looking for a double precision floating point value. For each floating point value found in standard input:

- the testFloatx program invokes `doubleToFloatx` with the value read from standard input, and the total bits and exponent size specified in the command line arguments.
- prints the original value, the floatx specification, and the floatx value in hexadecimal.

Any non-whitespace characters from standard input that cannot be translated to a double will cause the program to stop.

We will use UNIX redirection to read and write from files (see the Wikipedia [Redirection(computing)](https://en.wikipedia.org/wiki/Redirection_(computing)) article for more details.) Please do not edit the code in testFloatx.c. There is no need to use file I/O functions to read or write files for this project.

### Makefile

I have provided a Makefile with several targets:

- `clean` Removes all the files Makefile can create
- `testFloatx` - creates the testFloatx executable from testFloatx.c, floatx.c. Note that there is a commented version of this target in the file that manages the inclusion of bitFields.c and bitFields.h from the Lab04 assignment.
- `gdb` - A target to run gdb on the testFloatx executable. This will automatically set a breakpoint at the first instruction in `doubleToFloatx`, and run with command line arguments 32 and 8. *You will need to type in values to convert since stdin is connected to the keyboard in this case*.
- `test` - Runs testFloatx using some easy to debug formats such as 32 8 (for standard `float` format), 64 11 (for standard double format) and 16 4 which does not map to an existing floating point format. The conversion is run on all the numbers in test1.txt which contains several test cases that are useful for debugging.

## Coding the project

After reading through this README, start implementing the `doubleToFloatx` function. Use `make test` to check your results. You can use on-line floating point calculators to figure out what the correct values should be for a float (32 8) or double (64 11) specification, so get those working first.

You will have to manually verify the results for other specifications, such as a 16 4 floatx specification.

Here is an example of the output produced when your program is coded correctly:

```txt
>./testFloatx 32 8 <test1.txt
             1 = floatx(32,8) : 000000003f800000
           100 = floatx(32,8) : 0000000042c80000
     1.3333333 = floatx(32,8) : 000000003faaaaaa
   1.27563e+45 = floatx(32,8) : 000000007f800000
      6.23e+22 = floatx(32,8) : 00000000655314a2
       7.9e-41 = floatx(32,8) : 000000000000dc38
 3.4028235e+38 = floatx(32,8) : 000000007f7fffff
 3.4028236e+38 = floatx(32,8) : 000000007f7fffff
Input : q not recognized as a double...         quitting

```

Once you have the basics working, think about some other specifications and values that might cause problems, and try those as well.

## Hints and Suggestions

1. Use the same trick we will use in the Lab06 `showFloat` program to get the concrete bits of the double `val` parameter

   ```C
   union hexDouble {
     double dbl;
     unsigned long lng;
   } unionVal;   
   ```

   Then, you can take the double input parameter, copy it to `unionVal.dbl`, and use `unionVal.lng` to inspect the bits.

2. When the C `scanf` function reads standard input and converts to a double, it accepts many different formats. Several different formats are represented in the example above. Scan also recognizes "inf" for infinity, and on many machines "nan" for "not a number". However, not all versions of the C library recognize "nan" as not a number, including the C library on the LDAP machines. Therefore, we will not use "nan" as a test case when grading.

3. Remember that most of the examples we show have exponent fields that are the same length or shorter than the exponent field in a IEEE double (11 bits). However, it is possible to use a floatx specification, such as floatx(64,14) which has an exponent field that is actually *bigger* than a double exponent. Make sure your code can handle this case.

4. Use the online floating point calculators to double check your work, at least for common specifications such as "32 8" for single-precision floating point numbers [found here](https://www.h-schmidt.net/FloatConverter/IEEE754.html) or "64 11" for double-precision floating point numbers [found here](https://christophervickery.com/IEEE-754/).

5. One of the hardest special cases to deal with are "sub-normal" numbers... numbers in which the biased exponent is 0, which indicates that the fraction should be preceded with "0." instead of "1."

   This gets even more complicated because it is possible for numbers which are normal when represented as a double can become sub-normal when represented as a floatx. There are even cases where numbers which *are* subnormal when represented as a double may *not be* subnormal when represented in floatx (for instance, when floatx has more than the 11 bits reserved for exponents in the double format.)

## Submitting the Project

When you have finished coding and testing your testFloatx program, make sure you do a final git commit and push to update your main git repository. Then, retrieve the 16 digit git hash code either from browsing your updated repository on the web, or using the command `git rev-parse HEAD` on the command line. Paste the git hash code in the Brightspace Proj02 assignment. You may resubmit as many times as you would like to. Only the last submission will be graded.

## Grading Criteria

Your repository will be downloaded onto an LDAP machine at the level corresponding to the hash code retrieved from Brightspace, and compiled and run using the Makefile. Your code will be tested with the same inputs as the example above, and with three other sets of input values, and the results will be compared to correct results. The project is worth 100 points, and the following deductions will occur:

- -100 if your code matches or is significantly similar to any other student's code, or if your code does not attempt to address the problem.
- -80 if testFloatx does not compile
- Up to -30 if there are mismatches on the published test case
- Up to -30 points per test if there are incorrect results on each unpublished test case (for a maximum of -60)
- -10 if testFloatx produces warning messages when compiled
- -10 for each 24 hours (or part of 24 hours) the submission is late After 5 days, there will be a -100 deduction
- -5 If your repository contains an executable file.

If the correct results are not produced for the published test case, I will try to fix your code. For each fix, there will be a deduction of -15. If I cannot fix your code, or if you code requires more than 4 fixes, you will get a -60 deduction.

## Grace Days

Grace days used for this assignment: -/3

Grace days remaining -/7