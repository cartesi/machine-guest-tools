#include "libcmt/util.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmt_util_debug(int rc, const char *expr, const char *file, int line) {
    if (rc == 0) {
        return 0;
    }
    if (cmt_util_debug_enabled()) {
        (void) fprintf(stderr, "%s:%d Error %s on `%s'\n", file, line, strerror(-rc), expr);
    }
    return rc;
}

bool cmt_util_debug_enabled(void) {
    static bool checked = false;
    static bool enabled = false;

    if (!checked) {
        enabled = getenv("CMT_DEBUG") != NULL;
        checked = true;
    }

    return enabled;
}

int cmt_util_read_whole_file(const char *name, size_t max, void *data, size_t *length) {
    int rc = 0;

    FILE *file = fopen(name, "rb");
    if (!file) {
        return -errno;
    }
    *length = fread(data, 1, max, file);
    int eof = (fseek(file, 0, SEEK_END) == 0) && (*length == (size_t) ftell(file));
    if (!eof) {
        rc = -EIO;
    }
    if (fclose(file) != 0) {
        rc = -errno;
    }
    return rc;
}

int cmt_util_write_whole_file(const char *name, size_t length, const void *data) {
    int rc = 0;

    FILE *file = fopen(name, "wb");
    if (!file) {
        return -errno;
    }

    if (fwrite(data, 1, length, file) != length) {
        rc = -EIO;
    }
    if (fclose(file) != 0) {
        rc = -errno;
    }
    return rc;
}
