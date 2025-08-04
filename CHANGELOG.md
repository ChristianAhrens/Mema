# Mema Changelog
All notable changes to Mema will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added

### Changed

### Fixed

## [0.6.3] 2025-08-04
### Added

### Changed
- Changed Mema.Re input position sharpness parameter to be a per-input parameter

### Fixed
- Fixed Mema.Mo and Mema.Re to be able to connect to Mema instances while running on the same UNIX host
- Fixed Mema to correctly forward client control data to other connected clients
- Fixed Mema.Re 2D surround field panner to distribute inputs in a full circle by default and preserve manually modified positions
- Fixed Mema.Re 2D surround field panner input position stacking order and painting
- Fixed Mema.Mo and Mema.Re to allow new Mema instance selection on manual disconnect

## [0.6.2] 2025-07-27
### Added
- Added Mema.Mo waveform visualizer option
- Added Mema.Re 2D surround field panner, incl. sharpness parameter

### Changed

### Fixed

## [0.6.1] 2025-07-07
### Added
- Added Mema.Mo and Mema.Re coexistance awareness to prompt the user with discovery inavailablity when running at the same time

### Changed
- Updated JUCE framework to 8.0.8

### Fixed
- Fixed Mema crash when currently used audio device is removed from host

## [0.6.0] 2025-07-02
### Added
- Added new Mema client app - Mema.Re

### Changed
- Added service-to-connect to info to UI when clients are waiting to connect to Mema

### Fixed
- Fixed AudioBufferMessage data serialization when sending to clients

## [0.5.1] 2025-05-24
### Added
- Added Mema.Mo persistent configuration to restore UI and connection state after restart or connection loss

### Changed

### Fixed
- Fixed Mema crash on darkmode change

## [0.5.0] 2025-05-04
### Added
- Added plug-in processing switchability pre/post matrix
- Added plug-in state saving to and restoring from xml configuration
- Added storing MemaProcessor processing config state to and restore from xml (crosspoints + mutes)
- Added button to reset matrix to unity and unmute all IOs to UI

### Changed
- Changed Mema to be a CalloutBox from menubar, incl. major code refactoring to enable this
- Changed macOS DiskImage deployment to generate custom image for mounting
- Changed Mema / Mema.Mo icons to comply with general app icon scheme

### Fixed
- Fixed automatic Mema.Mo app hibernation (esp. of interest on iOS)
- Fixed processing for small buffer sizes (buffers smaller than a centisecond worth auf samples)

## [0.4.2] 2025-02-09
### Added
- Added metering colour selection to Mema and Mema.Mo
- Added manual LookAndFeel selection to Mema in addtion to automatic
- Added level processing to Mema routing matrix
- Added routing matrix level control to Mema UI

### Changed

### Fixed
- Fixed occasional unexpected Mema app termination on macOS when Mema.Mo instances disconnect

## [0.4.1] 2025-01-19
### Added
- Added saving/loading active plugin information to/from configuration file
- Added button to toggle showing Mema as standalone window
- Added macOS taskbar options menu to toggle showing mema as standalone window
- Added 9.1.6 channel layout to supported monitoring formats of Mema.Mo

### Changed
- Changed Mema.Mo default style to dark
- Changed JUCE framework version to tag 8.0.6

### Fixed
- Fixed Mema visibility toggling by fixing grabbing keyboard focus when shown
- Fixed Mema on Linux by always showing as standalone window

## [0.4.0] 2025-01-05
### Added
- Added support for optional VST/VST3/AU/LADSPA/LV2 plugin processing of audio signal before handing over to in routing matrix to Mema

### Changed
- Changed Mema.Mo 2D Field visualization to use a 5px levelmeter basewith for when singled out channel levels would otherwise remain invisible

### Fixed
- Fixed Mema main UI popup positioning to avoid offscreen area for large routing matrix sizes
- Fixed and streamlined levelmetering to consistent -80dBFS...0dBFS

## [0.3.2] 2024-12-05
### Added
- Added configurable 2D surround field visualization to Mema.Mo

### Changed

### Fixed
- Fixed Mema crosspoint patch to not be reset to unity when new Mema.Mo connections are established

## [0.3.1] 2024-11-23
### Added
- Added automatic update detection

### Changed

### Fixed

## [0.3.0] 2024-11-16
### Added

### Changed
- Project is now called 'Mema' accompanied by 'Mema.Mo' app

### Fixed
- Fixed storing and loading audio IO setup to config file

## [0.2.2] 2024-11-13
### Added
- Added visualization of network connections load incl. ovl to MenuBarMatrix
- Added support for portrait UI layout to MenuBarMatrixMonitor

### Changed

### Fixed
- Fixed potential macOS silent crash by checking network connection connected state before sending

## [0.2.1] 2024-11-06
### Added
- Added support for multiple MenuBarMatrixMontor connections to single MenuBarMatrix instance
- Added network decoupling from UI incl. dedicated network load monitoring per connection

### Changed

### Fixed
- Fixed MenuBarMatrix service advertisement for Windows

## [0.2.0] 2024-11-01
### Added
- Added improved README contents
- Added AppVeyor CI for macOS/Windows/Linux

### Changed
- Changed macOS app Sandbox to on
- Improved performance

### Fixed
- Fixed macOS, iOS provisioning
- Fixed typo and UI sizing issues

## [0.1.1] 2024-10-26
### Added
- Added capability to manually select LookAndFeel to MenuBarMatrixMonitor
- Added capability to manually disconnect from host to MenuBarMatrixMonitor

### Changed

### Fixed
- Fixed automated build environment for macOS, Windows, Linux
- Fixed incorrect link to github in MenuBarMatrixMonitor 'about'

## [0.1.0] 2024-10-23
### Added
- Initially added MenuBarMatrix toolbar audio rooting tool, incl. TCP/IP remote monitoring interface
- Initially added MenuBarMatrixMonitor Desktop-, Mobile- and Embedded-App to connect to MenuBarMatrix instances to provide level metering

### Changed

### Fixed
