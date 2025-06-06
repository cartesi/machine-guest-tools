# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.17.1] - 2025-05-27
### Added
- Added strace to guest-tools rootfs

### Fixed
- Fixed non-deterministic behavior when creating the dapp user

## [0.17.0] - 2025-04-25
### Added
- Allow compiling inside riscv64 environment without cross compilation
- Allow to install all tools with `make install`
- Added delegate call voucher to rollup tools and libcmt

### Changed
- Bump dependencies versions
- Generate `rootfs-tools.ext2.html` with licenses of all installed packages
- Bump Ubuntu to 24.04 LTS
- Rename repository to machine-guest-tools
- Simplified build system to make packaging easier
- Binaries are not automatically stripped anymore, this should be done when packaging
- Remove all references to `cartesi/toolchain` Docker image
- Avoid using `cttyhack` in `cartesi-init` to support Alpine Linux
- Increased JsonConfig limit in `rollup-http-server`
- Removed pinning of package versions from all Dockerfiles

## [0.16.1] - 2024-08-12
### Fixed
- Fixed curl version on rootfs
- Fixed openapi spec dependency version in the CI test

## [0.16.0] - 2024-07-26
### Changed
- Updated xgenext2fs to v1.5.6
- Updated migrated from output unification v1 to v2
- Fixed inconsistencies in the http server api
- Updated CI actions versions

## [0.15.0] - 2024-04-19
### Added
- Implemented a new libcmt library to interface with the Cartesi machine device
- Added generic IO entrypoint to libcmt
- Added generic IO endpoint on rollup-http-server
- Allowed customization of init with `/etc/cartesi-init.d`
- Allowed init to run in empty filesystems
- Introduced a new libcmt development Debian package as a regular artifact in releases

### Changed
- Added `stty -onlcr` to cartesi-init
- Set default USER to dapp in init script
- Created dapp user when installing tools package
- Removed unnecessary PATH change from init script
- Added a value field to vouchers
- Updated kernel to v6.5.13-ctsi-1
- Renamed kernel device
- Implemented Rollup-HTTP based on libcmt
- Rewrote yield with libcmt
- Rewrote Rollup with libcmt
- Updated Rollup-HTTP Rust dependencies
- Bumped Rustc to 1.77.2
- Updated the version of some workflow actions
- Added BusyBox dependency to Deb package
- Updated xgenext2fs to v1.5.5

### Removed
- Removed old build-with-toolchain script

### Fixed
- Fixed a bug where Docker builds would incorrectly succeed even when commands in the `RUN` directive failed.

## [0.14.1] - 2023-12-18
### Fixed
- Fix rootfs.ext2 build, xxd pinned version was not available in the repository

## [0.14.0] - 2023-12-13
### Changed
- Make rootfs the default target
- Reorganized repository structure
- Use image-kernel Linux headers package
- Cross-compiled sys-utils
- Built rust binaries dependencies in a separate stage
- Cleaned up Dockerfile and Makefile
- Update toolchain to 0.16.0

### Added
- Added support for building rootfs.ext2 (breaking change)
- Added dhrystone and whetstone benchmarks on fs
- Added extra packages to rootfs
- Cross-compiled rust application binaries

### Removed
- Removed example directory

## [0.13.0] - 2023-10-10
### Changed
- Updated tools version in example

### Added
- Added new init system using init and entrypoint from device tree
- Added a sha512sums.txt to release artifacts
- Added support for forwarding application exit status

## [0.12.0] - 2023-08-14
### Changed
- Cache build in CI
- Generate binary deb with tools
- Moved sbin/init -> opt/cartesi/sbin/init
- Updated license/copyright notice in all source code
- Update toolchain to 0.15.0

## [0.11.0] - 2023-04-19
### Changed
- Fixed rollups-http-server when it receives an inspect with 0 bytes
- Updated linux-sources to v5.15.63-ctsi-2
- Update toolchain to 0.14.0
- Renamed voucher field: address -> destination

## [0.10.0] - 2023-02-15
### Changed
- Moved skel/ from rootfs to this repository
- Build tools in a riscv64/ubuntu:22.04 image and pack them with skel/
- Compile tools using compressed instructions
- Updated toolchain to v0.13.0
- Updated CI image to Ubuntu 22.04

## [0.9.0] - 2022-11-17
### Changed
- Compile tools using floating-point instructions
- Update toolchain to v0.12.0

## [0.8.0] - 2022-08-29
### Changed
- Fix rollup-http-server printout
- Remove Dehash command line tool
- Remove timestamp from logs
- Update toolchain to v0.11.0

## [0.7.0] - 2022-06-27
### Changed
- Improved error handling in rollup command line tool
- Fixed indentation in rollup command line tool
- Removed explicit strip feature from Cargo.toml files
- Moved dapp initialization from rollup-init to rollup-http-server
- Simplified rollup-init script to only call rollup-http-server
- Handled rollup-http-server exit status in rollup-init
- Handled dapp exit status in rollup-http-server
- Updated regex crate on rollup-http-server and echo-dapp to address CVE-2022-24713

## [0.6.0] - 2022-04-20
### Added
- Added new rollup command line tool
- Added ioctl-echo-loop and rollup tools to CI
- Added rollup-exception handling to ioctl-echo-loop
- Added --reject-inspects as an option to ioctl-echo-loop and echo dapp
- Added exception to rollup-http-server and echo-dapp
- Added rollup http server and echo dapp CI build
- Added tests for rollup http server and echo dapp client

### Changed
- Update toolchain to v0.9.0
- Enabled rollup-exception in rollup
- Updated yield to the new yield\_request format
- Removed --ack from yield
- Rename http-dispatcher to rollup-http-server
- Switch rollup-http-server to server only and echo-dapp to client only implementation

## [0.5.1] - 2022-02-22
### Changed
- Lock dependencies versions using Cargo.lock on http-dispatcher and echo-dapp
- Fix --reject option on ioctl-echo-loop

## [0.5.0] - 2022-01-13
### Changed
- Updated http-dispatcher and echo-dapp according to openapi-interfaces changes
- Remove index from voucher and notice http-requests on http-dispatcher and echo-dapp
- Fixed http-dispatcher payload handling

## [0.4.1] - 2021-12-29
### Changed
- Fixed deadlock on http-dispatcher

## [0.4.0] - 2021-12-21
### Added
- New ioctl-echo-loop tool to test rollups
- New rollup http-dispatcher
- New echo dapp based on the http-dispatcher architecture

### Changed
- Updated yield command line tool to be compatible with Linux kernel v5.5.19-ctsi-3

## [Previous Versions]
- [0.3.0]
- [0.2.0]
- [0.1.0]

[Unreleased]: https://github.com/cartesi/machine-guest-tools/compare/v0.17.1...HEAD
[0.17.1]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.17.1
[0.17.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.17.0
[0.16.1]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.16.1
[0.16.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.16.0
[0.15.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.15.0
[0.14.1]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.14.1
[0.14.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.14.0
[0.13.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.13.0
[0.12.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.12.0
[0.11.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.11.0
[0.10.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.10.0
[0.9.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.9.0
[0.8.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.8.0
[0.7.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.7.0
[0.6.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.6.0
[0.5.1]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.5.1
[0.5.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.5.0
[0.4.1]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.4.1
[0.4.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.4.0
[0.3.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.3.0
[0.2.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.2.0
[0.1.0]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.1.0
