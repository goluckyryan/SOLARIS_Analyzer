#include "ClassHelios.h"

void test() {

  // HELIOS helios("../working/detectorGeo.txt", 1);
  // helios.GetDetectorGeometry().Print(true);

  std::vector<unsigned short> ID;
  {
    DetGeo temp("../working/detectorGeo.txt");
    for( size_t i = 0; i < temp.array.size(); i++ ){
      if( temp.array[i].enable ) ID.push_back(i);
    }
  }
  const unsigned short numID = ID.size();

}