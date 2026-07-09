#include "libcmt/abi.h"
#include "libcmt/buf.h"

int decode(cmt_buf_t *rx, uint32_t expected_funsel, cmt_abi_address_t *address, cmt_buf_t *payload0, cmt_buf_t *payload1) {
    cmt_buf_t rd = *rx;
    cmt_abi_frame_t frame;
    cmt_abi_dyn_state_t state[2];

    /* static section */
    return cmt_abi_check_funsel(&rd, expected_funsel)
    ||     cmt_abi_mark_frame(&rd, &frame)
    ||     cmt_abi_get_address(&rd, address)
    ||     cmt_abi_get_dyn_head(&rd, &state[0], &frame)
    ||     cmt_abi_get_dyn_head(&rd, &state[1], &frame)
    /* dynamic section */
    ||     cmt_abi_view_dyn_tail(&state[0], 1, payload0)
    ||     cmt_abi_view_dyn_tail(&state[1], 1, payload1)
    ;
}

