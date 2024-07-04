CC=g++

ALL = Isotope InFileCreator ExtractXSec ExtractXSecFromText PlotTGraphTObjArray Cleopatra FindThetaCM SimTransfer SimTransfer_single SimAlpha

all: $(ALL)

Isotope: ClassIsotope.h Isotope.C
	$(CC) Isotope.C -o Isotope

InFileCreator: InFileCreator.C InFileCreator.h ClassIsotope.h constant.h potentials.h
	$(CC) InFileCreator.C -o InFileCreator `root-config --cflags --glibs`

ExtractXSec: ExtractXSec.C ExtractXSec.h
	$(CC) ExtractXSec.C -o ExtractXSec `root-config --cflags --glibs`

ExtractXSecFromText: ExtractXSecFromText.C ExtractXSec.h
	$(CC) ExtractXSecFromText.C -o ExtractXSecFromText `root-config --cflags --glibs`

PlotTGraphTObjArray: PlotTGraphTObjArray.C PlotTGraphTObjArray.h
	$(CC) PlotTGraphTObjArray.C -o PlotTGraphTObjArray `root-config --cflags --glibs`

Cleopatra: Cleopatra.C InFileCreator.h ExtractXSec.h
	$(CC) Cleopatra.C -o Cleopatra `root-config --cflags --glibs`

FindThetaCM: FindThetaCM.C FindThetaCM.h ClassTransfer.h ClassHelios.h ClassIsotope.h constant.h
	$(CC) FindThetaCM.C -o FindThetaCM `root-config --cflags --glibs`

SimTransfer: SimTransfer.C ClassTransfer.h ClassHelios.h ClassIsotope.h constant.h 
	$(CC) SimTransfer.C -o SimTransfer `root-config --cflags --glibs`

SimTransfer_single: SimTransfer_single.C ClassTransfer.h ClassHelios.h ClassIsotope.h constant.h ../Armory/ClassReactionConfig.h ../Armory/ClassDetGeo.h
	$(CC) SimTransfer_single.C -o SimTransfer_single `root-config --cflags --glibs`

SimAlpha: SimAlpha.C ClassHelios.h 
	$(CC) SimAlpha.C -o SimAlpha `root-config --cflags --glibs`

clean:
	/bin/rm -f $(ALL)