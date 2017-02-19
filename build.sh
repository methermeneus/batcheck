#!/bin/bash

CXX=gcc
CFLAGS=
CXXFLAGS=
LDFLAGS=

debug=0
platform="laptop"
output="batcheck"
dset=
oset=
pset=

# @TODO: Help message?

while getopts do:p:h options; do
	case $options in
		d)
			if [[ ! -z $dset ]]; then
				echo "Error: option $OPTARG set multiple times." >&2
				exit -1
			fi
			echo "Building in debug mode."
			dset="true"
			CFLAGS=-Wall
			;;
		o)
			if [[ ! -z $oset ]]; then
				echo "Error: output set multiple times." >&2
				exit -2
			fi
			oset="true"
			output="$2"
			shift
			echo "Building as $output."
			;;
		p)
			if [[ ! -z $pset ]]; then
				echo "Error: platform set multiple times." >&2
				exit -3
			fi
			pset="true"
			case $2 in
				desktop)
					platform="desktop";;
				laptop)
					platform="laptop";;
				*)
					echo "Error: invalid platform." >&2
					exit -4
					;;
			esac
			shift
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit -5
			;;
	esac
done

input="$platform".c

$CXX $CFLAGS $CXXFLAGS -o$output $input $LDFLAGS
