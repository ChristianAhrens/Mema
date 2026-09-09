# get required dependencies on Raspberry PI OS
chmod +x get_dependencies_RaspberryPIOS.sh
./get_dependencies_RaspberryPIOS.sh

# we are in Resources/Deployment/Linux/ -> change directory to project root
cd ../../../

# setup git submodules first
git submodule update --init --recursive

# we are in project root, change to buildscript directory
cd Resources/Deployment/Linux

# start the buildscript (make -j n instead of default 8 to not overload the poor raspi)
Cores=2
if ! [ -z "$1" ]
then
  Cores="$1"
fi
chmod +x build_Mema.sh
./build_Mema.sh $Cores

# -> after a successful build, one could e.g. use the following as contents for .xsession to have the app start in kind of a kiosk mode. VNC of raspbian will still work in this scenario btw
##!/bin/sh
#exec Documents/Development/GitHub/Mema/Builds/LinuxMakefile/build/Mema
