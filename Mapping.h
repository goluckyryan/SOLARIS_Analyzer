//^===============================================================================
//^   This is mapping file for SOLARIS
//^   This file is used to constructe the SOLARIS panel in the SOLARIS DAQ
//^   If this file is modified, please Close Digitizer and Open again
//^-------------------------------------------------------------------------------
//^  
//^   Array  :   0 - 199
//^   Recoil : 200 - 299
//^   
//^ 
//^===============================================================================

#define NARRAY 60
#define NRDT   10

#define PARITYARRAY 1
#define PARITYRDT   1

#define NDIGITIZER 2
#define NCHANNEL   64  // number of channel per digitizer

int mapping[NDIGITIZER][NCHANNEL] = {
{
//* 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 // this line is an indicator DON'T Remove
    0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,  ///  0 - 15 
    5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10, 10,  /// 16 - 31
   10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15,  /// 32 - 47
   16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, -1   /// 48 - 63
//*------------- end of a digitizer  // this line is an indicator DON'T Remove
},
{
//* 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 // this line is an indicator DON'T Remove
   21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26,  ///  0 - 15 
   26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31,  /// 16 - 31
   31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36,  /// 32 - 47
   37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, -1   /// 48 - 63
//*------------- end of a digitizer // this line is an indicator DON'T Remove
}
};

//& if all array is arrange like this, no need kind mapping. it is always e, xf, xn

