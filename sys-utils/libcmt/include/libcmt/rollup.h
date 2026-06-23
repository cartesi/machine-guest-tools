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
 * @defgroup libcmt_rollup rollup
 * Rollup abstraction layer
 *
 * Takes care of @ref libcmt_io interactions and @ref libcmt_merkle handling.
 *
 * Mocked version has support for simulating I/O via environment variables:
 * @p CMT_INPUTS="0:input.bin,..." and verbose output with @p CMT_DEBUG=yes.
 *
 * Lets look at some code:
 *
 * @include doc/examples/rollup.c
 *
 * @ingroup libcmt
 * @{ */
#ifndef CMT_ROLLUP_H
#define CMT_ROLLUP_H
#include "io.h"
#include "merkle.h"

typedef struct cmt_rollup {
    union cmt_io io[1];
    cmt_merkle_t merkle[1];

    // cache merkle values and repeat them on finish when the tree doesn't change
    uint8_t finish_root_hash[CMT_KECCAK_LENGTH];
    uint64_t finish_leaf_count;
} cmt_rollup_t;

/** Initialize a @ref cmt_rollup_t state.
 *
 * @param [in]  me uninitialized state
 * @param [out] tx optional; if not NULL, will be set to the underlying io tx buffer
 *                 for encoding. This allows the caller to access the tx buffer
 *                 without calling @ref cmt_io_get_tx.
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value | */
int cmt_rollup_init(cmt_rollup_t *me, cmt_buf_t *tx);

/** Finalize a @ref cmt_rollup_t state previously initialized with @ref
 * cmt_rollup_init
 *
 * @param [in] me    initialized state
 *
 * @note use of @p me after this call is undefined behavior. */
void cmt_rollup_fini(cmt_rollup_t *me);

/** Access the internal io driver for advanced use cases.
 *
 * @param [in] me    initialized state
 *
 * @return internal io driver state
 *
 */
cmt_io_t *cmt_rollup_get_io(cmt_rollup_t *me);

/** Access the internal merkle tree state for advanced use cases.
 *
 * @param [in] me    initialized state
 *
 * @return internal merkle tree state
 *
 */
cmt_merkle_t *cmt_rollup_get_merkle(cmt_rollup_t *me);

/** Emit a on chain verifiable output
 *
 * @param [in,out] me      initialized @ref cmt_rollup_t instance
 * @param [in]     data    message contents
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value | */
int cmt_rollup_emit_output(cmt_rollup_t *me, cmt_buf_t data);

/** Emit a report
 * @param [in,out] me      initialized cmt_rollup_t instance
 * @param [in]     data    message contents
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value | */
int cmt_rollup_emit_report(cmt_rollup_t *me, cmt_buf_t data);

/** Emit a exception
 * @param [in,out] me      initialized cmt_rollup_t instance
 * @param [in]     data    message contents
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value | */
int cmt_rollup_emit_exception(cmt_rollup_t *me, cmt_buf_t data);

/** Report progress
 *
 * @param [in,out] me        initialized cmt_rollup_t instance
 * @param [in]     progress  progress value to be set
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value | */
int cmt_rollup_progress(cmt_rollup_t *me, uint32_t progress);

/** Accept or Reject the current input; wait for the next input
 *
 * @param [in,out] me        initialized cmt_rollup_t instance
 * @param [in]     accept    true to accept current input, false to reject it
 * @param [out]    out       input data
 *
 * ```c
 * // example with error handling
 * int req_type = cmt_rollup_wait_for_input(R, true, rx);
 * if (req_type < 0) {
 *     return EXIT_FAILURE;
 * }
 * switch (req_type) {
 * case HTIF_YIELD_REASON_ADVANCE:
 *     break;
 * case HTIF_YIELD_REASON_INSPECT:
 *     break;
 * }
 * ```
 * @return
 * |                           |                               |
 * |--------------------------:|-------------------------------|
 * | HTIF_YIELD_REASON_INSPECT | inspect request               |
 * | HTIF_YIELD_REASON_INSPECT | advance request               |
 * |                       < 0 | `errno` value for description | */
int cmt_rollup_wait_for_input(cmt_rollup_t *me, bool accept, cmt_buf_t *out);

#endif /* CMT_ROLLUP_H */
