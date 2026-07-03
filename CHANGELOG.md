# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Added `libcmt/codec.h` library for ABI encoding/decoding of EVM operations.
  All codec structs include an `app_context` (`cmt_abi_bytes32_t`) as the first field:
  - `cmt_call_voucher_args_t` / `cmt_call_voucher_encode` / `cmt_call_voucher_decode`
  - `cmt_notice_args_t` / `cmt_notice_encode` / `cmt_notice_decode`
  - `cmt_evm_advance_args_t` / `cmt_evm_advance_encode` / `cmt_evm_advance_decode`
  - `cmt_erc20_transfer_args_t` / `cmt_erc20_transfer_encode` / `cmt_erc20_transfer_decode`
  - `cmt_erc721_transfer_args_t` / `cmt_erc721_transfer_encode` / `cmt_erc721_transfer_decode`
  - `cmt_erc1155_transfer_args_t` / `cmt_erc1155_transfer_encode` / `cmt_erc1155_transfer_decode`
  - `cmt_erc1155_batch_transfer_args_t` / `cmt_erc1155_batch_transfer_encode` / `cmt_erc1155_batch_transfer_decode`
- Added `cmt_rollup_wait_for_input()` to `libcmt/rollup.h` for simplified input handling
- Added `cmt_rollup_get_io()` and `cmt_rollup_get_merkle()` accessor functions to `libcmt/rollup.h`
- Added `cmt_rollup_emit_output()`, `cmt_rollup_emit_report()`, and `cmt_rollup_emit_exception()` with zero-copy `cmt_buf_t` semantics
- Added `cmt_abi_bytes32_t`, `cmt_abi_frame_t`, and `cmt_abi_dyn_state_t` types to `libcmt/abi.h`
- Added `cmt_abi_put_bytesN` / `cmt_abi_get_bytesN` for encoding/decoding Solidity `bytes<M>` types
- Added `cmt_abi_reserve_dyn_tail` / `cmt_abi_commit_dyn_tail` for two-phase dynamic encoding
- Added `cmt_abi_view_dyn_tail` for zero-copy dynamic decoding
- Added `cmt_abi_get_dyn_tail` for copying dynamic data into user-provided buffers
- Added `cmt_abi_put_uint_be` / `cmt_abi_get_uint_be` for big-endian integer encoding/decoding
- Added `cmt_abi_put_uint256` / `cmt_abi_get_uint256` convenience functions
- Added `CMT_DBG` macro and `cmt_util_debug_enabled()` to `libcmt/util.h`
- Added output merkle hash caching: reuses root hash when leaf count hasn't changed

### Changed
- **ABI API overhaul** in `libcmt/abi.h`:
  - Renamed `cmt_abi_put_bytes_s` → `cmt_abi_put_dyn_head`, `cmt_abi_put_bytes_d` → `cmt_abi_put_dyn_tail`
  - Renamed `cmt_abi_get_bytes_s` → `cmt_abi_get_dyn_head`, `cmt_abi_get_bytes_d` → `cmt_abi_get_dyn_tail`
  - Renamed `cmt_abi_start_frame` → `cmt_abi_mark_frame`
  - ABI function parameter naming convention: `me` → `wr` (writer) / `rd` (reader)
- **Rollup API overhaul** in `libcmt/rollup.h`:
  - `cmt_rollup_init()` now takes an optional `tx` output parameter
  - Replaced `cmt_rollup_emit_voucher()` with `cmt_rollup_emit_output()` taking `cmt_buf_t`
  - Replaced `cmt_rollup_emit_notice()` with `cmt_rollup_emit_report()` taking `cmt_buf_t`
  - Rollup header no longer includes `abi.h` (ABI encoding moved to `codec.h`)
  - Output handling is zero-copy message start matches the tx buffer start
- Replaced `cmt_rollup_inspect_t`, `cmt_rollup_finish_t`, and `cmt_gio_t` with `cmt_rollup_req_type_t` enum and `cmt_rollup_wait_for_input()`
- Changed `cmt_rollup_advance_t` → `cmt_evm_advance_args_t` and moved to `libcmt/codec.h`
- Changed `cmt_io_driver_t` → `cmt_io_t`, `cmt_io_driver_mock_t` → `cmt_io_mock_t`, and `cmt_io_driver_ioctl_t` → `cmt_io_ioctl_t` in `libcmt/io.h`
- Removed `#include "abi.h"` from `libcmt/rollup.h`
- Updated `ioctl-echo-loop` to use the new libcmt codec API and `cmt_rollup_wait_for_input()`
- Updated `sys-utils/rollup/rollup.cpp` to use the new libcmt codec API
- Updated `sys-utils/yield/yield.c` to use the new `cmt_io_t` type

### Removed
- **rollup-http**: Removed the entire `rollup-http/` directory including `rollup-http-server`, `echo-dapp`, and `rollup-http-client`
- **Delegate call voucher**: Removed `cmt_rollup_emit_delegate_call_voucher()` from `libcmt/rollup.h`, the `delegate-call-voucher` command from `sys-utils/rollup/rollup.cpp`, and the `--delegate-call-vouchers` flag from `sys-utils/ioctl-echo-loop/ioctl-echo-loop.c`
- **GIO support**: Removed `cmt_gio_t` and `cmt_gio_request()` from `libcmt/rollup.h`
- Removed `cmt_rollup_read_advance_state()`, `cmt_rollup_read_inspect_state()`, and `cmt_rollup_finish()` (replaced by `cmt_rollup_wait_for_input()`)
- Removed `cmt_rollup_load_merkle()`, `cmt_rollup_save_merkle()`, and `cmt_rollup_reset_merkle()` from `libcmt/rollup.h`
- Removed raw encode/decode functions from `libcmt/abi.h`: `cmt_abi_encode_uint`, `cmt_abi_encode_uint_nr`, `cmt_abi_encode_uint_nn`, `cmt_abi_decode_uint`, `cmt_abi_decode_uint_nr`, `cmt_abi_decode_uint_nn`
- Removed `cmt_abi_peek_bytes_d` (replaced by `cmt_abi_view_dyn_tail`)

## [0.17.2] - 2025-10-21
### Changed
- Improved advances performance by caching the outputs Merkle hash when no new outputs are added

### Fixed
- Fix /etc/shadow- determinism when installing guest tools

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

[Unreleased]: https://github.com/cartesi/machine-guest-tools/compare/v0.17.2...HEAD
[0.17.2]: https://github.com/cartesi/machine-guest-tools/releases/tag/v0.17.2
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
