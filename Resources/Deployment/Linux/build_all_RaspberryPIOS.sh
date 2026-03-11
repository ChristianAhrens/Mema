Cores=2
if ! [ -z "$1" ]
then
  Cores="$1"
fi

chmod +x build_Mema_RaspberryPIOS.sh
./build_Mema_RaspberryPIOS.sh $Cores

chmod +x build_MemaMo_RaspberryPIOS.sh
./build_MemaMo_RaspberryPIOS.sh $Cores

chmod +x build_MemaRe_RaspberryPIOS.sh
./build_MemaRe_RaspberryPIOS.sh $Cores
