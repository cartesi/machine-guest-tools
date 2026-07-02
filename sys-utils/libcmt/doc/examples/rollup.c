#include "libcmt/rollup.h"
#include "libcmt/codec.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // rollup gets initialized
    cmt_buf_t tx;
    cmt_rollup_t rollup;
    if (cmt_rollup_init(&rollup, &tx)) {
        return EXIT_FAILURE;
    }

    for (;;) {
        cmt_buf_t rx = {};
        // Accepts the current input, then block waiting for the next input,
        //
        // Unblock when there is an input available, `rx` points to contents.
        // data is available until the next call to wait for input.
        switch (cmt_rollup_wait_for_input(&rollup, true, &rx)) {
        case HTIF_YIELD_REASON_ADVANCE: {
            cmt_evm_advance_args_t advance;
            if (cmt_evm_advance_decode(rx, &advance) < 0) {
                return EXIT_FAILURE;
            }

            cmt_notice_args_t notice = {
                    .payload = advance.payload,
            };
            cmt_buf_t out = {0};
            if (cmt_notice_encode(tx, &out, &notice) < 0) {
                return EXIT_FAILURE;
            }

            // message was already encoded in the tx buffer,
            // so we can use zero-copy mode emission.
            if (cmt_rollup_emit_output(&rollup, out) < 0) {
                (void)fprintf(stderr, "Error emitting report\n");
                break;
            }
            break;
        }
        case HTIF_YIELD_REASON_INSPECT:
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    return 0;
}
