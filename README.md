# Introduction

This is the analysis package for the SOLARIS DAQ. It is supposed to be the analysis path for the SOLARIS DAQ. 

The folder struture is

Analysis  
├── README.md  
├── SetupNewExp // bash script to create new branch and raw data folder  
├── SOLARIS.sh // bash script to define some env variable and functions  
├── Armory // analysis codes, independent from experiment.  
├── Cleopatra // Swaper for DWBA code Ptolomey and simulation  
├── data_raw // should be the symbolic link to the raw data, created by SetUpNewExp  
├── root_data // symbolic link to converted root file, created by SetUpNewExp  
└── working // working directory, depends on experiment.

# Analysis & Simulation

The Armory/AnalysisLib.h constains many small but handy functions.

All class headers are started with Class*.h

The classes **DetGeo**** and **ReactionConfig** are fundamental for loading the detectorGeo.txt and reactionConfig.txt. 

Both txt file support empty lines, and up to 2 settings. The reason for that is for dual-array configuration. It has potentail to extend and include more settings. But it is two now, one for upstream array (reaction) and downstream array (reaction).

The **TransferReaction** class is only use one of the reaction from the reactionConfig.txt. 

```C++
  TransferReaction::SetReactionFromFile("reactionConfig.txt", ID); // ID = 0 or 1
```
Same for the **Helios** class

```C++
  HELIOS::SetDetectorGeometry("detectorGeo.txt", ID); // ID = 0 or 1
```


# Event Builder

The EventBuilder is at the armory. It depends on the Hit.h and SolReader.h.

## Hit.h

The Hit class stores a hit (or a data block)

## SolReader.h

The SolReader class read the sol file. It can be loaded in CERN ROOT alone. 
