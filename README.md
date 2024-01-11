# Introduction

This is the analysis package for the SOLARIS DAQ. It is supposed to be the analysis path for the SOLARIS DAQ. 

The folder struture is

Analysis

├── README.md

├── SetupNewExp // bash script to create new branch and raw data folder

├── SOLARIS.sh // bash script to define some env variable and functions

├── armory // analysis codes, independent from experiment.

├── Cleopatra // Swaper for DWBA code Ptolomey

├── data_raw // should be the symbolic link to the raw data, created by SetUpNewExp

├── root_data // symbolic link to converted root file, created by SetUpNewExp

└── working // working directory, depends on experiment.

# Event Builder

The EventBuilder is at the armory. It depends on the Hit.h and SolReader.h.

## Hit.h

The Hit class stores a hit (or a data block)

## SolReader.h

The SolReader class read the sol file. It can be loaded in CERN ROOT alone. 
