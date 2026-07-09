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

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "libcmt/rollup.h"
#include "libcmt/codec.h"

static void help(const char *progname) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "Where options are: \n"
        "  --notices=<n>                    replicate input in n notices (default: 0)\n"
        "  --call-vouchers=<n>              replicate input in n call vouchers (default: 0)\n"
        "  --erc1155-batch-transfers=<n>    replicate input in n ERC1155 batch transfers (default: 0)\n"
        "  --erc1155-transfers=<n>          replicate input in n ERC1155 transfers (default: 0)\n"
        "  --erc20-transfers=<n>            replicate input in n ERC20 transfers (default: 0)\n"
        "  --erc721-transfers=<n>           replicate input in n ERC721 transfers (default: 0)\n"
        "  --reports=<n>                    replicate input in n reports (default: 1)\n"
        "  --reject=<n>                     reject the nth input (default: -1)\n"
        "  --reject-inspects                reject all inspects\n"
        "  --exception=<n>                  cause an exception on the nth input (default: -1)\n"
        "  --verbose=<n>                    display information of structures (default: 0)\n",
        progname);

    exit(1);
}

struct parsed_args {
    unsigned call_voucher_count;
    unsigned notice_count;
    unsigned report_count;
    unsigned erc1155_batch_transfer_count;
    unsigned erc1155_transfer_count;
    unsigned erc20_transfer_count;
    unsigned erc721_transfer_count;
    unsigned verbose;
    unsigned reject;
    bool reject_inspects;
    unsigned exception;
};

static int parse_token(const char *s, const char *fmt, bool *yes) {
    return *yes |= strcmp(fmt, s) == 0;
}

static int parse_number(const char *s, const char *fmt, unsigned *number) {
    int end = 0;
    if (sscanf(s, fmt, number, &end) != 1 || s[end] != 0) {
        return 0;
    }
    return 1;
}

static void parse_args(int argc, char *argv[], struct parsed_args *args) {
    int i = 0;
    const char *progname = argv[0];

    memset(args, 0, sizeof(*args));
    args->report_count = 1;
    args->reject = -1;
    args->reject_inspects = 0;
    args->exception = -1;

    for (i = 1; i < argc; i++) {
        if (!parse_number(argv[i], "--call-vouchers=%u%n", &args->call_voucher_count) &&
            !parse_number(argv[i], "--notices=%u%n", &args->notice_count) &&
            !parse_number(argv[i], "--erc1155-batch-transfers=%u%n", &args->erc1155_batch_transfer_count) &&
            !parse_number(argv[i], "--erc1155-transfers=%u%n", &args->erc1155_transfer_count) &&
            !parse_number(argv[i], "--erc20-transfers=%u%n", &args->erc20_transfer_count) &&
            !parse_number(argv[i], "--erc721-transfers=%u%n", &args->erc721_transfer_count) &&
            !parse_number(argv[i], "--reports=%u%n", &args->report_count) &&
            !parse_number(argv[i], "--verbose=%u%n", &args->verbose) &&
            !parse_token(argv[i], "--reject-inspects", &args->reject_inspects) &&
            !parse_number(argv[i], "--exception=%u%n", &args->exception) &&
            !parse_number(argv[i], "--reject=%u%n", &args->reject)) {
            help(progname);
        }
    }
}

static int write_notices(cmt_rollup_t *me, unsigned count, cmt_buf_t *payload) {
    for (unsigned i = 0; i < count; i++) {
        cmt_buf_t tx = cmt_io_get_tx(cmt_rollup_get_io(me));
        cmt_buf_t out;
        cmt_notice_args_t notice = {
            .app_context = {.data[31] = 0xff},
            .payload = *payload,
        };
        if (cmt_notice_encode(tx, &out, &notice) < 0 ||
            cmt_rollup_emit_output(me, out) < 0) {
            return -1;
        }
    }
    return 0;
}

static int write_vouchers(cmt_rollup_t *me, unsigned count, cmt_abi_address_t *destination, cmt_buf_t *payload) {
    cmt_abi_u256_t value = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xad, 0xbe, 0xef,
    }};
    for (unsigned i = 0; i < count; i++) {
        cmt_buf_t tx = cmt_io_get_tx(cmt_rollup_get_io(me));
        cmt_buf_t out;
        cmt_call_voucher_args_t voucher = {
            .app_context = {.data[31] = 0xff},
            .destination = *destination,
            .value = value,
            .payload = *payload,
        };
        if (cmt_call_voucher_encode(tx, &out, &voucher) < 0 ||
            cmt_rollup_emit_output(me, out) < 0) {
            return -1;
        }
    }
    return 0;
}

static int write_reports(cmt_rollup_t *me, unsigned count, cmt_buf_t *payload) {
    for (unsigned i = 0; i < count; i++) {
        if (cmt_rollup_emit_report(me, *payload))
            return -1;
    }
    return 0;
}

static int write_erc1155_batch_transfers(cmt_rollup_t *me, unsigned count, cmt_abi_address_t *recipient) {
    static const uint8_t erc1155_batch_items_data[64] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    };
    cmt_abi_address_t token = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02,
    }};
    cmt_buf_t items = cmt_buf_make(64, erc1155_batch_items_data);
    for (unsigned i = 0; i < count; i++) {
        cmt_buf_t tx = cmt_io_get_tx(cmt_rollup_get_io(me));
        cmt_buf_t out;
        cmt_erc1155_batch_transfer_args_t transfer = {
            .app_context = {.data[31] = 0xff},
            .recipient = *recipient,
            .token = token,
            .items = items,
        };
        if (cmt_erc1155_batch_transfer_encode(tx, &out, &transfer) < 0 ||
            cmt_rollup_emit_output(me, out) < 0) {
            return -1;
        }
    }
    return 0;
}

static int write_erc1155_transfers(cmt_rollup_t *me, unsigned count, cmt_abi_address_t *recipient) {
    cmt_abi_address_t token = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02,
    }};
    cmt_abi_u256_t token_id = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    }};
    cmt_abi_u256_t value = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    }};
    for (unsigned i = 0; i < count; i++) {
        cmt_buf_t tx = cmt_io_get_tx(cmt_rollup_get_io(me));
        cmt_buf_t out;
        cmt_erc1155_transfer_args_t transfer = {
            .app_context = {.data[31] = 0xff},
            .recipient = *recipient,
            .token = token,
            .token_id = token_id,
            .value = value,
        };
        if (cmt_erc1155_transfer_encode(tx, &out, &transfer) < 0 ||
            cmt_rollup_emit_output(me, out) < 0) {
            return -1;
        }
    }
    return 0;
}

static int write_erc20_transfers(cmt_rollup_t *me, unsigned count, cmt_abi_address_t *recipient) {
    cmt_abi_address_t token = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02,
    }};
    cmt_abi_u256_t value = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    }};
    for (unsigned i = 0; i < count; i++) {
        cmt_buf_t tx = cmt_io_get_tx(cmt_rollup_get_io(me));
        cmt_buf_t out;
        cmt_erc20_transfer_args_t transfer = {
            .app_context = {.data[31] = 0xff},
            .recipient = *recipient,
            .token = token,
            .value = value,
        };
        if (cmt_erc20_transfer_encode(tx, &out, &transfer) < 0 ||
            cmt_rollup_emit_output(me, out) < 0) {
            return -1;
        }
    }
    return 0;
}

static int write_erc721_transfers(cmt_rollup_t *me, unsigned count, cmt_abi_address_t *recipient) {
    cmt_abi_address_t token = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02,
    }};
    cmt_abi_u256_t token_id = {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
    }};
    for (unsigned i = 0; i < count; i++) {
        cmt_buf_t tx = cmt_io_get_tx(cmt_rollup_get_io(me));
        cmt_buf_t out;
        cmt_erc721_transfer_args_t transfer = {
            .app_context = {.data[31] = 0xff},
            .recipient = *recipient,
            .token = token,
            .token_id = token_id,
        };
        if (cmt_erc721_transfer_encode(tx, &out, &transfer) < 0 ||
            cmt_rollup_emit_output(me, out) < 0) {
            return -1;
        }
    }
    return 0;
}

static int handle_advance_state_request(cmt_rollup_t *me, cmt_buf_t rx, struct parsed_args *args, uint64_t *index) {
    cmt_evm_advance_args_t advance;
    if (cmt_evm_advance_decode(rx, &advance) < 0) {
        return -1;
    }
    *index = advance.index;
    fprintf(stderr, "advance with index %d\n", (int) advance.index);
    if (write_vouchers(me, args->call_voucher_count, &advance.msg_sender, &advance.payload) != 0) {
        return -1;
    }
    if (write_notices(me, args->notice_count, &advance.payload) != 0) {
        return -1;
    }
    if (write_erc1155_batch_transfers(me, args->erc1155_batch_transfer_count, &advance.msg_sender) != 0) {
        return -1;
    }
    if (write_erc1155_transfers(me, args->erc1155_transfer_count, &advance.msg_sender) != 0) {
        return -1;
    }
    if (write_erc20_transfers(me, args->erc20_transfer_count, &advance.msg_sender) != 0) {
        return -1;
    }
    if (write_erc721_transfers(me, args->erc721_transfer_count, &advance.msg_sender) != 0) {
        return -1;
    }
    if (write_reports(me, args->report_count, &advance.payload) != 0) {
        return -1;
    }
    return 0;
}

static int handle_inspect_state_request(cmt_rollup_t *me, cmt_buf_t *rx, struct parsed_args *args) {
    if (write_reports(me, args->report_count, rx) != 0) {
        return -1;
    }
    return 0;
}

static int handle_request(cmt_rollup_t *me, cmt_rollup_req_type_t req_type, cmt_buf_t rx, struct parsed_args *args, uint64_t *index) {
    switch (req_type) {
        case HTIF_YIELD_REASON_ADVANCE:
            return handle_advance_state_request(me, rx, args, index);
        case HTIF_YIELD_REASON_INSPECT:
            return handle_inspect_state_request(me, &rx, args);
        default:
            /* unknown request type */
            fprintf(stderr, "Unknown request type %d\n", req_type);
            return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    cmt_rollup_t rollup;
    uint64_t advance_index = 0;
    cmt_buf_t rx;

    if (cmt_rollup_init(&rollup, NULL))
        return EXIT_FAILURE;

    struct parsed_args args;
    parse_args(argc, argv, &args);

    fprintf(stderr, "Echoing as %d voucher copies, %d notice copies, and %d report copies\n", args.call_voucher_count,
        args.notice_count, args.report_count);

    long req_type = cmt_rollup_wait_for_input(&rollup, true, &rx);
    if (req_type < 0) {
        exit(1);
    }

    for (;;) {
        if (handle_request(&rollup, req_type, rx, &args, &advance_index) != 0) {
            break;
        }

        bool reject_advance = (req_type == CMT_ROLLUP_REQ_TYPE_ADVANCE) && (args.reject == advance_index);
        bool reject_inspect = (req_type == CMT_ROLLUP_REQ_TYPE_INSPECT) && args.reject_inspects;
        bool throw_exception = (req_type == CMT_ROLLUP_REQ_TYPE_ADVANCE) && (args.exception == advance_index);

        if (throw_exception) {
            char msg[] = "exception";
            cmt_buf_t data = cmt_buf_make(sizeof(msg) - 1, msg);
            cmt_rollup_emit_exception(&rollup, data);
        }

        req_type = cmt_rollup_wait_for_input(&rollup, !(reject_advance || reject_inspect), &rx);
        if (req_type < 0) {
            break;
        }
    }

    cmt_rollup_fini(&rollup);
    fprintf(stderr, "Exiting...\n");
    return 0;
}
