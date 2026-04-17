// Copyright Cartesi and individual authors (see AUTHORS)
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

#include "labelinfo.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define LABEL_DIR "/run/cartesi/memoryranges"

// Two-level stringification so PATH_MAX is expanded before '#' is applied.
#define STR2(x) #x
#define STR(x) STR2(x)

int labelinfo_lookup(const char *prog, const char *label, struct labelinfo *out) {
    char label_path[PATH_MAX];
    int n = snprintf(label_path, sizeof(label_path), LABEL_DIR "/%s", label);
    if (n <= 0 || (size_t)n >= sizeof(label_path)) {
        fprintf(stderr, "%s: label '%s' is too long\n", prog, label);
        return -1;
    }
    FILE *fp = fopen(label_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: no such label '%s': %s\n", prog, label, strerror(errno));
        return -1;
    }
    int matched = fscanf(fp, "%" STR(PATH_MAX) "s %" SCNx64 " %" SCNx64,
        out->device, &out->start, &out->length);
    fclose(fp);
    if (matched != 3) {
        fprintf(stderr, "%s: malformed label file '%s'\n", prog, label_path);
        return -1;
    }
    return 0;
}
