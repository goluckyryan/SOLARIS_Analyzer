#ifndef MAPPING_H
#define MAPPING_H

//^===============================================================================
//^   This is mapping file for SOLARIS
//^   This file is used to constructe the SOLARIS panel in the SOLARIS DAQ
//^   If this file is modified, please Close Digitizer and Open again
//^-------------------------------------------------------------------------------
//^  
//^   Array  :   0 -  99
//^   Recoil : 100 - 199
//^   
//^  line comment is '//^'   
//^ 
//^===============================================================================

#include <vector>
#include <string>

const std::vector<std::string> detName = {"Array", "Recoil"}; //C= The comment "//C=" is an indicator DON't Remove
const std::vector<int>        detMaxID = {    100,      200}; //C# The comment "//C#" is an indicator DON't Remove
const std::vector<int>       detParity = {      1,        1}; 

//!The mapping[i] must match as the IP setting in the DAQ

const std::vector<std::vector<int>> mapping = {
{
//C 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 // this line is an indicator DON'T Remove "//C " is an indcator
    0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4, -1,  ///  0 - 15 
    5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, -1,  /// 16 - 31
   10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, -1,  /// 32 - 47
   15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, -1   /// 48 - 63
//C------------- end of a digitizer  // this line is an indicator DON'T Remove "//C-" is an indcator
},
//^{
//^//C 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 // this line is an indicator DON'T Remove "//C " is an indcator
//^   20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, -1,  ///  0 - 15 
//^   25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, -1,  /// 16 - 31
//^   30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, -1,  /// 32 - 47
//^   35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, -1   /// 48 - 63
//^//C------------- end of a digitizer // this line is an indicator DON'T Remove "//C-" is an indcator
//^},
//^{
//^//C 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 // this line is an indicator DON'T Remove "//C " is an indcator
//^   40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, -1,  ///  0 - 15 
//^   45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49, -1,  /// 16 - 31
//^   50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, -1,  /// 32 - 47
//^   55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, -1   /// 48 - 63
//^//C------------- end of a digitizer // this line is an indicator DON'T Remove "//C-" is an indcator
//^},
{
//C  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15 // this line is an indicator DON'T Remove "//C " is an indcator
   100, 101, 102, 103, 104, 105, 106, 107,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  ///  0 - 15
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  /// 16 - 31
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  /// 32 - 47
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1   /// 48 - 63
//C------------- end of a digitizer // this line is an indicator DON'T Remove "//C-" is an indcator
}
};

#endif
