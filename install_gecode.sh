#!/bin/bash

GECODE_NAME=gecode
GECODE_VERSION=4.2.1

GECODE_ARCHIVE="$PWD/$GECODE_NAME-$GECODE_VERSION.tar.gz"
GECODE_DIR="$PWD/$GECODE_NAME"


if  [  -d  $PWD/$GECODE_NAME-$GECODE_VERSION  ] 
then
	rm -rf $PWD/$GECODE_NAME-$GECODE_VERSION 
	rm -rf $GECODE_DIR
fi

if  [  -d  $GECODE_DIR  ] 
then 
	echo $GECODE_DIR" exists. Please remove it if you would like to rebuild gecode from scratch"		
	
	cd $GECODE_DIR

	if  [  -f  $GECODE_DIR/Makefile  ] 
	then 
		echo "Makefile already exists"			
	else
		echo "Makefile does not exist, launch cmake"			
		cmake .		
	fi
else
	tar zxf $GECODE_ARCHIVE

	mv $PWD/$GECODE_NAME-$GECODE_VERSION $PWD/$GECODE_DIR 2> /dev/null

	echo $GECODE_DIR" exists. Please remove it if you would like to rebuild gecode from scratch"		

	cd $GECODE_NAME

	echo "Makefile does not exist, launch cmake"			
	cmake .

fi

echo "start gecode setup..."
make
echo "Gecode bin and libs are installed in"$PWD/$GECODE_NAME

