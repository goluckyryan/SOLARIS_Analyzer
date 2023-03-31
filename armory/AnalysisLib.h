#ifndef ANALYSIS_LIB_H
#define ANALYSIS_LIB_H

#include <cstdio>
#include <vector>
#include <string>

std::vector<int> ExtractDetNum(std::vector<std::vector<int>> mapping, std::vector<std::string> detName, std::vector<int> detMaxID){
  std::vector<int> detNum;
  for( int i = 0; i < (int) detName.size(); i ++) detNum.push_back(0);
  for( int i = 0; i < (int) mapping.size(); i ++){
    for( int j = 0; j < (int) mapping[i].size(); j++){
      if( mapping[i][j] < 0) continue;
      for( int k = 0; k < (int) detName.size() ; k ++ ){
        int low = (k == 0 ? 0 : detMaxID[k-1]);
        int high = detMaxID[k];
        if( low <=  mapping[i][j]  && mapping[i][j] < high ) {
          detNum[k]++;
        }
      }
    }
  }
  return detNum;
}

void PrintMapping(std::vector<std::vector<int>> mapping, std::vector<std::string> detName, std::vector<int> detMaxID){

  //------------                       Red          Green      Yellow       Cyan               
  std::vector<const char* > Color = {"\033[31m", "\033[32m", "\033[33m", "\033[36m"};

  printf("==================================== Mapping ===================================\n");
  std::vector<int> detNum = ExtractDetNum(mapping, detName, detMaxID);
  for(int i = 0 ; i < (int) detName.size(); i++) {
    printf(" %2d | %7s | %3d | %3d - %3d\n", i, detName[i].c_str(), detNum[i], (i == 0 ? 0 : detMaxID[i-1]), detMaxID[i]);
  }
  for( int i = 0; i < (int) mapping.size(); i++){
    printf("Digi-%d ------------------------------------------------------------------------ \n", i);
    int colorIndex = -1;
    for( int j = 0; j < (int) mapping[i].size(); j++){
      if( mapping[i][j] < 0 ){
        printf("%4d,", mapping[i][j]); 
      }else{
        for( int k = 0; k < (int) detName.size() ; k ++ ){
          int low = (k == 0 ? 0 : detMaxID[k-1]);
          int high = detMaxID[k];
          if( low <=  mapping[i][j]  && mapping[i][j] < high ) {
            if( k == 0 ) {
              colorIndex = (colorIndex + 1) % 3;
            }else{
              colorIndex = 3;
            }
            printf("%s%4d\033[0m,", Color[colorIndex], mapping[i][j]);
          }
        }
      }
      if( j % 16 == 15 ) printf("\n");
    }
  }
  printf("================================================================================\n");
}




#endif