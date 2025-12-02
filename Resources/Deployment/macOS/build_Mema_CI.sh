# we are in Resources/Deployment/macOS/ -> change directory to project root
cd ../../../

# set convenience variables
JUCEDir=submodules/JUCE
ProjucerPath="$JUCEDir"/extras/Projucer/Builds/MacOSX
ProjucerBinPath="$ProjucerPath"/build/Release/Projucer.app/Contents/MacOS/Projucer
JucerProjectPath=Mema.jucer
XCodeProjectPath=Builds/MacOSX/Mema.xcodeproj

# build projucer
xcodebuild -project "$ProjucerPath"/Projucer.xcodeproj -configuration Release -jobs 8

# export projucer project
"$ProjucerBinPath" --resave "$JucerProjectPath" --fix-missing-dependencies

# start building the project. The provisioning profile specification refers to a profile manually created and physically present on the build machine...
xcodebuild -project "$XCodeProjectPath" -configuration Release -jobs 8 CODE_SIGNING_ALLOWED=NO
