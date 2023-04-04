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

export PATH=$PATH:$SOLARISANADIR/armory

echo "####### add ${SOLARISANADIR}/armory into PATH"


###########################

echo "####### Define BASH Alias / Functions for SOLARIS"

alias 2Working='cd ${SOLARISANADIR}/working'
alias ShowRunTimeStamp='cat $SOLARISANADIR/data_raw/data/RunTimeStamp.dat'

function ShowRunSize {
  if [ $# -ne 1 ]; then
  	echo 'Please set run number '
    return 0
  fi
  source $SOLARISANADIR/working/expName.sh
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
  du -hc $SOLARISANADIR/data_raw/data/${expName}_${RUN}_*.sol
}
