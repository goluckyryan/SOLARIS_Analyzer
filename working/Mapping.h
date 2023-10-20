#ifndef MAPPING_H
#define MAPPING_H

//^===============================================================================
//^   This is mapping file for SOLARIS
//^   This file is used to constructe the SOLARIS panel in the SOLARIS DAQ
//^   If this file is modified, please Close Digitizer and Open again
//^-------------------------------------------------------------------------------
//^  
//^   Array-e  :    0 -  99
//^   Array-xf :  100 - 199
//^   Array-xn :  200 - 299
//^   Recoil   :  300 - 399
//^   Enum     :  400 - 499
//^   EZERO    :  500 - 599
//^   Apollo   :  600 - 699
//^   
//^   line comment is line constains '//^' or '// //' or '////'
//^ 
//^===============================================================================

#include <vector>
#include <string>

namespace mapping{

const std::vector<std::string> detTypeName = {  "e",  "xf", "xn", "rdt", "eNum"}; //C= The comment "//C=" is an indicator DON't Remove
const std::vector<int>          detGroupID = {    0,     0,    0,     1,      2}; //C& The comment "//C&" is an indicator DON't Remove
const std::vector<int>            detMaxID = {  100,   200,  300,   400,    500}; //C# The comment "//C#" is an indicator DON't Remove
const std::vector<int>           detParity = {    1,     1,    1,     1,      1}; 

const std::vector<std::string>   groupName = { "Array", "Recoil", "ELUM"}; //C% The comment "//C%" is an indicator DON't Remove

//!The mapping[i] must match as the IP setting in the DAQ

const std::vector<std::vector<int>> map = {
{
//C   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15 // this line is an indicator DON'T Remove "//C " is an indcator
      0,  100,  200,    1,  101,  201,    2,  102,  202,    3,  103,  203,    4,  104,  204,   -1,  ///  0 - 15 
      5,  105,  205,    6,  106,  206,    7,  107,  207,    8,  108,  208,    9,  109,  209,   -1,  /// 16 - 31
     10,  110,  210,   11,  111,  211,   12,  112,  212,   13,  113,  213,   14,  114,  214,   -1,  /// 32 - 47
     15,  115,  215,   16,  116,  216,   17,  117,  217,   18,  118,  218,   19,  119,  219,   -1   /// 48 - 63
//C------------- end of a digitizer  // this line is an indicator DON'T Remove "//C-" is an indcator
}
,{
//C   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15 // this line is an indicator DON'T Remove "//C " is an indcator
     20,  120,  220,   21,  121,  221,   22,  122,  222,   23,  123,  223,   24,  124,  224,   -1,  ///  0 - 15 
     25,  125,  225,   26,  126,  226,   27,  127,  227,   28,  128,  228,   29,  129,  229,   -1,  /// 16 - 31
     30,  130,  230,   31,  131,  231,   32,  132,  232,   33,  133,  233,   34,  134,  234,   -1,  /// 32 - 47
     35,  135,  235,   36,  136,  236,   37,  137,  237,   38,  138,  238,   39,  139,  239,   -1   /// 48 - 63
//C------------- end of a digitizer // this line is an indicator DON'T Remove "//C-" is an indcator
}
,{
//C   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15 // this line is an indicator DON'T Remove "//C " is an indcator
     40,  140,  240,   41,  141,  241,   42,  142,  242,   43,  143,  243,   44,  144,  244,   -1,  ///  0 - 15 
     45,  145,  245,   46,  146,  246,   47,  147,  247,   48,  148,  248,   49,  149,  249,   -1,  /// 16 - 31
     50,  150,  250,   51,  151,  251,   52,  152,  252,   53,  153,  253,   54,  154,  254,   -1,  /// 32 - 47
     55,  155,  255,   56,  156,  256,   57,  157,  257,   58,  158,  258,   59,  159,  259,   -1   /// 48 - 63
//C------------- end of a digitizer // this line is an indicator DON'T Remove "//C-" is an indcator
}
,{
//C   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15 // this line is an indicator DON'T Remove "//C " is an indcator
    300,  301,  302,  303,  304,  305,  306,  307,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  ///  0 - 15
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  /// 16 - 31
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  /// 32 - 47
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1   /// 48 - 63
//C------------- end of a digitizer // this line is an indicator DON'T Remove "//C-" is an indcator
}
};

//^===============================================================================
int FindDetTypeIndex(int detID){
  for( int k = 0; k < (int) detMaxID.size(); k++){
    int low = (k == 0 ? 0 : detMaxID[k-1]);
    int high = detMaxID[k];
    if( low <=  detID  && detID < high ) {
      return k;
    }
  }
  return -1;
}

std::vector<int> ExtractDetNum(){
  std::vector<int> detTypeNum;
  for( int i = 0; i < (int) detTypeName.size(); i ++) detTypeNum.push_back(0);
  for( int i = 0; i < (int) map.size(); i ++){
    for( int j = 0; j < (int) map[i].size(); j++){
      if( map[i][j] < 0) continue;
      for( int k = 0; k < (int) detTypeName.size() ; k ++ ){
        int low = (k == 0 ? 0 : detMaxID[k-1]);
        int high = detMaxID[k];
        if( low <=  map[i][j]  && map[i][j] < high ) {
          detTypeNum[k]++;
        }
      }
    }
  }
  return detTypeNum;
}

void PrintMapping(){

  //------------                       Red          Green      Yellow       Cyan        blue      Magenta      Gray
  std::vector<const char* > Color = {"\033[31m", "\033[32m", "\033[33m", "\033[36m", "\033[34m", "\033[35m", "\033[37m"};

  printf("==================================== Mapping ===================================\n");
  std::vector<int> detTypeNum = ExtractDetNum();
  for(int i = 0 ; i < (int) detTypeName.size(); i++) {
    printf(" %2d | %7s | %3d | %3d - %3d\n", i, detTypeName[i].c_str(), detTypeNum[i], (i == 0 ? 0 : detMaxID[i-1]), detMaxID[i]);
  }
  for( int i = 0; i < (int) map.size(); i++){
    printf("Digi-%d ------------------------------------------------------------------------ \n", i);
    for( int j = 0; j < (int) map[i].size(); j++){
      if( map[i][j] < 0 ){
        printf("%4d,", map[i][j]); 
      }else{
        int colorIndex = FindDetTypeIndex(map[i][j]);
        printf("%s%4d\033[0m,", Color[colorIndex], map[i][j]);
      }
      if( j % 16 == 15 ) printf("\n");
    }
  }
  printf("================================================================================\n");
}

const std::vector<int>  detNum = ExtractDetNum();
const int nDetType = detNum.size();

const int NARRAY = detNum[0]; //@ assumed
const int NRDT   = detNum[3]; //@ assumed
     
} // namespace solarismap
#endif
