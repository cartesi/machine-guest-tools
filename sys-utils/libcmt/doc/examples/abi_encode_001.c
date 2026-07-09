#include "libcmt/abi.h"
#include "libcmt/buf.h"

int encode_bytes(cmt_buf_t *tx, uint32_t funsel, cmt_buf_t *payload) {
    cmt_buf_t wr = *tx;
    cmt_abi_dyn_state_t state;
    cmt_abi_frame_t frame;

    // static section
    return cmt_abi_put_funsel(&wr, funsel)
    ||     cmt_abi_mark_frame(&wr, &frame)
    ||     cmt_abi_put_dyn_head(&wr, &state, &frame)
    // dynamic section
    ||     cmt_abi_put_dyn_tail(&wr, &state, 1, *payload);
}
