#=================== Main
unset SOLARISSYS

SOURCE=${BASH_ARGV[0]}
PCName=$(uname -n)
if [ $(uname -n) == "solaris-daq" ]; then
  SOLARISSYS=~/Analysis
else
  if [ $(pwd) == $HOME ]; then 
    SOLARISSYS=$(dirname ${SOURCE})
    
    if [[ ${SOLARISSYS} == *"$HOME"* ]]; then
      dummpy=0
    else
      SOLARISSYS=${HOME}/$SOLARISSYS
    fi
    
  else
    SOLARISSYS=$(pwd)
  fi
fi

export SOLARISSYS