![Showreel.001.png](Resources/Documentation/Showreel/Showreel.001.png "Mema Headline Icons")
![Showreel.002.png](Resources/Documentation/Showreel/Showreel.002.png "Mema Headline Icons")

See [LATEST RELEASE](https://github.com/ChristianAhrens/Mema/releases/latest) for available binary packages or join iOS TestFlight Beta:

<img src="Resources/AppStore/TestFlightQRCode.png" alt="TestFlight QR Code" width="15%">


|Appveyor CI build status|Mema|Mema.Mo|
|:----------------|:-----|:-----|
|macOS Xcode| [![Build status](https://ci.appveyor.com/api/projects/status/42rpdmi560qotdfb/branch/main?svg=true)](https://ci.appveyor.com/project/ChristianAhrens/mema-macos) | [![Build status](https://ci.appveyor.com/api/projects/status/v7uy57s7peoiqw71/branch/main?svg=true)](https://ci.appveyor.com/project/ChristianAhrens/memamo-macos) |
|Windows Visual Studio| [![Build status](https://ci.appveyor.com/api/projects/status/pth9bd9r42l7n42r/branch/main?svg=true)](https://ci.appveyor.com/project/ChristianAhrens/mema-windows) | [![Build status](https://ci.appveyor.com/api/projects/status/jjb3xuxm8oe94fc6/branch/main?svg=true)](https://ci.appveyor.com/project/ChristianAhrens/memamo-windows) |
|Linux makefile| [![Build status](https://ci.appveyor.com/api/projects/status/4mkms53fa8acra9d/branch/main?svg=true)](https://ci.appveyor.com/project/ChristianAhrens/mema-linux) | [![Build status](https://ci.appveyor.com/api/projects/status/uraaj3kbmsce52tt/branch/main?svg=true)](https://ci.appveyor.com/project/ChristianAhrens/memamo-linux) |


<a name="toc" />

## Table of contents

* [Introduction](#introduction)
  * [Mema and Mema.Mo](#MemaNMo)
  * [Mema UI](#MemaUI)
  * [Mema.Mo UI](#MemaMoUI)
* [How to build the tools](#howtobuild)
  * [Mema](#buildMema)
  * [Mema.Mo](#buildMemaMo)
    * [Note on running on RaspberryPiOS Bullseye](#runonbullseye)
* [Usecase: Studio sidecar monitoring](#rackmonitoringusecase)
* [Usecase: Mobile recording monitoring](#mobilerecordingusecase)
* [App Architecture](#architectureoverview)


<a name="introduction" />

## Introduction

Mema (MenubarMatrix) is a project initially created to try out if a macOS menubar tool based on JUCE C++ framework can be created, that provides audio matrix routing functionality - e.g. to route [BlackHole](https://github.com/ExistentialAudio/BlackHole) 16ch virtual device to system output to overcome AppleMusic behaviour on macOS to only play out higher order surround sound formats when exactly 16 output channels are presented by selected system audio output device.

Since version 0.4.0 the optional loading of an audio processing plug-in is supported to process the incoming audio before it is feed into the routing matrix. Platform dependant, the usual VST, VST3, AU, LADSPA, LV2 plug-in formats are supported and their respective editor user interfaces can be used as a separate window. _A usecase for this could be an n-input to m-output upmix plug-in that in combination with BlackHole can serve to process stereo macOS system audio output to a 7.1.4 speaker system._

It is accompanied by a separate tool Mema.Mo (MenubarMatrixMonitor) to monitor the audio IO as levelmeters via network. It connects to Mema through a TCP connection and supports discovering the available instances through a multicast service announcement done by Mema.

Its sourcecode and prebuilt binaries are made publicly available to enable interested users to experiment, extend and create own adaptations.

Use what is provided here at your own risk!

<a name="MemaNMo" />

### Mema and Mema.Mo

![Showreel.003.png](Resources/Documentation/Showreel/Showreel.003.png "Mema and -Monitor in action")

<a name="MemaUI" />

### Mema UI details

![Showreel.004.png](Resources/Documentation/Showreel/Showreel.004.png "Mema UI")

#### Mema UI plug-in handling details

![Showreel.005.png](Resources/Documentation/Showreel/Showreel.005.png "Mema UI plug-in handling")

<a name="MemaMoUI" />

### Mema.Mo UI details

![Showreel.006.png](Resources/Documentation/Showreel/Showreel.006.png "Mema.Mo UI")

#### Mema.Mo UI output format visualization details

![Showreel.007.png](Resources/Documentation/Showreel/Showreel.007.png "Mema.Mo UI output formats")

#### Supported UI coloring

![Showreel.008.png](Resources/Documentation/Showreel/Showreel.008.png "UI colouring options")


<a name="howtobuild" />

## How to build the tools

Mema and Mema.Mo are based on JUCE C++ framework, which is a submodule of this repository.

JUCE's Projucer tool can either be used from a local installation or from within the submodule (submodules/JUCE/extras/Projucer).

<a name="buildMema" />

### Mema

[Mema Projucer project](Mema.jucer) file can be found in repository root directory.

In [macOS buildscripts](Resources/Deployment/macOS), shellscripts for automated building of the app, dmg and notarization are kept. These require a properly prepared machine to run on (signing certificates, provisioning profiles, notarization cretentials).

In [iOS buildscripts](Resources/Deployment/iOS), shellscripts for automated building of the app and updloading to the appstore are kept. These require a properly prepared machine to run on (appstore cretentials).

In [Windows buildscripts](Resources/Deployment/Windows), bash scripts for automated building of the app and installer (Innosetup based) are kept. These require a properly prepared machine to run on (innosetup installation).

In [Linux buildscripts](Resources/Deployment/Linux), shell scripts for automated building of the app are kept. These are aimed at building on Debian/Ubuntu/RaspberryPiOS and TRY to collect the required dev packages via apt packetmanager automatically.

<a name="buildMemaMo" />

### Mema.Mo

[Mema.Mo Projucer project](MemaMo/MemaMo.jucer) file can be found in /MemaMo subdirectory .

In [macOS buildscripts](Resources/Deployment/macOS), shellscripts for automated building of the app, dmg and notarization are kept. These require a properly prepared machine to run on (signing certificates, provisioning profiles, notarization cretentials).

In [iOS buildscripts](Resources/Deployment/iOS), shellscripts for automated building of the app and updloading to the appstore are kept. These require a properly prepared machine to run on (appstore cretentials).

In [Windows buildscripts](Resources/Deployment/Windows), bash scripts for automated building of the app and installer (Innosetup based) are kept. These require a properly prepared machine to run on (innosetup installation).

In [Linux buildscripts](Resources/Deployment/Linux), shell scripts for automated building of the app are kept. These are aimed at building on Debian/Ubuntu/RaspberryPiOS and TRY to collect the required dev packages via apt packetmanager automatically.

<a name="runonbullseye" />

#### Building and running Mema.Mo on RaspberryPiOS Bullseye

The build script `build_MemaMo_RaspberryPIOS.sh` in `Resources/Deployment/Linux` can be used on a vanilla installation of RaspberryPi OS to build the tool.

On RaspberriPi 3B it is required to run the build without graphical interface, to avoid the build failing due to going out of memory (e.g. `sudo raspi-config` -> System Options -> Boot -> Console Autologin).

The build result can be run in kind of a kiosk configuration by changing the system to not start the desktop session when running Xserver, but instead run Mema.Mo directly in the X session. To do this, edit or create `.xsession` in user home and simply add a line
```
exec <PATH_TO_REPO>/Mema/MemaMo/Builds/LinuxMakefile/build/MemaMo
```
Then configure the system to auto login to x session (e.g. `sudo raspi-config` -> System Options -> Boot -> Desktop Autologin).

This does only work up to Raspberry Pi OS Bullseye versions, that use X server as window manager. From Bookworm on, Wayland is used that requires differing approaches.

<a name="rackmonitoringusecase" />

## Usecase: Studio rack monitoring

![Showreel.009.png](Resources/Documentation/Showreel/Showreel.009.png "Homestudio setup")

![Showreel.010.png](Resources/Documentation/Showreel/Showreel.010.png "RaspberryPi rack DIY device")

![Studio_rack_monitoring_usecase.png](Resources/Documentation/Studio_rack_monitoring_usecase.png "Homestudio setup signalflow schematic")

* Mema on macOS
  * BlackHole 16ch used to route signal from LogicPro, Apple Music, etc. to Mema
  * Output to Allen&Heath QU-16 22ch audio driver interface
* Mema.Mo on DIY 19" rack display, based on RaspberryPi (32bit RaspberryPiOS, Bullseye)
  * 16 audio input channel metering visible
  * 22 audio output channel metering visible


<a name="mobilerecordingusecase" />

## Usecase: Mobile recording monitoring

![Showreel.011.png](Resources/Documentation/Showreel/Showreel.011.png "Mobile rig")

* Mema on macOS
  * BlackHole 16ch used to route signal from LogicPro, Apple Music, etc. to Mema
  * Output to stereo audio driver interface
* Mema.Mo on iPadOS in Stagemanager mode
  * 16 audio input channel metering visible
  * 2 audio output channel metering visible


<a name="architectureoverview" />

## App Architecture

_T.B.D._