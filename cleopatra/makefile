CC = g++
CFLAG = -O2

depend = ClassTransfer.h ClassHelios.h ClassIsotope.h ClassDecay.h constant.h potentials.h

ALL = Isotope InFileCreator ExtractXSec ExtractXSecFromText PlotTGraphTObjArray Cleopatra FindThetaCM SimTransfer SimAlpha

all: $(ALL)

Isotope: ClassIsotope.h Isotope.C
	$(CC) $(CFLAG) Isotope.C -o Isotope

InFileCreator: InFileCreator.C InFileCreator.h $(depend)
	$(CC) $(CFLAG) InFileCreator.C -o InFileCreator `root-config --cflags --glibs`

ExtractXSec: ExtractXSec.C ExtractXSec.h
	$(CC) $(CFLAG) ExtractXSec.C -o ExtractXSec `root-config --cflags --glibs`

ExtractXSecFromText: ExtractXSecFromText.C ExtractXSec.h
	$(CC) $(CFLAG) ExtractXSecFromText.C -o ExtractXSecFromText `root-config --cflags --glibs`

PlotTGraphTObjArray: PlotTGraphTObjArray.C PlotTGraphTObjArray.h
	$(CC) $(CFLAG) PlotTGraphTObjArray.C -o PlotTGraphTObjArray `root-config --cflags --glibs`

Cleopatra: Cleopatra.C InFileCreator.h ExtractXSec.h
	$(CC) $(CFLAG) Cleopatra.C -o Cleopatra `root-config --cflags --glibs`

FindThetaCM: FindThetaCM.C FindThetaCM.h $(depend)
	$(CC) $(CFLAG) FindThetaCM.C -o FindThetaCM `root-config --cflags --glibs`

SimTransfer: SimTransfer.C ClassTransfer.h $(depend) ../Armory/ClassReactionConfig.h ../Armory/ClassDetGeo.h
	$(CC) $(CFLAG) SimTransfer.C -o SimTransfer `root-config --cflags --glibs`

SimTransfer_single: SimTransfer_single.C $(depend) ../Armory/ClassReactionConfig.h ../Armory/ClassDetGeo.h
	$(CC) $(CFLAG) SimTransfer_single.C -o SimTransfer_single `root-config --cflags --glibs`

SimAlpha: SimAlpha.C ClassHelios.h 
	$(CC) $(CFLAG) SimAlpha.C -o SimAlpha `root-config --cflags --glibs`

clean:
	/bin/rm -f $(ALL)