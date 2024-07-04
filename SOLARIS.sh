#!/bin/bash
#========== this script define the SOLARISANADIR as global env.

unset SOLARISANADIR

SOURCE=${BASH_ARGV[0]}
PCName=$(uname -n)
if [ $(uname -n) == "solaris-daq" ]; then
  SOLARISANADIR=~/Analysis
else
  ABSPATH=$(realpath ${SOURCE})
  SOLARISANADIR=$(dirname ${ABSPATH})
fi

export SOLARISANADIR

echo "####### set global variable SOLARISANADIR = ${SOLARISANADIR}"

export PATH=$PATH:$SOLARISANADIR/Armory:$SOLARISANADIR/Cleopatra

echo "####### add ${SOLARISANADIR}/Armory into PATH"
echo "####### add ${SOLARISANADIR}/Cleopatra into PATH"


###########################

echo "####### Define BASH Alias and Functions for SOLARIS"
echo "           2Working = goto the working directory"
echo "   ShowRunTimeStamp = show Run Timestamp"
echo "        ShowRunSize = show Run Size"

alias 2Working='cd ${SOLARISANADIR}/working'
alias ShowRunTimeStamp='cat $SOLARISANADIR/data_raw/RunTimeStamp.dat'

function ShowRunSize {
  if [ $# -ne 1 ]; then
  	echo 'Please set run number '
    return 0
  fi
  source $SOLARISANADIR/data_raw/expName.sh
  RUN=$1
  if [ ${RUN} = "latest" ]; then
    RUN=${runID}
  fi
  runLen=${#RUN}
  if [ ${runLen} -eq 1 ]; then
    RUN="00"${RUN}
  elif [ ${runLen} -eq 2 ]; then
    RUN="0"${RUN}
  fi
  du -hc $SOLARISANADIR/data_raw/${expName}_${RUN}_*.sol
}
