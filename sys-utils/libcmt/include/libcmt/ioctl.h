/* Copyright Cartesi and individual authors (see AUTHORS)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file
 * @defgroup libcmt_ioctl Cartesi Machine cmio ioctl interface
 * ABI definitions of the Cartesi Machine `cmio` kernel driver.
 *
 * The structures and command numbers below are copied verbatim from the Linux
 * kernel UAPI header `<linux/cartesi/cmio.h>` and must be kept in sync with it.
 * Vendoring them here allows libcmt to be compiled (and cross-compiled) without
 * requiring the Cartesi kernel headers package to be installed in the sysroot.
 *
 * The ioctl command numbers are precomputed and assume the @b asm-generic
 * `ioctl` encoding.
 *
 * @ingroup libcmt
 * @{ */
#ifndef CMT_IOCTL_H
#define CMT_IOCTL_H

#include <stdint.h>

/** A `cmio` shared memory region as reported by the kernel driver */
struct cmt_ioctl_cmio_buffer {
	uint64_t data;   /**< physical address of the memory region */
	uint64_t length; /**< length of the memory region in bytes */
};

/** Layout of the tx and rx buffers returned by @ref CMT_IOCTL_CMIO_SETUP */
struct cmt_ioctl_cmio_setup {
	struct cmt_ioctl_cmio_buffer tx; /**< transmit buffer (guest to emulator) */
	struct cmt_ioctl_cmio_buffer rx; /**< receive buffer (emulator to guest) */
};

/** Return a @ref cmt_ioctl_cmio_setup structure filled with tx and rx buffer
 * details. Use these values to mmap them into the user-space.
 *
 * @note Equivalent to the kernel's `_IOR(0xd3, 0, struct cmio_setup)`.
 *
 * @return
 * - 0 on success.
 * - -1 on error and errno is set. */
#define CMT_IOCTL_CMIO_SETUP 0x8020d300UL

/** Yield the machine execution and transfer control back to the emulator.
 *
 * @note Equivalent to the kernel's `_IOWR(0xd3, 1, uint64_t)`.
 *
 * @return
 * - 0 on success.
 * - -1 on error and errno is set. */
#define CMT_IOCTL_CMIO_YIELD 0xc008d301UL

#endif /* CMT_IOCTL_H */
/** @} */
