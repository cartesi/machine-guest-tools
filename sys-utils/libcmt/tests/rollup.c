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
#include "libcmt/rollup.h"
#include "data.h"
#include "libcmt/util.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void test_rollup_init_and_fini(void) {
    cmt_rollup_t rollup;

    // init twice returns failure, same as when using the kernel driver
    assert(cmt_rollup_init(&rollup, NULL) == 0);
    assert(cmt_rollup_init(&rollup, NULL) == -EBUSY);
    cmt_rollup_fini(&rollup);

    // and should reset when closed
    assert(cmt_rollup_init(&rollup, NULL) == 0);
    cmt_rollup_fini(&rollup);

    // double free is a bug, but try to avoid crashing
    cmt_rollup_fini(&rollup);

    // fail to initialize with NULL
    assert(cmt_rollup_init(NULL, NULL) == -EINVAL);
    cmt_rollup_fini(NULL);

    // getters: NULL inputs
    assert(cmt_rollup_get_io(NULL) == NULL);
    assert(cmt_rollup_get_merkle(NULL) == NULL);

    // getters: valid rollup
    assert(cmt_rollup_init(&rollup, NULL) == 0);
    assert(cmt_rollup_get_io(&rollup) != NULL);
    assert(cmt_rollup_get_merkle(&rollup) != NULL);
    cmt_rollup_fini(&rollup);

    printf("test %s passed\n", __func__);
}

static void test_rollup_parse_inputs(void) {
    cmt_rollup_t rollup;
    cmt_buf_t rx[1] = {};

    uint8_t data[] = {0};

    // synthesize inputs and feed them to io-mock via CMT_INPUTS env.
    assert(cmt_util_write_whole_file("0.bin", sizeof valid_advance_0, valid_advance_0) == 0);
    assert(cmt_util_write_whole_file("1.bin", sizeof valid_inspect_0, valid_inspect_0) == 0);
    assert(cmt_util_write_whole_file("2.bin", sizeof data, data) == 0);
    assert(cmt_util_write_whole_file("3.bin", sizeof data, data) == 0);
    assert(truncate("2.bin", (2UL << 20) + 1) == 0); // large buffer
    assert(setenv("CMT_INPUTS", "0:0.bin,1:1.bin,0:2.bin,0:3.bin", 1) == 0);

    assert(cmt_rollup_init(&rollup, NULL) == 0);

    // input 0: advance state
    assert(cmt_rollup_wait_for_input(&rollup, true, rx) == HTIF_YIELD_REASON_ADVANCE);
    assert(cmt_buf_length(*rx) == sizeof valid_advance_0);
    assert(memcmp(rx->begin, valid_advance_0, sizeof valid_advance_0) == 0);

    // input 1: inspect state
    assert(cmt_rollup_wait_for_input(&rollup, true, rx) == HTIF_YIELD_REASON_INSPECT);
    assert(cmt_buf_length(*rx) == sizeof valid_inspect_0);
    assert(memcmp(rx->begin, valid_inspect_0, sizeof valid_inspect_0) == 0);

    // input 2: too large for the mock buffer, should fail
    assert(cmt_rollup_wait_for_input(&rollup, true, rx) == -ENODATA);

    // reject should fail in mock (revert is not supported)
    assert(cmt_rollup_wait_for_input(&rollup, false, rx) == -ENOSYS);

    // no more inputs available
    assert(cmt_rollup_wait_for_input(&rollup, true, rx) == -ENODATA);

    cmt_rollup_fini(&rollup);
    printf("test %s passed\n", __func__);
}

static void test_rollup_outputs_reports_and_exceptions(void) {
    cmt_rollup_t rollup;
    uint8_t buffer[1024];
    size_t read_size = 0;

    cmt_buf_t tx;
    assert(cmt_rollup_init(&rollup, &tx) == 0);

    // ---- emit_output ------------------------------------------------
    { // copy mode
        char output_data[] = "output-0";
        cmt_buf_t data = cmt_buf_make(strlen(output_data), output_data);
        assert(cmt_rollup_emit_output(&rollup, data) == 0);
        assert(cmt_util_read_whole_file("none.output-0.bin", sizeof buffer, buffer, &read_size) == 0);
        assert(read_size == strlen(output_data));
        assert(memcmp(buffer, output_data, strlen(output_data)) == 0);
    }

    { // zero-copy mode
        char output_data[] = "output-1";
        memcpy(cmt_buf_begin(tx), output_data, strlen(output_data));
        assert(cmt_rollup_emit_output(&rollup, cmt_buf_make(strlen(output_data), cmt_buf_begin(tx))) == 0);
        assert(cmt_util_read_whole_file("none.output-1.bin", sizeof buffer, buffer, &read_size) == 0);
        assert(read_size == strlen(output_data));
        assert(memcmp(buffer, output_data, strlen(output_data)) == 0);
    }

    // emit_output (invalid: NULL rollup)
    {
        char output_data[] = "x";
        cmt_buf_t data = cmt_buf_make(1, output_data);
        assert(cmt_rollup_emit_output(NULL, data) == -EINVAL);
    }

    // emit_output (invalid: data exceeds TX buffer)
    {
        char output_data[] = "x";
        cmt_buf_t data = cmt_buf_make(UINT32_MAX, output_data);
        assert(cmt_rollup_emit_output(&rollup, data) == -ENOBUFS);
    }

    // ---- emit_report ------------------------------------------------
    {
        char report_data[] = "report-0";
        cmt_buf_t data = cmt_buf_make(strlen(report_data), report_data);
        assert(cmt_rollup_emit_report(&rollup, data) == 0);
        assert(cmt_util_read_whole_file("none.report-0.bin", sizeof buffer, buffer, &read_size) == 0);
        assert(read_size == strlen(report_data));
        assert(memcmp(buffer, report_data, strlen(report_data)) == 0);
    }

    { // zero-copy mode
        char report_data[] = "report-1";
        memcpy(cmt_buf_begin(tx), report_data, strlen(report_data));
        assert(cmt_rollup_emit_report(&rollup,
            cmt_buf_make(strlen(report_data), cmt_buf_begin(tx))) == 0);
        assert(cmt_util_read_whole_file("none.report-1.bin", sizeof buffer, buffer, &read_size) == 0);
        assert(read_size == strlen(report_data));
        assert(memcmp(buffer, report_data, strlen(report_data)) == 0);
    }

    // emit_report (invalid: NULL rollup)
    {
        char report_data[] = "x";
        cmt_buf_t data = cmt_buf_make(1, report_data);
        assert(cmt_rollup_emit_report(NULL, data) == -EINVAL);
    }

    // emit_report (invalid: data exceeds TX buffer)
    {
        char report_data[] = "x";
        cmt_buf_t data = cmt_buf_make(UINT32_MAX, report_data);
        assert(cmt_rollup_emit_report(&rollup, data) == -ENOBUFS);
    }

    // ---- emit_exception ---------------------------------------------
    {
        char exception_data[] = "exception-0";
        cmt_buf_t data = cmt_buf_make(strlen(exception_data), exception_data);
        assert(cmt_rollup_emit_exception(&rollup, data) == 0);
        assert(cmt_util_read_whole_file("none.exception-0.bin", sizeof buffer, buffer, &read_size) == 0);
        assert(read_size == strlen(exception_data));
        assert(memcmp(buffer, exception_data, strlen(exception_data)) == 0);
    }

    { // zero-copy mode
        char exception_data[] = "exception-1";
        memcpy(cmt_buf_begin(tx), exception_data, strlen(exception_data));
        assert(cmt_rollup_emit_exception(&rollup,
            cmt_buf_make(strlen(exception_data), cmt_buf_begin(tx))) == 0);
        assert(cmt_util_read_whole_file("none.exception-1.bin", sizeof buffer, buffer, &read_size) == 0);
        assert(read_size == strlen(exception_data));
        assert(memcmp(buffer, exception_data, strlen(exception_data)) == 0);
    }

    // emit_exception (invalid: NULL rollup)
    {
        char exception_data[] = "x";
        cmt_buf_t data = cmt_buf_make(1, exception_data);
        assert(cmt_rollup_emit_exception(NULL, data) == -EINVAL);
    }

    // emit_exception (invalid: data exceeds TX buffer)
    {
        char exception_data[] = "x";
        cmt_buf_t data = cmt_buf_make(UINT32_MAX, exception_data);
        assert(cmt_rollup_emit_exception(&rollup, data) == -ENOBUFS);
    }

    cmt_rollup_fini(&rollup);
    printf("test %s passed\n", __func__);
}

int main(void) {
    setenv("CMT_DEBUG", "yes", 1);
    test_rollup_init_and_fini();
    test_rollup_parse_inputs();
    test_rollup_outputs_reports_and_exceptions();
    return 0;
}
