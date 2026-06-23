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
#include "libcmt/io.h"
#include "libcmt/util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** track the number of open "devices". Mimic the kernel driver behavior by limiting it to 1 */
static int open_count = 0; // NOLINT

int cmt_io_init(cmt_io_t *_me) {
    if (!_me) {
        return -EINVAL;
    }
    if (open_count) {
        return -EBUSY;
    }

    open_count++;
    cmt_io_mock_t *me = &_me->mock;

    size_t tx_length = 2U << 20; // 2MB
    size_t rx_length = 2U << 20; // 2MB
    *me->tx = cmt_buf_make(tx_length, calloc(tx_length, 1));
    *me->rx = cmt_buf_make(rx_length, calloc(rx_length, 1));

    if (!me->tx->begin || !me->rx->begin) {
        free(me->tx->begin);
        free(me->rx->begin);
        return -ENOMEM;
    }

    char *inputs = getenv("CMT_INPUTS");
    if (inputs) {
        me->inputs_left = cmt_buf_make(strlen(inputs), inputs);
    } else {
        me->inputs_left = cmt_buf_make(0, "");
    }

    // in case the user writes something before loading any input
    strcpy(me->input_filename, "none");
    strcpy(me->input_fileext, ".bin");

    me->input_seq = 0;
    me->output_seq = 0;
    me->report_seq = 0;
    me->exception_seq = 0;

    return 0;
}

void cmt_io_fini(cmt_io_t *_me) {
    if (!_me) {
        return;
    }

    if (open_count == 0) {
        return;
    }

    open_count--;
    cmt_io_mock_t *me = &_me->mock;

    free(me->tx->begin);
    free(me->rx->begin);

    memset(_me, 0, sizeof(*_me));
}

cmt_buf_t cmt_io_get_tx(cmt_io_t *me) {
    cmt_buf_t empty = {NULL, NULL};
    if (!me) {
        return empty;
    }
    return *me->mock.tx;
}

cmt_buf_t cmt_io_get_rx(cmt_io_t *me) {
    cmt_buf_t empty = {NULL, NULL};
    if (!me) {
        return empty;
    }
    return *me->mock.rx;
}

static int load_next_input(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    cmt_buf_t current_input;
    char typestr[16] = {0};
    char filepath[128] = {0};
    if (!cmt_buf_split_by_comma(&current_input, &me->inputs_left)) {
        return -EINVAL;
    }

    // parse typestr as a string and then convert it to a number so we can detect errors
    if (sscanf((char *) current_input.begin, "%15[^:]:%127[^,]", typestr, filepath) != 2) {
        return -EINVAL;
    }
    char *end = NULL;
    unsigned long type = strtoul(typestr, &end, 10);
    if (*end != '\0' || type > UINT16_MAX) {
        return -EINVAL;
    }
    me->input_type = (int) type;

    size_t file_length = 0;
    int rc = cmt_util_read_whole_file(filepath, cmt_buf_length(*me->rx), me->rx->begin, &file_length);
    if (rc) {
        if (cmt_util_debug_enabled()) {
            (void) fprintf(stderr, "failed to load \"%s\". %s\n", filepath, strerror(-rc));
        }
        return rc;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    if (sscanf(filepath, " %127[^.]%15s", me->input_filename, me->input_fileext) != 2) {
        if (cmt_util_debug_enabled()) {
            (void) fprintf(stderr, "failed to parse filename: \"%s\"\n", filepath);
        }
        return -EINVAL;
    }
    rr->reason = me->input_type;
    rr->data = file_length;

    me->output_seq = 0;
    me->report_seq = 0;
    me->exception_seq = 0;

    if (cmt_util_debug_enabled()) {
        (void) fprintf(stderr, "processing filename: \"%s\" (%lu), type: %d\n", filepath, file_length, me->input_type);
    }
    return 0;
}

static int store_output(cmt_io_mock_t *me, const char *filepath, struct cmt_io_yield *rr) {
    if (rr->data > cmt_buf_length(*me->rx)) {
        return -ENOBUFS;
    }

    int rc = cmt_util_write_whole_file(filepath, rr->data, me->tx->begin);
    if (rc) {
        (void) fprintf(stderr, "failed to store \"%s\". %s\n", filepath, strerror(-rc));
        return rc;
    }
    if (cmt_util_debug_enabled()) {
        (void) fprintf(stderr, "wrote filename: \"%s\" (%u)\n", filepath, rr->data);
    }
    return 0;
}

static int store_next_output(cmt_io_mock_t *me, char *ns, int *seq, struct cmt_io_yield *rr) {
    char filepath[128 + 32 + 8 + 16];
    // NOLINTNEXTLINE(cert-err33-c, clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    snprintf(filepath, sizeof filepath, "%s.%s%d%s", me->input_filename, ns, (*seq)++, me->input_fileext);
    return store_output(me, filepath, rr);
}

static int mock_progress(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    (void) me;
    if (rr->cmd != HTIF_YIELD_CMD_AUTOMATIC) {
        (void) fprintf(stderr, "Expected cmd to be AUTOMATIC\n");
        return -EINVAL;
    }
    (void) fprintf(stderr, "Progress: %6.2f\n", (double) rr->data / 10);
    return 0;
}

static int mock_rx_accepted(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    if (rr->cmd != HTIF_YIELD_CMD_MANUAL) {
        (void) fprintf(stderr, "Expected cmd to be MANUAL\n");
        return -EINVAL;
    }
    if (me->input_seq++) { // skip the first
        char filepath[128 + 32 + 8 + 16];
        // NOLINTNEXTLINE(cert-err33-c, clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
        snprintf(filepath, sizeof filepath, "%s.outputs_root_hash%s", me->input_filename, me->input_fileext);
        int rc = store_output(me, filepath, rr);
        if (rc) {
            return rc;
        }
    }
    if (load_next_input(me, rr)) {
        return -ENODATA;
    }
    return 0;
}

static int mock_rx_rejected(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    if (rr->cmd != HTIF_YIELD_CMD_MANUAL) {
        (void) fprintf(stderr, "Expected cmd to be MANUAL\n");
        return -EINVAL;
    }
    (void) fprintf(stderr, "%s:%d no revert for the mock implementation\n", __FILE__, __LINE__);
    if (load_next_input(me, rr)) {
        return -ENODATA;
    }
    return -ENOSYS;
}

static int mock_tx_output(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    if (rr->cmd != HTIF_YIELD_CMD_AUTOMATIC) {
        (void) fprintf(stderr, "Expected cmd to be AUTOMATIC\n");
        return -EINVAL;
    }
    return store_next_output(me, "output-", &me->output_seq, rr);
}

static int mock_tx_report(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    if (rr->cmd != HTIF_YIELD_CMD_AUTOMATIC) {
        (void) fprintf(stderr, "Expected cmd to be AUTOMATIC\n");
        return -EINVAL;
    }
    return store_next_output(me, "report-", &me->report_seq, rr);
}

static int mock_tx_exception(cmt_io_mock_t *me, struct cmt_io_yield *rr) {
    if (rr->cmd != HTIF_YIELD_CMD_MANUAL) {
        (void) fprintf(stderr, "Expected cmd to be MANUAL\n");
        return -EINVAL;
    }
    return store_next_output(me, "exception-", &me->exception_seq, rr);
}

/* These behaviours are defined by the cartesi-machine emulator */
static int cmt_io_yield_inner(cmt_io_t *_me, struct cmt_io_yield *rr) {
    cmt_io_mock_t *me = &_me->mock;

    if (rr->cmd == HTIF_YIELD_CMD_MANUAL) {
        switch (rr->reason) {
            case HTIF_YIELD_MANUAL_REASON_RX_ACCEPTED:
                return mock_rx_accepted(me, rr);
            case HTIF_YIELD_MANUAL_REASON_RX_REJECTED:
                return mock_rx_rejected(me, rr);
            case HTIF_YIELD_MANUAL_REASON_TX_EXCEPTION:
                return mock_tx_exception(me, rr);
            default:
                return -EINVAL;
        }
    } else if (rr->cmd == HTIF_YIELD_CMD_AUTOMATIC) {
        switch (rr->reason) {
            case HTIF_YIELD_AUTOMATIC_REASON_PROGRESS:
                return mock_progress(me, rr);
            case HTIF_YIELD_AUTOMATIC_REASON_TX_OUTPUT:
                return mock_tx_output(me, rr);
            case HTIF_YIELD_AUTOMATIC_REASON_TX_REPORT:
                return mock_tx_report(me, rr);
            default:
                return -EINVAL;
        }
    } else {
        return -EINVAL;
    }

    return 0;
}

/* emulate io.c:cmt_io_yield behavior (go and check it does if you change it) */
int cmt_io_yield(cmt_io_t *_me, struct cmt_io_yield *rr) {
    if (!_me) {
        return -EINVAL;
    }
    if (!rr) {
        return -EINVAL;
    }

    bool debug = cmt_util_debug_enabled();
    if (debug) {
        (void) fprintf(stderr,
            "tohost {\n"
            "\t.dev = %d,\n"
            "\t.cmd = %d,\n"
            "\t.reason = %d,\n"
            "\t.data = %d,\n"
            "};\n",
            rr->dev, rr->cmd, rr->reason, rr->data);
    }
    int rc = cmt_io_yield_inner(_me, rr);
    if (rc) {
        return rc;
    }

    if (debug) {
        (void) fprintf(stderr,
            "fromhost {\n"
            "\t.dev = %d,\n"
            "\t.cmd = %d,\n"
            "\t.reason = %d,\n"
            "\t.data = %d,\n"
            "};\n",
            rr->dev, rr->cmd, rr->reason, rr->data);
    }
    return rc;
}
