CC=g++

ALL = Isotope InFileCreator ExtractXSec ExtractXSecFromText PlotTGraphTObjArray Cleopatra FindThetaCM SimTransfer SimAlpha

all: $(ALL)

Isotope: ../Cleopatra/ClassIsotope.h ../Cleopatra/Isotope.C
	$(CC) Isotope.C -o Isotope

InFileCreator: InFileCreator.C InFileCreator.h ../Cleopatra/ClassIsotope.h ../Cleopatra/constant.h potentials.h
	$(CC) InFileCreator.C -o InFileCreator `root-config --cflags --glibs`

ExtractXSec: ExtractXSec.C ExtractXSec.h
	$(CC) ExtractXSec.C -o ExtractXSec `root-config --cflags --glibs`

ExtractXSecFromText: ExtractXSecFromText.C ExtractXSec.h
	$(CC) ExtractXSecFromText.C -o ExtractXSecFromText `root-config --cflags --glibs`

PlotTGraphTObjArray: PlotTGraphTObjArray.C PlotTGraphTObjArray.h
	$(CC) PlotTGraphTObjArray.C -o PlotTGraphTObjArray `root-config --cflags --glibs`

Cleopatra: Cleopatra.C
	$(CC) Cleopatra.C -o Cleopatra `root-config --cflags --glibs`

FindThetaCM: FindThetaCM.C FindThetaCM.h ../Cleopatra/ClassTransfer.h ../Cleopatra/ClassHelios.h ../Cleopatra/ClassIsotope.h ../Cleopatra/constant.h
	$(CC) FindThetaCM.C -o FindThetaCM `root-config --cflags --glibs`

SimTransfer: SimTransfer.C ../Cleopatra/ClassTransfer.h ../Cleopatra/ClassHelios.h ../Cleopatra/ClassIsotope.h ../Cleopatra/constant.h 
	$(CC) SimTransfer.C -o SimTransfer `root-config --cflags --glibs`

SimAlpha: SimAlpha.C ../Cleopatra/ClassHelios.h 
	$(CC) SimAlpha.C -o SimAlpha `root-config --cflags --glibs`

clean:
	/bin/rm -f $(ALL)