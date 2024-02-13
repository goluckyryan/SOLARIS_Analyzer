#ifndef targetScattering_h
#define targetScattering_h

#include <vector>
#include <fstream>
#include <string>

#include "../Armory/AnalysisLib.h"

//=======================================================
//#######################################################
// Class for multi-scattering of the beam inside target
// using SRIM to generate stopping power
// input : TLorentzVector, data_files
// output : scattered TLorentzVector
//=======================================================
class TargetScattering{

public:
  TargetScattering();
  ~TargetScattering();
   
  double GetKE0(){return KE0;}
  double GetKE() {return KE;}
  double GetKELoss() {return KE0-KE;}
  double GetDepth() {return depth;}
  double GetPathLength() {return length;}
   
  void LoadStoppingPower(std::string file);
   
  void SetTarget(double density, double depth){
    this->density = density;
    this->depth = depth;
    isTargetSet = true;
    //printf("===== Target, density: %f g/cm3, depth: %f um \n", density, depth * 1e+4 );
  }
   
  TLorentzVector Scattering(TLorentzVector P){
    double mass = P.M();
    KE0 = (P.E() - mass);
    KE = KE0;
    double theta = P.Theta();   
    this->length = TMath::Abs(depth/TMath::Cos(theta));
    //printf("------- theta: %f deg, length: %f um, KE: %f MeV \n", theta * TMath::RadToDeg(), this->length * 1e+4, KE);
    
    //integrate the energy loss within the depth of A
    double dx = length/100.; // in cm
    double x = 0;
    double densityUse = density;
    if( unitID == 0 ) densityUse = 1.;
    do{
      //assume the particle will not be stopped
      //printf(" x: %f, KE:  %f, S: %f \n", x, KE, gA->Eval(KE));
      KE = KE - densityUse * gA->Eval(KE) * 10 * dx  ; // factor 10, convert MeV/cm -> MeV/cm
      
      //angular Straggling, assume (Lateral Straggling)/(Projected range)
      
      
      x = x + dx;
    }while(x < length && KE > 0 );
    
    //printf(" depth: %f cm = %f um, KE : %f -> %f MeV , dE = %f MeV \n", depth, depth * 1e+4, KE0, KE, KE0 - KE);
    double newk = 0;
    
    TLorentzVector Pnew;
    TVector3 vb(0,0,0);
    
    if( KE < 0 ) {
      KE = 0.0; // somehow, when KE == 0 , the program has problem to rotate the 4-vector
    }else{
      newk = TMath::Sqrt(TMath::Power(mass+KE,2) - mass * mass);
      vb = P.Vect();
      vb.SetMag(newk);      
    }
    Pnew.SetVectM(vb,mass);

    return Pnew;
  }
   
private:
  bool isTargetSet;
  double density; // density [mg/cm2]
  int unitID; // 0 = MeV /mm or keV/um , 1 = MeV / (ug/cm2) 
  
  double depth; // depth in target [cm]
  double length; // total path length in target [cm]
  double KE0, KE;
  
  TGraph * gA; // stopping power of A, b, B, in unit of MeV/(mg/cm2)
  TGraph * gB; // projection range [nm]
  TGraph * gC; // parallel Straggling [nm]
  TGraph * gD; // perpendicular Straggling [nm]
      
};

inline TargetScattering::TargetScattering(){
  isTargetSet = false;
  density = 1; // mg/cm2
  unitID = 0; 
  KE0 = 0;
  KE = 0;
  depth = 0;
  length = 0;
  gA = NULL;
  gB = NULL;
  gC = NULL;
  gD = NULL;
}

inline TargetScattering::~TargetScattering(){
   delete gA;
}

inline void TargetScattering::LoadStoppingPower(std::string filename){
   
  printf("loading Stopping power: %s.", filename.c_str());
   
  std::ifstream file;
  file.open(filename.c_str());
   
  std::vector<double> energy;
  std::vector<double> stopping;
  std::vector<double> projRange;
  std::vector<double> paraStraggling;
  std::vector<double> prepStraggling;
   
  if( file.is_open() ){
    printf("... OK\n");
    char lineChar[16635];
    std::string line;
    while( !file.eof() ){
      file.getline(lineChar, 16635, '\n');
      line = lineChar;

      size_t found = line.find("Target Density");
      if( found != std::string::npos ){
        printf("    %s\n", line.c_str());
      }
         
      found = line.find("Stopping Units =");
      if( found != std::string::npos){         
        printf("    %s\n", line.c_str());
        if( line.find("MeV / mm") != std::string::npos ) { 
            unitID = 0;
        }else if( line.find("keV / micron") != std::string::npos ){
            unitID = 0;
        }else if( line.find("MeV / (mg/cm2)") != std::string::npos ) {
            unitID = 1;
        }else{
            printf("please using MeV/mm, keV/um, or MeV/(mg/cm2) \n");  
        }
      }
         
      size_t foundkeV = line.find("keV   ");
      size_t foundMeV = line.find("MeV   ");
      size_t foundGeV = line.find("GeV   ");
      if ( foundkeV != std::string::npos || foundMeV != std::string::npos || foundGeV != std::string::npos ){
        std::vector<std::string> haha = AnalysisLib::SplitStr(line, "  ");
        //for( int i = 0 ; i < (int) haha.size()-1; i++){
        //   printf("%d,%s|", i, haha[i].c_str());
        //}
        //printf("\n");
         
        found = haha[0].find("keV"); if( found != std::string::npos ) energy.push_back(atof(haha[0].substr(0, found).c_str()) * 0.001);
        found = haha[0].find("MeV"); if( found != std::string::npos ) energy.push_back(atof(haha[0].substr(0, found).c_str()) * 1.);
        found = haha[0].find("GeV"); if( found != std::string::npos ) energy.push_back(atof(haha[0].substr(0, found).c_str()) * 1000.);
        
        double a = atof(haha[1].c_str());
        double b = atof(haha[2].c_str());
        stopping.push_back(a+b);
        
        found = haha[3].find("A") ; if( found != std::string::npos ) projRange.push_back(atof(haha[3].substr(0, found).c_str()) * 0.1);
        found = haha[3].find("um"); if( found != std::string::npos ) projRange.push_back(atof(haha[3].substr(0, found).c_str()) * 1000.);
        found = haha[3].find("mm"); if( found != std::string::npos ) projRange.push_back(atof(haha[3].substr(0, found).c_str()) * 1e6);
        
        found = haha[4].find("A") ; if( found != std::string::npos ) paraStraggling.push_back(atof(haha[4].substr(0, found).c_str()) * 0.1);
        found = haha[4].find("um"); if( found != std::string::npos ) paraStraggling.push_back(atof(haha[4].substr(0, found).c_str()) * 1e3);
        found = haha[4].find("mm"); if( found != std::string::npos ) paraStraggling.push_back(atof(haha[4].substr(0, found).c_str()) * 1e6);
        
        found = haha[5].find("A") ; if( found != std::string::npos ) prepStraggling.push_back(atof(haha[5].substr(0, found).c_str()) * 0.1);
        found = haha[5].find("um"); if( found != std::string::npos ) prepStraggling.push_back(atof(haha[5].substr(0, found).c_str()) * 1e3);
        found = haha[5].find("mm"); if( found != std::string::npos ) prepStraggling.push_back(atof(haha[5].substr(0, found).c_str()) * 1e6);
        
        //printf(" %f, %f, %f, %f, %f \n", energy.back(), stopping.back(), projRange.back(), paraStraggling.back(), prepStraggling.back());
      }
         
         
    }
      
  }else{
      printf("... fail\n");
  }
  
  gA = new TGraph(energy.size(), &energy[0], &stopping[0]);
  
  gB = new TGraph(energy.size(), &energy[0], &projRange[0]);
  gC = new TGraph(energy.size(), &energy[0], &paraStraggling[0]);
  gD = new TGraph(energy.size(), &energy[0], &prepStraggling[0]);
  
}

#endif