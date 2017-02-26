#!/bin/bash

CXX=gcc
CFLAGS=
CXXFLAGS=
LDFLAGS="-D LAPTOP"

debug=0
platform="laptop"
output="batcheck"
dset="false"
oset="false"
pset="false"

# @TODO: Help message?

while getopts ":dp:ho:" options; do
	case $options in
		d)
			if [[ $dset == "true" ]]; then
				echo "Error: option $OPTARG set multiple times." >&2
				exit -1
			fi
			echo "Building in debug mode."
			dset="true"
			CFLAGS=-Wall
			;;
		o)
			if [[ $oset == "true" ]]; then
				echo "Error: output set multiple times." >&2
				exit -2
			fi
			oset="true"
			output="$OPTARG"
			echo "Building as $output."
			;;
		p)
			if [[ $pset == "true" ]]; then
				echo "Error: platform set multiple times." >&2
				exit -3
			fi
			pset="true"
			case $OPTARG in
				desktop)
					platform="desktop"
					LDFLAGS="-U LAPTOP -D DESKTOP"
					;;
				laptop)
					platform="laptop"
					;;
				*)
					echo "Error: invalid platform." >&2
					exit -4
					;;
			esac
			echo "Building for $platform."
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit -5
			;;
	esac
done

input="batcheck.c"

$CXX $CFLAGS $CXXFLAGS -o$output $input $LDFLAGS
