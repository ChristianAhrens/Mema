# we are in Resources/Deployment/macOS/ -> change directory to project root
cd ../../../

# set convenience variables
AppBundlePath=Builds/MacOSX/build/Release/MemaRe.app
ChangeLogPath=../CHANGELOG.md
CreateDmgPath=../submodules/create-dmg/create-dmg
LicensePath=../LICENSE
IconSetSourcePng=MemaReRect.png
IconSetSourceDiskPng=disk-icon.png
VolIconPath=../Resources/Images/Iconset.icns
DmgTargetPath=MemaRe.dmg
DmgContentsCollectionPath=ContentsPath

# create the icns from existing png
cd Resources/Images
chmod +x makeDmgIcon.sh
./makeDmgIcon.sh "$IconSetSourcePng" "$IconSetSourceDiskPng" dmgIcon.png
chmod +x makeIconset.sh
./makeIconset.sh dmgIcon.png
rm dmgIcon.png
cd ../../MemaRe

# collect dmg contents
test -d "$DmgContentsCollectionPath" && rm -r "$DmgContentsCollectionPath"
mkdir "$DmgContentsCollectionPath"
cp -r "$AppBundlePath" "$DmgContentsCollectionPath"
cp -r "$ChangeLogPath" "$DmgContentsCollectionPath"
ln -s /Applications "$DmgContentsCollectionPath"/Applications

# create project disk image
test -f "$DmgTargetPath" && rm "$DmgTargetPath"
"$CreateDmgPath" --eula "$LicensePath" --window-size 565 245 --volicon "$VolIconPath" "$DmgTargetPath" "$DmgContentsCollectionPath"

# cleanup
test -d "$DmgContentsCollectionPath" && rm -r "$DmgContentsCollectionPath"
test -f "$VolIconPath" && rm "$VolIconPath"
