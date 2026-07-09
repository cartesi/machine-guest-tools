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
#include "libcmt/buf.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

static inline int is_pow2(int l) {
    return !(l & (l - 1));
}

cmt_buf_t cmt_buf_make(size_t length, const void *data) {
    return (cmt_buf_t){.begin = (uint8_t *) data, .end = (uint8_t *) data + length};
}

int cmt_buf_split(cmt_buf_t me, size_t lhs_length, cmt_buf_t *lhs, cmt_buf_t *rhs) {
    uintptr_t begin = (uintptr_t) me.begin;
    uintptr_t split = (uintptr_t) me.begin + lhs_length;
    uintptr_t end = (uintptr_t) me.end;

    if (split < begin || end < split) {
        return -ENOBUFS;
    }
    if (lhs) {
        lhs->begin = (uint8_t *) begin; // NOLINT performance-no-int-to-ptr
        lhs->end = (uint8_t *) split;   // NOLINT performance-no-int-to-ptr
    }
    if (rhs) {
        rhs->begin = (uint8_t *) split; // NOLINT performance-no-int-to-ptr
        rhs->end = (uint8_t *) end;     // NOLINT performance-no-int-to-ptr
    }

    return 0;
}

static int comma(const uint8_t *s, const uint8_t *end) {
    return s < end && *s == ',';
}

static bool cmt_buf_split_by(cmt_buf_t *x, cmt_buf_t *xs, int (*nxt)(const uint8_t *s, const uint8_t *end)) {
    if (xs->begin == xs->end) {
        return false;
    }
    x->begin = xs->begin;
    for (; *xs->begin && xs->begin < xs->end && !nxt(xs->begin, xs->end); ++xs->begin) {
        ;
    }
    x->end = xs->begin;
    xs->begin = xs->begin + nxt(xs->begin, xs->end);
    return *x->begin;
}

bool cmt_buf_split_by_comma(cmt_buf_t *x, cmt_buf_t *xs) {
    return cmt_buf_split_by(x, xs, comma);
}

void *cmt_buf_begin(cmt_buf_t me) {
    return me.begin;
}

size_t cmt_buf_length(cmt_buf_t me) {
    return me.end - me.begin;
}

static void xxd(const uint8_t *p, const uint8_t *q, size_t mask) {
    if (q < p) {
        return;
    }

    for (size_t i = 0U, n = q - p; i < n; ++i) {
        bool is_line_start = (i & mask) == 0;
        bool is_line_end = (i & mask) == mask || (i + 1 == n);
        char separator = is_line_end ? '\n' : ' ';

        if (is_line_start) {
            printf("%p %4zu: ", (void *) (p + i), i);
        }
        printf("%02x%c", p[i], separator);
    }
}

void cmt_buf_xxd(const void *begin, const void *end, int bytes_per_line) {
    if (!begin) {
        return;
    }
    if (!end) {
        return;
    }
    if (end <= begin) {
        return;
    }
    if (!is_pow2(bytes_per_line)) {
        return;
    }

    xxd(begin, end, bytes_per_line - 1);
}
