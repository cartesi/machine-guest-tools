#include "libcmt/rollup.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // rollup gets initialized
    cmt_rollup_t rollup;
    if (cmt_rollup_init(&rollup, NULL)) {
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
            const char report[] = "hello world";
            cmt_buf_t data = cmt_buf_make(sizeof(report)-1, report);
            if (cmt_rollup_emit_report(&rollup, data) < 0) {
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
