# Introduction

This is the analysis package for the SOLARIS DAQ. It is supposed to be the analysis path for the SOLARIS DAQ. 

The folder struture is

Analysis  
├── README.md  
├── SetupNewExp // bash script to create new branch and raw data folder  
├── SOLARIS.sh // bash script to define some env variable and functions  
├── armory // analysis codes, independent from experiment.  
├── cleopatra // Swaper for DWBA code Ptolomey and simulation  
├── data_raw // should be the symbolic link to the raw data, created by SetUpNewExp  
├── root_data // symbolic link to converted root file, created by SetUpNewExp  
└── working // working directory, depends on experiment.

# SOLARIS.sh

this batch shell script adds few enviroment variables and functions. Add the `armory` and `cleopatra` into the system PATH.

```sh
>source SOLARIS.sh
```

and also set your PCName

```sh
>export PCName="what-ever-your-like"
```

the PCName use to identify different computer.

# Event Builder

Please download the SOLARIS_DAQ, under the `Aux` directory, make, and link the EventBuilder to `armory`.

The reason for having EventBuilder in the DAQ code is the Hit.h is original from the DAQ code.

# ROOT issue

We are still using TProof for parallel calculation. TProof is not pre-compiled since 6.32+. And 6.30 only precompiled for Ubuntu 22.04. So, for system using Ubuntu 24.04, user must precompiled to root in order to work.

## Compilation

We can manual download the git repository of the root following the instruction. in the cmake 

```sh
root_install="/opt/root_v6.32.00_compiled"
root_src="root_src"
root_build="root_build"
cmake -DCMAKE_INSTALL_PREFIX=$root_install $root_src -Dproof=ON -Dmathmore=ON
sudo cmake --build . --target install -j <number_of_threads>
```

# Analysis & Simulation

The armory/AnalysisLib.h constains many small but handy functions.

All class headers are started with Class*.h

The classes **DetGeo**** and **ReactionConfig** are fundamental for loading the detectorGeo.txt and reactionConfig.txt. 

Both txt file support empty lines, can have multiple settings. The reason for that is for many-array configuration. 

The **TransferReaction** class is only use one of the reaction from the reactionConfig.txt. This class generates the TLorentzVector for ligh and heavy recoils.

```C++
  TransferReaction::SetReactionFromFile("reactionConfig.txt", ID); // ID = 0 or 1
```
Same for the **Helios** class, **Helios** class use the detectorGeo.txt. It takes TLorentzVector and calculate does it be detected by the array or recoil detector.

```C++
  HELIOS::SetDetectorGeometry("detectorGeo.txt", ID); // ID = 0 or 1
```

## Simulation

Simply run
```sh
>SimTransfer
```

it will digest the detectorGeo.txt, reactionConfig.txt, if DWBA.root exist, find the reactions. 

* it does not have TargetScattering (yet)
* for multiple reactions, it will randomly use any and disregard the total Xsec of different reactions. The Xsec only takes effect within same reaction.
* the decay of heavy recoil only have isotropic decay.


