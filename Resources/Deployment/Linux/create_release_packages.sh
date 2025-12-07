Cores=2
if ! [ -z "$1" ]
then
  Cores="$1"
fi

chmod +x build_all_RaspberryPIOS.sh
./build_all_RaspberryPIOS.sh $Cores

# we are in Resources/Deployment/Linux/ -> change directory to project root
cd ../../../

# set convenience variables
MemaBinaryPath=Builds/LinuxMakefile/build/Mema
MemaMoBinaryPath=MemaMo/Builds/LinuxMakefile/build/Mema
MemaReBinaryPath=MemaRe/Builds/LinuxMakefile/build/Mema
ChangeLogPath=CHANGELOG.md
LicensePath=LICENSE
DependencyInstallScript=get_dependencies_RaspberryPIOS.sh
DependencyInstallScriptPath="Resources/Deployment/Linux/""$DependencyInstallScript"
MemaZipTargetPath=Mema.zip
MemaMoZipTargetPath=MemaMo.zip
MemaReZipTargetPath=MemaRe.zip
PackageContentCollectionPath=PackageContentCollection

mkdir "$PackageContentCollectionPath"

# copy required assets into temp collection folder
cp "$DependencyInstallScriptPath" "$PackageContentCollectionPath/$DependencyInstallScript"
cp "$ChangeLogPath" "$PackageContentCollectionPath/$ChangeLogPath"
cp "$LicensePath" "$PackageContentCollectionPath/$LicensePath"
mv "$MemaBinaryPath" "$PackageContentCollectionPath/Mema"
mv "$MemaMoBinaryPath" "$PackageContentCollectionPath/MemaMo"
mv "$MemaReBinaryPath" "$PackageContentCollectionPath/MemaRe"

# change directory into collection folder
cd "$PackageContentCollectionPath"

# Mema app release package
test -f "$MemaZipTargetPath" && rm "$MemaZipTargetPath"
zip "$MemaZipTargetPath" "Mema" "$ChangeLogPath" "$LicensePath" "$DependencyInstallScript"

# MemaMo app release package
test -f "$MemaMoZipTargetPath" && rm "$MemaMoZipTargetPath"
zip "$MemaMoZipTargetPath" "MemaMo" "$ChangeLogPath" "$LicensePath" "$DependencyInstallScript"

# MemaRe app release package
test -f "$MemaReZipTargetPath" && rm "$MemaReZipTargetPath"
zip "$MemaReZipTargetPath" "MemaRe" "$ChangeLogPath" "$LicensePath" "$DependencyInstallScript"

# back to root for further package handling
cd ..
cp "$PackageContentCollectionPath/$MemaZipTargetPath" "$MemaZipTargetPath"
cp "$PackageContentCollectionPath/$MemaMoZipTargetPath" "$MemaMoZipTargetPath"
cp "$PackageContentCollectionPath/$MemaReZipTargetPath" "$MemaReZipTargetPath"

# cleanup collection folder
test -d "$PackageContentCollectionPath" && rm -R "$PackageContentCollectionPath"
