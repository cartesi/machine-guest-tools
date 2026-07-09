#include "libcmt/abi.h"
#include "libcmt/buf.h"

int encode_bytes(cmt_buf_t *tx, uint32_t funsel, cmt_buf_t *payload0, cmt_buf_t *payload1)
{
    cmt_buf_t wr = *tx;
    cmt_abi_dyn_state_t state[2];
    cmt_abi_frame_t frame;

    // static section
    return cmt_abi_put_funsel(&wr, funsel)
    ||     cmt_abi_mark_frame(&wr, &frame)
    ||     cmt_abi_put_dyn_head(&wr, &state[0], &frame)
    ||     cmt_abi_put_dyn_head(&wr, &state[1], &frame)
    // dynamic section (must match static order)
    ||     cmt_abi_put_dyn_tail(&wr, &state[0], 1, *payload0)
    ||     cmt_abi_put_dyn_tail(&wr, &state[1], 1, *payload1);
}
