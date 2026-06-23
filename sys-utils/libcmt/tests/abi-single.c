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
#include "libcmt/abi.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/** Declare a cmt_buf_t with stack backed memory.
 * @param [in] N - size in bytes
 * @note don't port */
#define CMT_BUF_DECL(S, L) cmt_buf_t S[1] = {cmt_buf_make((L), (uint8_t[L]){0})}

/** Declare a cmt_buf_t with parameters backed memory.
 * @param [in] L - size in bytes
 * @note don't port */
#define CMT_BUF_DECL3(S, L, P) cmt_buf_t S[1] = {cmt_buf_make((L), (P))}

// funsel(address)
#define FUNSEL CMT_ABI_FUNSEL(0xe6, 0x36, 0xe3, 0x33)

static void macro_matches_cmt_abi_funsel(void) {
    uint32_t funsel = cmt_abi_funsel(0xe6, 0x36, 0xe3, 0x33);
    assert(funsel == FUNSEL);
}

static void put_funsel(void) {
    uint8_t data[] = {0xcd, 0xcd, 0x77, 0xc0};
    uint32_t funsel = CMT_ABI_FUNSEL(data[0], data[1], data[2], data[3]);
    CMT_BUF_DECL(b, 64);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_funsel(it, funsel) == 0);
    assert(memcmp(b->begin, data, 4) == 0);
}

static void put_funsel_enobufs(void) {
    uint8_t data[] = {0xcd, 0xcd, 0x77, 0xc0};
    uint32_t funsel = CMT_ABI_FUNSEL(data[0], data[1], data[2], data[3]);
    CMT_BUF_DECL(b, 1);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_funsel(it, funsel) == -ENOBUFS);
}

static void put_uint(void) {
    uint64_t x = UINT64_C(0x0123456789abcdef);
    uint8_t be[CMT_ABI_U256_LENGTH] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        // clang-format on
    };
    CMT_BUF_DECL(b, 64);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_uint(it, sizeof(x), &x) == 0);
    assert(memcmp(b->begin, be, sizeof(be)) == 0);
}

static void put_uint_enobufs(void) {
    uint64_t x = UINT64_C(0x0123456789abcdef);
    CMT_BUF_DECL(b, 31);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_uint(it, sizeof(x), &x) == -ENOBUFS);
    assert(cmt_abi_put_uint_be(it, sizeof(x), &x) == -ENOBUFS);
}

static void put_uint_edom(void) {
    uint64_t x[CMT_ABI_U256_LENGTH + 1] = {0};
    CMT_BUF_DECL(b, CMT_ABI_U256_LENGTH);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_uint(it, sizeof(x), &x) == -EDOM);
    assert(cmt_abi_put_uint_be(it, sizeof(x), &x) == -EDOM);
}

static void put_bool(void) {
    uint8_t be[CMT_ABI_U256_LENGTH] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        // clang-format on
    };
    CMT_BUF_DECL(b, 64);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_bool(it, true) == 0);
    assert(memcmp(b->begin, be, sizeof(be)) == 0);
}

static void put_address(void) {
    // clang-format off
    cmt_abi_address_t x = {{
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67,
    }};
    uint8_t be[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67,
    };
    // clang-format on
    CMT_BUF_DECL(b, 64);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_address(it, &x) == 0);
    assert(memcmp(b->begin, be, sizeof(be)) == 0);
}

static void put_address_enobufs(void) {
    cmt_abi_address_t x = {{0}};
    CMT_BUF_DECL(b, CMT_ABI_U256_LENGTH - 1);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_put_address(it, &x) == -ENOBUFS);
}

static void put_bytes(void) {
    uint64_t x = UINT64_C(0x0123456789abcdef);
    uint8_t be[] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
        0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // clang-format on
    };
    CMT_BUF_DECL(b, 128);
    cmt_buf_t it[1] = {*b};
    cmt_abi_dyn_state_t state;
    cmt_abi_frame_t frame;

    assert(cmt_abi_mark_frame(it, &frame) == 0);
    assert(cmt_abi_put_dyn_head(it, &state, &frame) == 0);
    assert(cmt_abi_put_dyn_tail(it, &state, 1, cmt_buf_make(sizeof(x), &x)) == 0);
    assert(memcmp(b->begin, be, sizeof(be)) == 0);
}

static void put_bytes_enobufs(void) {
    uint64_t x = UINT64_C(0x0123456789abcdef);
    cmt_abi_dyn_state_t state;
    cmt_abi_frame_t frame;

    /* buffer too small for reserve_dyn_tail to have enough space for data */
    CMT_BUF_DECL(b, (2UL * 32) + sizeof(x) - 1);
    cmt_buf_t it[1] = {*b};
    assert(cmt_abi_mark_frame(it, &frame) == 0);
    assert(cmt_abi_put_dyn_head(it, &state, &frame) == 0);
    assert(cmt_abi_put_dyn_tail(it, &state, 1, cmt_buf_make(sizeof(x), &x)) == -ENOBUFS);
}

static void get_funsel(void) {
    CMT_BUF_DECL(b, 64);
    cmt_buf_t wr[1] = {*b};
    cmt_buf_t rd[1] = {*b};
    uint32_t right = CMT_ABI_FUNSEL(1, 2, 3, 4);
    uint32_t wrong = CMT_ABI_FUNSEL(1, 2, 3, 3);

    assert(cmt_abi_put_funsel(wr, right) == 0);
    assert(cmt_abi_peek_funsel(rd) == right);
    assert(cmt_abi_check_funsel(rd, wrong) == -EBADMSG); // don't advance
    assert(cmt_abi_check_funsel(rd, right) == 0);
}

static void peek_funsel_error(void) {
    CMT_BUF_DECL(b, 3);
    uint32_t wrong = 0; // value doesn't matter
    cmt_buf_t rd[1] = {*b};

    assert(cmt_abi_peek_funsel(rd) == 0);
    assert(cmt_abi_check_funsel(rd, wrong) == -ENOBUFS); // don't advance
}

static void get_uint(void) {
    uint64_t x = 0;
    uint64_t ex = UINT64_C(0x0123456789abcdef);
    uint8_t be[CMT_ABI_U256_LENGTH] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        // clang-format on
    };
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t rd[1] = {*b};

    assert(cmt_abi_get_uint(rd, sizeof(x), &x) == 0);
    assert(x == ex);
}

static void get_uint_enobufs(void) {
    uint64_t x = UINT64_C(0x0123456789abcdef);
    CMT_BUF_DECL(b, 31);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_get_uint(it, sizeof(x), &x) == -ENOBUFS);
    assert(cmt_abi_get_uint_be(it, sizeof(x), &x) == -ENOBUFS);
}

static void get_uint_edom(void) {
    uint64_t x[CMT_ABI_U256_LENGTH + 1] = {0};
    CMT_BUF_DECL(b, CMT_ABI_U256_LENGTH);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_get_uint(it, sizeof(x), &x) == -EDOM);
    assert(cmt_abi_get_uint_be(it, sizeof(x), &x) == -EDOM);
}

static void get_uint_be(void) {
    uint64_t x = 0;
    uint64_t ex = UINT64_C(0x0123456789abcdef);
    uint8_t be[CMT_ABI_U256_LENGTH] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
        // clang-format on
    };
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t rd[1] = {*b};

    assert(cmt_abi_get_uint_be(rd, sizeof(x), &x) == 0);
    assert(x == ex);
}

static void get_bool(void) {
    bool x = false;
    bool ex = true;
    uint8_t be[CMT_ABI_U256_LENGTH] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        // clang-format on
    };
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t rd[1] = {*b};

    assert(cmt_abi_get_bool(rd, &x) == 0);
    assert(x == ex);
}

static void get_bool_enobufs(void) {
    bool x = false;
    uint8_t be[CMT_ABI_U256_LENGTH - 1] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // clang-format on
    };
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t rd[1] = {*b};

    assert(cmt_abi_get_bool(rd, &x) == -ENOBUFS);
}

static void get_address(void) {
    // clang-format off
    cmt_abi_address_t x;
    uint8_t ex[CMT_ABI_ADDRESS_LENGTH] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67
    };
    uint8_t be[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67,
    };
    // clang-format on
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t it[1] = {*b};

    assert(cmt_abi_get_address(it, &x) == 0);
    assert(memcmp(x.data, ex, sizeof(ex)) == 0);
}

static void get_address_enobufs(void) {
    cmt_abi_address_t x;
    // clang-format off
    uint8_t be[CMT_ABI_U256_LENGTH - 1] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45,
    };
    // clang-format on
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t it[1] = {*b};
    assert(cmt_abi_get_address(it, &x) == -ENOBUFS);
}

static void get_bytes(void) {
    uint64_t ex = UINT64_C(0x0123456789abcdef);
    uint8_t be[] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
        0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // clang-format on
    };
    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_abi_frame_t frame = {.range = {{b->begin, b->end}}};
    cmt_buf_t it[1] = {*b};
    cmt_abi_dyn_state_t state;
    cmt_buf_t bytes[1];

    assert(cmt_abi_get_dyn_head(it, &state, &frame) == 0);
    assert(cmt_abi_view_dyn_tail(&state, 1, bytes) == 0);
    assert(cmt_buf_length(*bytes) == sizeof(ex));
    assert(memcmp(bytes->begin, &ex, sizeof(ex)) == 0);
}

static void get_bytes_enobufs(void) {
    // clang-format off
    uint8_t be[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
        0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    // clang-format on

    { // buffer too small for offset read
        CMT_BUF_DECL3(b, CMT_ABI_U256_LENGTH - 1, be);
        cmt_buf_t it[1] = {*b};
        cmt_abi_frame_t frame = {.range = {{b->begin, b->end}}};
        cmt_abi_dyn_state_t state;

        assert(cmt_abi_get_dyn_head(it, &state, &frame) == -ENOBUFS);
    }

    { // copy dynamic bytes into adequately-sized buffer
        CMT_BUF_DECL3(b, 3UL * CMT_ABI_U256_LENGTH, be);
        cmt_buf_t it[1] = {*b};
        cmt_abi_frame_t frame = {.range = {{b->begin, b->end}}};
        cmt_abi_dyn_state_t state;
        uint8_t data[8];

        assert(cmt_abi_get_dyn_head(it, &state, &frame) == 0);
        assert(cmt_abi_get_dyn_tail(&state, 1, sizeof(data), data) == 0);
    }

    { // dynamic region too small to copy: provided buffer too small
        CMT_BUF_DECL3(b, 3UL * CMT_ABI_U256_LENGTH, be);
        cmt_buf_t it[1] = {*b};
        cmt_abi_frame_t frame = {.range = {{b->begin, b->end}}};
        cmt_abi_dyn_state_t state;
        uint8_t data[7]; // too small for 8 bytes of data

        assert(cmt_abi_get_dyn_head(it, &state, &frame) == 0);
        assert(cmt_abi_get_dyn_tail(&state, 1, sizeof(data), data) == -ENOBUFS);
    }
}

static void get_uint_padding_edom(void) {
    // Non-zero bytes in upper padding must cause EDOM (value doesn't fit)
    uint8_t be[CMT_ABI_U256_LENGTH] = {
        // clang-format off
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        // clang-format on
    };
    uint8_t x[8] = {0};

    CMT_BUF_DECL3(b, sizeof(be), be);
    cmt_buf_t rd[1] = {*b};

    // get_uint uses decode_uint which rejects non-zero upper bytes
    assert(cmt_abi_get_uint(rd, sizeof(x), x) == -EDOM);
}

static void put_get_bytesN(void) {
    uint8_t data[4] = {0xde, 0xad, 0xbe, 0xef};
    uint8_t expected[CMT_ABI_U256_LENGTH] = {0xde, 0xad, 0xbe, 0xef};
    CMT_BUF_DECL(b, 64);
    cmt_buf_t wr[1] = {*b};

    // Encode
    assert(cmt_abi_put_bytesN(wr, 4, data) == 0);
    assert(memcmp(b->begin, expected, 4) == 0);
    assert(b->begin[4] == 0x00); // zero-padded

    // Decode
    cmt_buf_t rd[1] = {*b};
    uint8_t out[4] = {0};
    assert(cmt_abi_get_bytesN(rd, 4, out) == 0);
    assert(memcmp(out, data, 4) == 0);
}

static void put_get_bytesN_errors(void) {
    uint8_t buf[16] = {0};

    // n > 32 → EDOM
    {
        cmt_buf_t wr[1] = {cmt_buf_make(sizeof(buf), buf)};
        assert(cmt_abi_put_bytesN(wr, 33, buf) == -EDOM);
    }
    {
        cmt_buf_t rd[1] = {cmt_buf_make(sizeof(buf), buf)};
        assert(cmt_abi_get_bytesN(rd, 33, buf) == -EDOM);
    }

    // buffer too small → ENOBUFS
    {
        cmt_buf_t wr[1] = {cmt_buf_make(CMT_ABI_U256_LENGTH - 1, buf)};
        assert(cmt_abi_put_bytesN(wr, 4, buf) == -ENOBUFS);
    }
}

int main(void) {
    macro_matches_cmt_abi_funsel();

    put_funsel();
    put_funsel_enobufs();
    put_uint();
    put_uint_enobufs();
    put_uint_edom();
    put_bool();
    put_address();
    put_address_enobufs();
    put_bytes();
    put_bytes_enobufs();

    get_funsel();
    peek_funsel_error();
    get_uint();
    get_uint_be();
    get_uint_enobufs();
    get_uint_edom();
    get_bool();
    get_bool_enobufs();
    get_address();
    get_address_enobufs();
    get_bytes();
    get_bytes_enobufs();
    get_uint_padding_edom();
    put_get_bytesN();
    put_get_bytesN_errors();

    printf("All abi-single tests passed!\n");
    return 0;
}
