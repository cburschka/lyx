#!/bin/bash

# find out where we are relative to the program directory
curdir=$(pwd);
progloc=$0;
pathto=${progloc%/*};
# get us into development/tools
if [ "$progloc" != "$pathto" ]; then
  if ! cd $pathto; then
	echo "Couldn't get to development/tools!";
	exit 1;
  fi
fi

if ! cd ../../lib/layouts/; then
  echo "Couldn't get to lib/layouts!";
  exit 1;
fi

for i in *; do 
  cp $i $i.old; 
  python ../scripts/layout2layout.py <$i.old >$i; 
done
