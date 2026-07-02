#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libcmt/abi.h>
#include <libcmt/buf.h>
#include <libcmt/codec.h>
#include <libcmt/util.h>

#include "data.h"

#define CMT_BUF_DECL(S, L) cmt_buf_t S[1] = {cmt_buf_make((L), (uint8_t[L]){0})}

static void cmt_call_voucher_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_call_voucher_0
    const char payload[] = "CallVoucher-0";
    cmt_call_voucher_args_t dec[1] = {0};
    cmt_call_voucher_args_t enc[1] = {{
        .app_context = {.data = {[31] = 255}},
        .destination = {.data[19] = 1},
        .value = {.data[31] = 2},
        .payload = cmt_buf_make(strlen(payload), payload),
    }};

    cmt_buf_t out = {0};
    assert(cmt_call_voucher_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_call_voucher_0));
    assert(memcmp(cmt_buf_begin(out), valid_call_voucher_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_call_voucher_decode(*rx, dec) == 0);
    assert(memcmp(&enc->destination, &dec->destination, 20) == 0);
    assert(memcmp(&enc->value, &dec->value, 32) == 0);
    assert(cmt_buf_length(enc->payload) == cmt_buf_length(dec->payload));
    assert(memcmp(cmt_buf_begin(enc->payload), cmt_buf_begin(dec->payload), 1) == 0);

    printf("test %s passed!\n", __func__);
}

static void cmt_erc1155_batch_transfer_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_erc11551_batch_transfer_0
    cmt_erc1155_batch_transfer_args_t dec[1] = {0};
    cmt_erc1155_batch_transfer_args_t enc[1] = {{
        .app_context = {.data = {[31] = 255}},
        .recipient = {.data[19] = 1},
        .token = {.data[19] = 2},
        .items = cmt_buf_make(64,
            (uint8_t[64]){
                [31] = 3,
                [63] = 4,
            }),
    }};

    cmt_buf_t out = {0};
    assert(cmt_erc1155_batch_transfer_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_erc11551_batch_transfer_0));
    assert(memcmp(cmt_buf_begin(out), valid_erc11551_batch_transfer_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_erc1155_batch_transfer_decode(*rx, dec) == 0);
    assert(memcmp(&enc->recipient, &dec->recipient, 20) == 0);
    assert(memcmp(&enc->token, &dec->token, 20) == 0);
    assert(cmt_buf_length(enc->items) == cmt_buf_length(dec->items));
    assert(memcmp(cmt_buf_begin(enc->items), cmt_buf_begin(dec->items), 64) == 0);

    printf("test %s passed!\n", __func__);
}

static void cmt_erc1155_transfer_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_erc11551_single_transfer_0
    cmt_erc1155_transfer_args_t dec[1] = {0};
    cmt_erc1155_transfer_args_t enc[1] = {{
        .app_context = {.data = {[31] = 255}},
        .recipient = {.data[19] = 1},
        .token = {.data[19] = 2},
        .token_id = {.data[31] = 3},
        .value = {.data[31] = 4},
    }};

    cmt_buf_t out = {0};
    assert(cmt_erc1155_transfer_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_erc11551_single_transfer_0));
    assert(memcmp(cmt_buf_begin(out), valid_erc11551_single_transfer_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_erc1155_transfer_decode(*rx, dec) == 0);
    assert(memcmp(&enc->recipient, &dec->recipient, 20) == 0);
    assert(memcmp(&enc->token, &dec->token, 20) == 0);
    assert(memcmp(&enc->token_id, &dec->token_id, 32) == 0);
    assert(memcmp(&enc->value, &dec->value, 32) == 0);

    printf("test %s passed!\n", __func__);
}

static void cmt_erc20_transfer_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_erc20_transfer_0
    cmt_erc20_transfer_args_t dec[1] = {0};
    cmt_erc20_transfer_args_t enc[1] = {{
        .app_context = {.data = {[31] = 255}},
        .recipient = {.data[19] = 1},
        .token = {.data[19] = 2},
        .value = {.data[31] = 3},
    }};

    cmt_buf_t out = {0};
    assert(cmt_erc20_transfer_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_erc20_transfer_0));
    assert(memcmp(cmt_buf_begin(out), valid_erc20_transfer_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_erc20_transfer_decode(*rx, dec) == 0);
    assert(memcmp(&enc->recipient, &dec->recipient, 20) == 0);
    assert(memcmp(&enc->token, &dec->token, 20) == 0);
    assert(memcmp(&enc->value, &dec->value, 32) == 0);

    printf("test %s passed!\n", __func__);
}

static void cmt_erc721_transfer_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_erc721_transfer_0
    cmt_erc721_transfer_args_t dec[1] = {0};
    cmt_erc721_transfer_args_t enc[1] = {{
        .app_context = {.data = {[31] = 255}},
        .recipient = {.data[19] = 1},
        .token = {.data[19] = 2},
        .token_id = {.data[31] = 3},
    }};

    cmt_buf_t out = {0};
    assert(cmt_erc721_transfer_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_erc721_transfer_0));
    assert(memcmp(cmt_buf_begin(out), valid_erc721_transfer_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_erc721_transfer_decode(*rx, dec) == 0);
    assert(memcmp(&enc->recipient, &dec->recipient, 20) == 0);
    assert(memcmp(&enc->token, &dec->token, 20) == 0);
    assert(memcmp(&enc->token_id, &dec->token_id, 32) == 0);

    printf("test %s passed!\n", __func__);
}

static void cmt_evm_advance_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_advance_0
    const char payload[] = "EvmAdvance-0";
    cmt_evm_advance_args_t dec[1] = {0};
    cmt_evm_advance_args_t enc[1] = {{
        .chain_id = 1,
        .app_contract = {.data[19] = 2},
        .msg_sender = {.data[19] = 3},
        .block_number = 4,
        .block_timestamp = 5,
        .prev_randao = {.data[31] = 6},
        .index = 7,
        .payload = cmt_buf_make(strlen(payload), payload),
    }};

    cmt_buf_t out = {0};
    assert(cmt_evm_advance_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_advance_0));
    assert(memcmp(cmt_buf_begin(out), valid_advance_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_evm_advance_decode(*rx, dec) == 0);
    assert(enc->chain_id == dec->chain_id);
    assert(memcmp(&enc->app_contract, &dec->app_contract, 20) == 0);
    assert(memcmp(&enc->msg_sender, &dec->msg_sender, 20) == 0);
    assert(enc->block_number == dec->block_number);
    assert(enc->block_timestamp == dec->block_timestamp);
    assert(memcmp(&enc->prev_randao, &dec->prev_randao, 32) == 0);
    assert(enc->index == dec->index);
    assert(cmt_buf_length(enc->payload) == cmt_buf_length(dec->payload));
    assert(memcmp(cmt_buf_begin(enc->payload), cmt_buf_begin(dec->payload), 1) == 0);

    printf("test %s passed!\n", __func__);
}

static void cmt_notice_test(void) {
    CMT_BUF_DECL(b, 1024);
    cmt_buf_t tx[1] = {*b};
    cmt_buf_t rx[1] = {*b};

    // must match valid_notice_0
    const char payload[] = "Notice-0";
    cmt_notice_args_t dec[1] = {0};
    cmt_notice_args_t enc[1] = {{
        .app_context = {.data = {[31] = 255}},
        .payload = cmt_buf_make(strlen(payload), payload),
    }};

    cmt_buf_t out = {0};
    assert(cmt_notice_encode(*tx, &out, enc) == 0);

    // match encoded vs golden sample
    assert(cmt_buf_length(out) == sizeof(valid_notice_0));
    assert(memcmp(cmt_buf_begin(out), valid_notice_0, cmt_buf_length(out)) == 0);

    // match decoded vs encoded
    assert(cmt_notice_decode(*rx, dec) == 0);
    assert(cmt_buf_length(enc->payload) == cmt_buf_length(dec->payload));
    assert(memcmp(cmt_buf_begin(enc->payload), cmt_buf_begin(dec->payload), 1) == 0);

    printf("test %s passed!\n", __func__);
}

int main(void) {
    setenv("CMT_DEBUG", "y", true);
    cmt_call_voucher_test();
    cmt_erc1155_batch_transfer_test();
    cmt_erc1155_transfer_test();
    cmt_erc20_transfer_test();
    cmt_erc721_transfer_test();
    cmt_evm_advance_test();
    cmt_notice_test();
    printf("all codec tests passed!\n");
    return 0;
}
