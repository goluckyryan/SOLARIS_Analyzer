#ifndef ANALYSIS_LIB_H
#define ANALYSIS_LIB_H

#include <cstdio>
#include <vector>
#include <string>


int FindDetType(int detID, std::vector<int> detMaxID){
  for( int k = 0; k < (int) detMaxID.size(); k++){
    int low = (k == 0 ? 0 : detMaxID[k-1]);
    int high = detMaxID[k];
    if( low <=  detID  && detID < high ) {
      return k;
    }
  }
  return -1;
}

std::vector<int> ExtractDetNum(std::vector<std::vector<int>> mapping, std::vector<std::string> detTypeName, std::vector<int> detMaxID){
  std::vector<int> detTypeNum;
  for( int i = 0; i < (int) detTypeName.size(); i ++) detTypeNum.push_back(0);
  for( int i = 0; i < (int) mapping.size(); i ++){
    for( int j = 0; j < (int) mapping[i].size(); j++){
      if( mapping[i][j] < 0) continue;
      for( int k = 0; k < (int) detTypeName.size() ; k ++ ){
        int low = (k == 0 ? 0 : detMaxID[k-1]);
        int high = detMaxID[k];
        if( low <=  mapping[i][j]  && mapping[i][j] < high ) {
          detTypeNum[k]++;
        }
      }
    }
  }
  return detTypeNum;
}

void PrintMapping(std::vector<std::vector<int>> mapping, std::vector<std::string> detTypeName, std::vector<int> detMaxID){

  //------------                       Red          Green      Yellow       Cyan        blue      Magenta      Gray
  std::vector<const char* > Color = {"\033[31m", "\033[32m", "\033[33m", "\033[36m", "\033[34m", "\033[35m", "\033[37m"};

  printf("==================================== Mapping ===================================\n");
  std::vector<int> detTypeNum = ExtractDetNum(mapping, detTypeName, detMaxID);
  for(int i = 0 ; i < (int) detTypeName.size(); i++) {
    printf(" %2d | %7s | %3d | %3d - %3d\n", i, detTypeName[i].c_str(), detTypeNum[i], (i == 0 ? 0 : detMaxID[i-1]), detMaxID[i]);
  }
  for( int i = 0; i < (int) mapping.size(); i++){
    printf("Digi-%d ------------------------------------------------------------------------ \n", i);
    for( int j = 0; j < (int) mapping[i].size(); j++){
      if( mapping[i][j] < 0 ){
        printf("%4d,", mapping[i][j]); 
      }else{
        int colorIndex = FindDetType(mapping[i][j], detMaxID);
        printf("%s%4d\033[0m,", Color[colorIndex], mapping[i][j]);
      }
      if( j % 16 == 15 ) printf("\n");
    }
  }
  printf("================================================================================\n");
}




#endif