# we are in Resources/Deployment/iOS/ -> change directory to project root
cd ../../../

# set convenience variables
AppStoreUser=$1 # appleid to use for appstore access
ProjectName=MemaRe
ProjectSpecialName=Mema.Re
JUCEDir=submodules/JUCE
ProjucerPath="$JUCEDir"/extras/Projucer/Builds/MacOSX
ProjucerBinPath="$ProjucerPath"/build/Release/Projucer.app/Contents/MacOS/Projucer
ProjectDir="$ProjectName"/Builds/iOS
JucerProjectPath="$ProjectName"/"$ProjectName".jucer
XCodeProjectPath="$ProjectDir"/Mema.xcodeproj
XCodeProjectScheme="$ProjectSpecialName"" - App"
XCodeBuildPath="$ProjectDir"/build
XCodeEntitlementsPath=Resources/Deployment/iOS/App.entitlements
XCodeExportOptionsPath=Resources/Deployment/iOS/exportOptions.plist
AppExportPath="$ProjectDir"/"$ProjectName"

# build projucer
xcodebuild -project "$ProjucerPath"/Projucer.xcodeproj -configuration Release -jobs 8

# export projucer project
"$ProjucerBinPath" --resave "$JucerProjectPath"

# copy entitlements in place
cp "$XCodeEntitlementsPath" "$ProjectDir"

# build the project
xcodebuild -project "$XCodeProjectPath" -scheme "$XCodeProjectScheme" clean archive -archivePath "$XCodeBuildPath"/"$ProjectName" -allowProvisioningUpdates -destination generic/platform=iOS

# export the archive to app bundle
xcodebuild -exportArchive -archivePath "$XCodeBuildPath"/"$ProjectName".xcarchive -exportOptionsPlist "$XCodeExportOptionsPath" -exportPath "$AppExportPath" -allowProvisioningUpdates

# for validation and upload, we use a simplified package name
mv "$AppExportPath"/"$ProjectSpecialName".ipa "$AppExportPath"/"$ProjectName".ipa

# validate the app bundle
# (assumes that a keychain item "AppSpecificPW" exists, created e.g. with "xcrun altool --store-password-in-keychain-item "AppSpecificPW" -u "abc" -p "def"")
xcrun altool --validate-app --file "$AppExportPath"/"$ProjectName".ipa --type ios --username "$AppStoreUser" --password "@keychain:AppSpecificPW"

# uploading the app bundle can now be done with the following
xcrun altool --upload-app --file "$AppExportPath"/"$ProjectName".ipa --type ios --username "$AppStoreUser" --password "@keychain:AppSpecificPW"
