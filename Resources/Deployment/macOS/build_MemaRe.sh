# we are in Resources/Deployment/macOS/ -> change directory to project root
cd ../../../MemaRe

# set convenience variables
JUCEDir=../submodules/JUCE
ProjucerPath="$JUCEDir"/extras/Projucer/Builds/MacOSX
ProjucerBinPath="$ProjucerPath"/build/Release/Projucer.app/Contents/MacOS/Projucer
JucerProjectPath=MemaRe.jucer
XCodeProjectPath=Builds/MacOSX/Mema.xcodeproj

# build projucer
xcodebuild -project "$ProjucerPath"/Projucer.xcodeproj -configuration Release -jobs 8

# export projucer project
"$ProjucerBinPath" --resave "$JucerProjectPath"

# start building the project. The provisioning profile specification refers to a profile manually created and physically present on the build machine...
xcodebuild -project "$XCodeProjectPath" -configuration Release -jobs 8 PROVISIONING_PROFILE_SPECIFIER="e7160d5b-5720-4db7-867b-5aad5332f221"
