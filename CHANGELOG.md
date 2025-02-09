# Mema Changelog
All notable changes to Mema will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added

### Changed

### Fixed

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
