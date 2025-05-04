# set external variables
AppIconSource=$1
DiskIconSource=$2
DmgIcon=$3

mkdir DmgIconTmpDir
cp "$AppIconSource" DmgIconTmpDir/"$AppIconSource"
cp "$DiskIconSource" DmgIconTmpDir/

cd DmgIconTmpDir

# in the following we blow up or shrink whatever input to 512x512 target size

# Step 1: Prepare input
magick convert "$AppIconSource" -resize 512x512! "$AppIconSource"
magick convert "$DiskIconSource" -resize 512x512! "$DiskIconSource"
# Step 2: Perspective Transform
magick convert "$AppIconSource" -matte -virtual-pixel transparent -distort Perspective "0,0  40,0   512,0  472,0   0,512  0,512   512,512  512,512" distorted_app.png
# Step 3: Resize the Icon
magick convert distorted_app.png -resize 432x324! resized_app.png
# Step 4: Composite the Icons
magick composite -gravity center -geometry +0-4 resized_app.png "$DiskIconSource" "$DmgIcon"

cd ..

cp DmgIconTmpDir/"$DmgIcon" "$DmgIcon"

rm -R DmgIconTmpDir
