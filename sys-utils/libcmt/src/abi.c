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
#include <errno.h>
#include <string.h>

#include "libcmt/abi.h"
#include "libcmt/buf.h"

static uintptr_t align_forward(uintptr_t p, size_t a) {
    return (p + (a - 1)) & ~(a - 1);
}

uint32_t cmt_abi_funsel(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return CMT_ABI_FUNSEL(a, b, c, d);
}

int cmt_abi_mark_frame(const cmt_buf_t *me, cmt_abi_frame_t *frame) {
    if (!me || !frame) {
        return -EINVAL;
    }
    *frame->range = *me;
    return 0;
}

int cmt_abi_put_funsel(cmt_buf_t *wr, uint32_t funsel) {
    cmt_buf_t x[1];
    int rc = cmt_buf_split(*wr, sizeof(funsel), x, wr);
    if (rc) {
        return rc;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(x->begin, &funsel, sizeof(funsel));
    return 0;
}

static int cmt_abi_encode_uint_nr(size_t n, const uint8_t *data, uint8_t out[CMT_ABI_U256_LENGTH]) {
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    for (size_t i = 0; i < n; ++i) {
        out[CMT_ABI_U256_LENGTH - 1 - i] = data[i];
    }
    for (size_t i = n; i < CMT_ABI_U256_LENGTH; ++i) {
        out[CMT_ABI_U256_LENGTH - 1 - i] = 0;
    }
    return 0;
}

static int cmt_abi_encode_uint_nn(size_t n, const uint8_t *data, uint8_t out[CMT_ABI_U256_LENGTH]) {
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    for (size_t i = 0; i < CMT_ABI_U256_LENGTH - n; ++i) {
        out[i] = 0;
    }
    for (size_t i = CMT_ABI_U256_LENGTH - n; i < CMT_ABI_U256_LENGTH; ++i) {
        out[i] = data[i - CMT_ABI_U256_LENGTH + n];
    }
    return 0;
}

static int cmt_abi_encode_bytesN(size_t n, const void *data, uint8_t out[CMT_ABI_U256_LENGTH]) {
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    // bytes<M> is left-aligned: place bytes at the front, zero the rest
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(out, data, n);
    memset(out + n, 0, CMT_ABI_U256_LENGTH - n);
    return 0;
}

static int cmt_abi_encode_uint(size_t n, const void *data, uint8_t out[CMT_ABI_U256_LENGTH]) {
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    return cmt_abi_encode_uint_nn(n, data, out);
#else
    return cmt_abi_encode_uint_nr(n, data, out);
#endif
}

static int cmt_abi_decode_uint_nr(const uint8_t data[CMT_ABI_U256_LENGTH], size_t n, uint8_t *out) {
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    for (size_t i = 0; i < CMT_ABI_U256_LENGTH - n; ++i) {
        if (data[i]) {
            return -EDOM;
        }
    }
    for (size_t i = CMT_ABI_U256_LENGTH - n; i < CMT_ABI_U256_LENGTH; ++i) {
        out[CMT_ABI_U256_LENGTH - 1 - i] = data[i];
    }
    return 0;
}

static int cmt_abi_decode_uint_nn(const uint8_t data[CMT_ABI_U256_LENGTH], size_t n, uint8_t *out) {
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    for (size_t i = 0; i < CMT_ABI_U256_LENGTH - n; ++i) {
        if (data[i]) {
            return -EDOM;
        }
    }
    for (size_t i = CMT_ABI_U256_LENGTH - n; i < CMT_ABI_U256_LENGTH; ++i) {
        out[i - CMT_ABI_U256_LENGTH + n] = data[i];
    }
    return 0;
}

static int cmt_abi_decode_bytesN(const uint8_t data[CMT_ABI_U256_LENGTH], size_t n, void *out) {
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    // bytes<M> is left-aligned: copy the first n bytes only
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(out, data, n);
    return 0;
}

static int cmt_abi_decode_uint(const uint8_t data[CMT_ABI_U256_LENGTH], size_t n, uint8_t *out) {
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    return cmt_abi_decode_uint_nn(data, n, out);
#else
    return cmt_abi_decode_uint_nr(data, n, out);
#endif
}

int cmt_abi_put_uint(cmt_buf_t *wr, size_t data_length, const void *data) {
    cmt_buf_t x[1];
    if (data_length > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    if (cmt_buf_split(*wr, CMT_ABI_U256_LENGTH, x, wr)) {
        return -ENOBUFS;
    }
    return cmt_abi_encode_uint(data_length, data, x->begin);
}

int cmt_abi_put_uint_be(cmt_buf_t *wr, size_t data_length, const void *data) {
    cmt_buf_t x[1];
    if (data_length > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    if (cmt_buf_split(*wr, CMT_ABI_U256_LENGTH, x, wr)) {
        return -ENOBUFS;
    }
    return cmt_abi_encode_uint_nn(data_length, data, x->begin);
}
int cmt_abi_put_uint256(cmt_buf_t *wr, const cmt_abi_u256_t *value) {
    return cmt_abi_put_uint_be(wr, CMT_ABI_U256_LENGTH, value->data);
}

int cmt_abi_put_bytesN(cmt_buf_t *wr, size_t n, const void *data) {
    cmt_buf_t x[1];
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    if (cmt_buf_split(*wr, CMT_ABI_U256_LENGTH, x, wr)) {
        return -ENOBUFS;
    }
    return cmt_abi_encode_bytesN(n, data, x->begin);
}

int cmt_abi_put_bool(cmt_buf_t *wr, bool value) {
    uint8_t boolean = !!value;
    return cmt_abi_put_uint(wr, sizeof(boolean), &boolean);
}

int cmt_abi_put_address(cmt_buf_t *wr, const cmt_abi_address_t *address) {
    return cmt_abi_put_uint_be(wr, CMT_ABI_ADDRESS_LENGTH, address->data);
}

int cmt_abi_put_dyn_head(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, cmt_abi_frame_t *frame) {
    *state->frame = *frame;
    return cmt_buf_split(*wr, CMT_ABI_U256_LENGTH, state->offset, wr);
}

int cmt_abi_reserve_dyn_tail(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, cmt_buf_t *available) {
    return cmt_buf_split(*wr, CMT_ABI_U256_LENGTH, state->length, available);
}

int cmt_abi_commit_dyn_tail(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, size_t size, size_t n) {
    if (!wr || !state || !size) {
        return -EINVAL;
    }
    int rc = 0;
    size_t offset = state->length->begin - state->frame->range->begin;

    rc = cmt_abi_encode_uint(sizeof(offset), &offset, state->offset->begin);
    if (rc != 0) {
        return rc;
    }

    rc = cmt_abi_encode_uint(sizeof(n), &n, state->length->begin);
    if (rc != 0) {
        return rc;
    }

    size_t used = size * n;

    // zero out the padding at the end of data.
    // dynamic part: [ length | data | padding? ]
    size_t used32 = align_forward(used, CMT_ABI_U256_LENGTH);
    if (used32 != used) {
        memset(state->length->end + used, 0, used32 - used);
    }

    // advance cursor
    wr->begin += CMT_ABI_U256_LENGTH + used32;
    return 0;
}

int cmt_abi_put_dyn_tail(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, size_t size, cmt_buf_t data) {
    if (!wr || !state || !size) {
        return -EINVAL;
    }
    int rc = 0;
    cmt_buf_t available[1];

    rc = cmt_abi_reserve_dyn_tail(wr, state, available);
    if (rc != 0) {
        return rc;
    }

    size_t required = cmt_buf_length(data);

    if (cmt_buf_length(*available) < required) {
        return -ENOBUFS;
    }

    // length(data) must be an exact multiple of size
    size_t count = required / size;
    if (count * size != required) {
        return -EDOM;
    }

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(available->begin, cmt_buf_begin(data), required);
    cmt_abi_commit_dyn_tail(wr, state, size, count);

    return 0;
}

uint32_t cmt_abi_peek_funsel(cmt_buf_t *me) {
    if (cmt_buf_length(*me) < 4) {
        return 0;
    }
    return CMT_ABI_FUNSEL(me->begin[0], me->begin[1], me->begin[2], me->begin[3]);
}

int cmt_abi_check_funsel(cmt_buf_t *me, uint32_t expected) {
    if (cmt_buf_length(*me) < 4) {
        return -ENOBUFS;
    }

    if (cmt_abi_peek_funsel(me) != expected) {
        return -EBADMSG;
    }

    me->begin += 4;
    return 0;
}

int cmt_abi_get_uint(cmt_buf_t *rd, size_t n, void *data) {
    cmt_buf_t x[1];

    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    int rc = cmt_buf_split(*rd, CMT_ABI_U256_LENGTH, x, rd);
    if (rc) {
        return rc;
    }

    return cmt_abi_decode_uint(x->begin, n, data);
}

int cmt_abi_get_uint_be(cmt_buf_t *rd, size_t n, void *data) {
    cmt_buf_t x[1];

    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    int rc = cmt_buf_split(*rd, CMT_ABI_U256_LENGTH, x, rd);
    if (rc) {
        return rc;
    }

    return cmt_abi_decode_uint_nn(x->begin, n, data);
}

int cmt_abi_get_uint256(cmt_buf_t *rd, cmt_abi_u256_t *value) {
    return cmt_abi_get_uint_be(rd, CMT_ABI_U256_LENGTH, value->data);
}

int cmt_abi_get_bytesN(cmt_buf_t *rd, size_t n, void *data) {
    cmt_buf_t x[1];
    if (n > CMT_ABI_U256_LENGTH) {
        return -EDOM;
    }
    if (cmt_buf_split(*rd, CMT_ABI_U256_LENGTH, x, rd)) {
        return -ENOBUFS;
    }
    return cmt_abi_decode_bytesN(x->begin, n, data);
}

int cmt_abi_get_bool(cmt_buf_t *rd, bool *value) {
    return cmt_abi_get_uint(rd, sizeof(*value), value);
}

int cmt_abi_get_address(cmt_buf_t *rd, cmt_abi_address_t *address) {
    return cmt_abi_get_uint_be(rd, CMT_ABI_ADDRESS_LENGTH, address->data);
}

int cmt_abi_get_dyn_head(cmt_buf_t *rd, cmt_abi_dyn_state_t *state, cmt_abi_frame_t *frame) {
    *state->frame = *frame;
    return cmt_buf_split(*rd, CMT_ABI_U256_LENGTH, state->offset, rd);
}

int cmt_abi_view_dyn_tail(cmt_abi_dyn_state_t *state, size_t size, cmt_buf_t *data) {
    if (!state || !size || !data) {
        return -EINVAL;
    }
    int rc = 0;
    size_t offset = 0;
    size_t count = 0;
    size_t total_bytes = 0;

    rc = cmt_abi_decode_uint(state->offset->begin, sizeof(offset), (uint8_t *) &offset);
    if (rc != 0) {
        return rc;
    }

    cmt_buf_t view[1] = {{state->frame->range->begin + offset, state->frame->range->end}};
    rc = cmt_abi_get_uint(view, sizeof(count), (uint8_t *) &count);
    if (rc != 0) {
        return rc;
    }
    total_bytes = count * size;
    return cmt_buf_split(*view, total_bytes, data, NULL);
}

int cmt_abi_get_dyn_tail(cmt_abi_dyn_state_t *state, size_t size, size_t max, void *data) {
    if (!state || !size || !data) {
        return -EINVAL;
    }
    int rc = 0;

    cmt_buf_t view[1];
    rc = cmt_abi_view_dyn_tail(state, size, view);
    if (rc != 0) {
        return rc;
    }

    if (cmt_buf_length(*view) > max) {
        return -ENOBUFS;
    }

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(data, view->begin, cmt_buf_length(*view));
    return 0;
}
