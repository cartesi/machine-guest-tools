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
#include "libcmt/rollup.h"
#include "libcmt/abi.h"
#include "libcmt/buf.h"
#include "libcmt/merkle.h"
#include "libcmt/util.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

int cmt_rollup_init(cmt_rollup_t *me, cmt_buf_t *tx) {
    if (!me) {
        return -EINVAL;
    }

    int rc = CMT_DBG(cmt_io_init(me->io));
    if (rc) {
        return rc;
    }

    if (tx) {
        *tx = *me->io->ioctl.tx;
    }

    cmt_merkle_init(me->merkle);
    me->finish_leaf_count = UINT64_C(-1);
    memset(me->finish_root_hash, 0, sizeof(me->finish_root_hash));
    return 0;
}

void cmt_rollup_fini(cmt_rollup_t *me) {
    if (!me) {
        return;
    }

    cmt_io_fini(me->io);
    cmt_merkle_fini(me->merkle);
}

cmt_io_t *cmt_rollup_get_io(cmt_rollup_t *me) {
    if (!me) {
        return NULL;
    }
    return me->io;
}

cmt_merkle_t *cmt_rollup_get_merkle(cmt_rollup_t *me) {
    if (!me) {
        return NULL;
    }
    return me->merkle;
}

int cmt_rollup_emit_output(cmt_rollup_t *me, cmt_buf_t data) {
    if (!me) {
        return -EINVAL;
    }
    cmt_buf_t tx[1] = {cmt_io_get_tx(me->io)};
    size_t length = cmt_buf_length(data);
    if (length > cmt_buf_length(*tx)) {
        return -ENOBUFS;
    }
    if (cmt_buf_begin(data) != tx->begin && length > 0) {
        memcpy(tx->begin, cmt_buf_begin(data), length);
    } else if (cmt_util_debug_enabled()) {
        (void) fprintf(stderr, "zero-copy output (%zu)\n", length);
    }

    struct cmt_io_yield req[1] = {{
        .dev = HTIF_DEVICE_YIELD,
        .cmd = HTIF_YIELD_CMD_AUTOMATIC,
        .reason = HTIF_YIELD_AUTOMATIC_REASON_TX_OUTPUT,
        .data = length,
    }};
    int rc = CMT_DBG(cmt_io_yield(me->io, req));
    if (rc) {
        return rc;
    }

    rc = cmt_merkle_push_back_data(me->merkle, length, tx->begin);
    if (rc) {
        return rc;
    }

    return 0;
}

int cmt_rollup_emit_report(cmt_rollup_t *me, cmt_buf_t data) {
    if (!me) {
        return -EINVAL;
    }
    cmt_buf_t tx[1] = {cmt_io_get_tx(me->io)};
    size_t length = cmt_buf_length(data);
    if (length > cmt_buf_length(*tx)) {
        return -ENOBUFS;
    }
    if (cmt_buf_begin(data) != tx->begin && length > 0) {
        memcpy(tx->begin, cmt_buf_begin(data), length);
    } else if (cmt_util_debug_enabled()) {
        (void) fprintf(stderr, "zero-copy report (%zu)\n", length);
    }

    struct cmt_io_yield req[1] = {{
        .dev = HTIF_DEVICE_YIELD,
        .cmd = HTIF_YIELD_CMD_AUTOMATIC,
        .reason = HTIF_YIELD_AUTOMATIC_REASON_TX_REPORT,
        .data = length,
    }};
    return CMT_DBG(cmt_io_yield(me->io, req));
}

int cmt_rollup_emit_exception(cmt_rollup_t *me, cmt_buf_t data) {
    if (!me) {
        return -EINVAL;
    }

    cmt_buf_t tx[1] = {cmt_io_get_tx(me->io)};
    size_t length = cmt_buf_length(data);
    if (length > cmt_buf_length(*tx)) {
        return -ENOBUFS;
    }
    if (cmt_buf_begin(data) != tx->begin && length > 0) {
        memcpy(tx->begin, cmt_buf_begin(data), length);
    } else if (cmt_util_debug_enabled()) {
        (void) fprintf(stderr, "zero-copy exception (%zu)\n", length);
    }

    struct cmt_io_yield req[1] = {{
        .dev = HTIF_DEVICE_YIELD,
        .cmd = HTIF_YIELD_CMD_MANUAL,
        .reason = HTIF_YIELD_MANUAL_REASON_TX_EXCEPTION,
        .data = length,
    }};
    return CMT_DBG(cmt_io_yield(me->io, req));
}

static int accepted(union cmt_io *io, uint32_t *n) {
    struct cmt_io_yield req[1] = {{
        .dev = HTIF_DEVICE_YIELD,
        .cmd = HTIF_YIELD_CMD_MANUAL,
        .reason = HTIF_YIELD_MANUAL_REASON_RX_ACCEPTED,
        .data = *n,
    }};
    int rc = CMT_DBG(cmt_io_yield(io, req));
    if (rc) {
        return rc;
    }

    *n = req->data;
    return req->reason;
}

static int revert(union cmt_io *io) {
    struct cmt_io_yield req[1] = {{
        .dev = HTIF_DEVICE_YIELD,
        .cmd = HTIF_YIELD_CMD_MANUAL,
        .reason = HTIF_YIELD_MANUAL_REASON_RX_REJECTED,
        .data = 0,
    }};
    return CMT_DBG(cmt_io_yield(io, req));
}

int cmt_rollup_wait_for_input(cmt_rollup_t *me, bool accept, cmt_buf_t *out) {
    if (!me) {
        return -EINVAL;
    }
    if (!out) {
        return -EINVAL;
    }

    if (!accept) {
        return revert(me->io); /* revert should not return! */
    }

    // use cached root hash if there are no new outputs
    uint64_t leaf_count = cmt_merkle_get_leaf_count(me->merkle);
    if (me->finish_leaf_count != leaf_count) {
        cmt_merkle_get_root_hash(me->merkle, me->finish_root_hash);
        me->finish_leaf_count = leaf_count;
    }

    cmt_buf_t tx[1] = {cmt_io_get_tx(me->io)};
    cmt_buf_t rx[1] = {cmt_io_get_rx(me->io)};
    memcpy(tx->begin, me->finish_root_hash, CMT_ABI_U256_LENGTH);
    uint32_t data = CMT_ABI_U256_LENGTH;
    int reason = accepted(me->io, &data);
    if (reason < 0) {
        return reason;
    }
    if (cmt_buf_length(*rx) < data) {
        return -ENOBUFS;
    }
    *out = cmt_buf_make(data, cmt_io_get_rx(me->io).begin);
    return reason;
}

int cmt_rollup_progress(cmt_rollup_t *me, uint32_t progress) {
    if (!me) {
        return -EINVAL;
    }
    cmt_io_yield_t req[1] = {{
        .dev = HTIF_DEVICE_YIELD,
        .cmd = HTIF_YIELD_CMD_AUTOMATIC,
        .reason = HTIF_YIELD_AUTOMATIC_REASON_PROGRESS,
        .data = progress,
    }};
    return CMT_DBG(cmt_io_yield(me->io, req));
}
