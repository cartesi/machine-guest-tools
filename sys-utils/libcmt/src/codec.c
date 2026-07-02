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
#include <libcmt/codec.h>

// -----------------------------------------------------------------------------
int cmt_call_voucher_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_call_voucher_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_call_voucher_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(wr, frame)) ||
        CMT_DBG(cmt_abi_put_bytesN(wr, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->destination)) ||
        CMT_DBG(cmt_abi_put_uint256(wr, &args->value)) ||
        CMT_DBG(cmt_abi_put_dyn_head(wr, state+0, frame)) ||
        CMT_DBG(cmt_abi_put_dyn_tail(wr, state+0, 1, args->payload))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_call_voucher_decode(cmt_buf_t rx, cmt_call_voucher_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_call_voucher_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(rd, frame)) ||
        CMT_DBG(cmt_abi_get_bytesN(rd, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->destination))||
        CMT_DBG(cmt_abi_get_uint256(rd, &args->value)) ||
        CMT_DBG(cmt_abi_get_dyn_head(rd, state+0, frame)) ||
        CMT_DBG(cmt_abi_view_dyn_tail(state+0, 1, &args->payload))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}

// -----------------------------------------------------------------------------
int cmt_erc1155_batch_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc1155_batch_transfer_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_erc1155_batch_transfer_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(wr, frame)) ||
        CMT_DBG(cmt_abi_put_bytesN(wr, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->recipient)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->token)) ||
        CMT_DBG(cmt_abi_put_dyn_head(wr, state+0, frame)) ||
        CMT_DBG(cmt_abi_put_dyn_tail(wr, state+0, 64, args->items))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_erc1155_batch_transfer_decode(cmt_buf_t rx, cmt_erc1155_batch_transfer_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_erc1155_batch_transfer_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(rd, frame)) ||
        CMT_DBG(cmt_abi_get_bytesN(rd, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->recipient)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->token)) ||
        CMT_DBG(cmt_abi_get_dyn_head(rd, state+0, frame)) ||
        CMT_DBG(cmt_abi_view_dyn_tail(state+0, 64, &args->items))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}

// -----------------------------------------------------------------------------
int cmt_erc1155_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc1155_transfer_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_erc1155_transfer_funsel)) ||
        CMT_DBG(cmt_abi_put_bytesN(wr, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->recipient)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->token)) ||
        CMT_DBG(cmt_abi_put_uint256(wr, &args->token_id)) ||
        CMT_DBG(cmt_abi_put_uint256(wr, &args->value))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_erc1155_transfer_decode(cmt_buf_t rx, cmt_erc1155_transfer_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_erc1155_transfer_funsel)) ||
        CMT_DBG(cmt_abi_get_bytesN(rd, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->recipient)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->token))||
        CMT_DBG(cmt_abi_get_uint256(rd, &args->token_id))||
        CMT_DBG(cmt_abi_get_uint256(rd, &args->value))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}

// -----------------------------------------------------------------------------
int cmt_erc20_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc20_transfer_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_erc20_transfer_funsel)) ||
        CMT_DBG(cmt_abi_put_bytesN(wr, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->recipient)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->token)) ||
        CMT_DBG(cmt_abi_put_uint256(wr, &args->value))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_erc20_transfer_decode(cmt_buf_t rx, cmt_erc20_transfer_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_erc20_transfer_funsel)) ||
        CMT_DBG(cmt_abi_get_bytesN(rd, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->recipient)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->token))||
        CMT_DBG(cmt_abi_get_uint256(rd, &args->value))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}

// -----------------------------------------------------------------------------
int cmt_erc721_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc721_transfer_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_erc721_transfer_funsel)) ||
        CMT_DBG(cmt_abi_put_bytesN(wr, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->recipient)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->token)) ||
        CMT_DBG(cmt_abi_put_uint256(wr, &args->token_id))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_erc721_transfer_decode(cmt_buf_t rx, cmt_erc721_transfer_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_erc721_transfer_funsel)) ||
        CMT_DBG(cmt_abi_get_bytesN(rd, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->recipient)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->token))||
        CMT_DBG(cmt_abi_get_uint256(rd, &args->token_id))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}

// -----------------------------------------------------------------------------
int cmt_evm_advance_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_evm_advance_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_evm_advance_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(wr, frame)) ||
        CMT_DBG(cmt_abi_put_uint(wr, sizeof(args->chain_id), &args->chain_id)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->app_contract)) ||
        CMT_DBG(cmt_abi_put_address(wr, &args->msg_sender)) ||
        CMT_DBG(cmt_abi_put_uint(wr, sizeof(args->block_number), &args->block_number)) ||
        CMT_DBG(cmt_abi_put_uint(wr, sizeof(args->block_timestamp), &args->block_timestamp)) ||
        CMT_DBG(cmt_abi_put_uint256(wr, &args->prev_randao)) ||
        CMT_DBG(cmt_abi_put_uint(wr, sizeof(args->index), &args->index)) ||
        CMT_DBG(cmt_abi_put_dyn_head(wr, state+0, frame)) ||
        CMT_DBG(cmt_abi_put_dyn_tail(wr, state+0, 1, args->payload))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_evm_advance_decode(cmt_buf_t rx, cmt_evm_advance_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_evm_advance_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(rd, frame))||
        CMT_DBG(cmt_abi_get_uint(rd, sizeof(args->chain_id), &args->chain_id)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->app_contract)) ||
        CMT_DBG(cmt_abi_get_address(rd, &args->msg_sender))||
        CMT_DBG(cmt_abi_get_uint(rd, sizeof(args->block_number), &args->block_number))||
        CMT_DBG(cmt_abi_get_uint(rd, sizeof(args->block_timestamp), &args->block_timestamp))||
        CMT_DBG(cmt_abi_get_uint256(rd, &args->prev_randao))||
        CMT_DBG(cmt_abi_get_uint(rd, sizeof(args->index), &args->index)) ||
        CMT_DBG(cmt_abi_get_dyn_head(rd, state+0, frame)) ||
        CMT_DBG(cmt_abi_view_dyn_tail(state+0, 1, &args->payload))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}

// -----------------------------------------------------------------------------
int cmt_notice_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_notice_args_t *args) {
    if (!out || !args) {
        return -EINVAL;
    }
    cmt_buf_t wr[1] = {tx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_put_funsel(wr, cmt_notice_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(wr, frame)) ||
        CMT_DBG(cmt_abi_put_bytesN(wr, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_put_dyn_head(wr, state+0, frame)) ||
        CMT_DBG(cmt_abi_put_dyn_tail(wr, state+0, 1, args->payload))) {
        return -ENOBUFS;
    }
    // clang-format on
    out->begin = tx.begin;
    out->end = wr->begin;
    return 0;
}

int cmt_notice_decode(cmt_buf_t rx, cmt_notice_args_t *args) {
    if (!args) {
        return -EINVAL;
    }
    cmt_buf_t rd[1] = {rx};
    cmt_abi_dyn_state_t state[1];
    cmt_abi_frame_t frame[1];
    // clang-format off
    if (CMT_DBG(cmt_abi_check_funsel(rd, cmt_notice_funsel)) ||
        CMT_DBG(cmt_abi_mark_frame(rd, frame)) ||
        CMT_DBG(cmt_abi_get_bytesN(rd, sizeof(args->app_context), &args->app_context)) ||
        CMT_DBG(cmt_abi_get_dyn_head(rd, state+0, frame)) ||
        CMT_DBG(cmt_abi_view_dyn_tail(state+0, 1, &args->payload))) {
        return -ENOBUFS;
    }
    // clang-format on
    return 0;
}
