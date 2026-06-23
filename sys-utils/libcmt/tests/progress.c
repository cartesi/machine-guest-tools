#include "libcmt/rollup.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_rollup_progress(void) {
    cmt_rollup_t rollup;

    assert(cmt_rollup_init(&rollup, NULL) == 0);
    assert(cmt_rollup_progress(&rollup, 0) == 0);
    assert(cmt_rollup_progress(&rollup, 10) == 0);
    assert(cmt_rollup_progress(&rollup, 100) == 0);
    assert(cmt_rollup_progress(&rollup, 1000) == 0);
    assert(cmt_rollup_progress(&rollup, UINT32_MAX) == 0);
    cmt_rollup_fini(&rollup);

    printf("test %s passed\n", __func__);
}

int main(void) {
    test_rollup_progress();
    return 0;
}
