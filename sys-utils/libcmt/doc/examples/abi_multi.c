#include <errno.h>
#include <stdlib.h>

#include "libcmt/abi.h"
#include "libcmt/buf.h"
#include "libcmt/util.h"

// Echo(bytes)
#define ECHO CMT_ABI_FUNSEL(0x5f, 0x88, 0x6b, 0x86)

static ptrdiff_t encode_echo(cmt_buf_t *tx, uint32_t funsel, cmt_buf_t *payload) {
    cmt_buf_t wr = *tx;
    cmt_abi_dyn_state_t state;
    cmt_abi_frame_t frame;

    // static section
    if (CMT_DBG(cmt_abi_put_funsel(&wr, funsel))
    ||  CMT_DBG(cmt_abi_mark_frame(&wr, &frame))
    ||  CMT_DBG(cmt_abi_put_dyn_head(&wr, &state, &frame))
    // dynamic section
    ||  CMT_DBG(cmt_abi_put_dyn_tail(&wr, &state, 1, *payload))) {
        return -ENOBUFS;
    }
    return cmt_buf_begin(wr) - cmt_buf_begin(*tx);
}

static ptrdiff_t decode_echo(cmt_buf_t *rx, uint32_t funsel, cmt_buf_t *payload) {
    cmt_buf_t rd = *rx;
    cmt_abi_dyn_state_t state;
    cmt_abi_frame_t frame;

    // static section
    if (CMT_DBG(cmt_abi_check_funsel(&rd, funsel))
    ||  CMT_DBG(cmt_abi_mark_frame(&rd, &frame))
    ||  CMT_DBG(cmt_abi_get_dyn_head(&rd, &state, &frame))
    // dynamic section
    ||  CMT_DBG(cmt_abi_view_dyn_tail(&state, 1, payload))) {
        return -ENOBUFS;
    }
    return cmt_buf_begin(rd) - cmt_buf_begin(*rx);
}

int f(cmt_buf_t *wr, cmt_buf_t *rd)
{
    ptrdiff_t rc = 0;
    cmt_buf_t data;

    if (cmt_buf_length(rd) < 4) {
        return EXIT_FAILURE;
    }
    switch (cmt_abi_peek_funsel(rd)) {
        case ECHO:
            rc = decode_echo(rd, ECHO, &data);
            if (rc < 0) {
                return EXIT_FAILURE;
            }

            rc = encode_echo(wr, ECHO, &data);
            if (rc < 0) {
                return EXIT_FAILURE;
            }

            break;
        default:
                return EXIT_FAILURE;
    }
    return 0;
}
