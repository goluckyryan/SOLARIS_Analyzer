
#========== this script define the SOLARISANADIR as global env.

unset SOLARISANADIR

SOURCE=${BASH_ARGV[0]}
PCName=$(uname -n)
if [ $(uname -n) == "solaris-daq" ]; then
  SOLARISANADIR=~/Analysis
else
  if [ $(pwd) == $HOME ]; then 
    SOLARISANADIR=$(dirname ${SOURCE})
    
    if [[ ${SOLARISANADIR} == *"$HOME"* ]]; then
      dummpy=0
    else
      SOLARISANADIR=${HOME}/$SOLARISANADIR
    fi
    
  else
    SOLARISANADIR=$(pwd)
  fi
fi

export SOLARISANADIR

echo "####### set global variable SOLARISANADIR = ${SOLARISANADIR}"

export PATH=$PATH:$SOLARISANADIR/armory

echo "####### add ${SOLARISANADIR}/armory into PATH"


###########################

alias 2Working='cd ${SOLARISANADIR}/working'
