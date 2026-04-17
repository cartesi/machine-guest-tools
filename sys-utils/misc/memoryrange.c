// Copyright Cartesi and individual authors (see AUTHORS)
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

// memoryrange / flashdrive / nvram
//
// Looks up a memory range configured by the Cartesi machine by its label and
// prints one of the three fields (device path, start address, length). The
// invocation name (argv[0]) filters the device type:
//
//   memoryrange  - any device type
//   flashdrive   - only /dev/pmem*
//   nvram        - only /dev/uio*

#include "labelinfo.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

enum field { FIELD_NOTSET, FIELD_DEVICE, FIELD_START, FIELD_LENGTH };

static int set_mode(const char *prog, enum field *mode, enum field to) {
    if (*mode != FIELD_NOTSET) {
        fprintf(stderr, "%s: conflicting output flags\n", prog);
        return -1;
    }
    *mode = to;
    return 0;
}

int main(int argc, char **argv) {
    const char *prog = strrchr(argv[0], '/');
    prog = prog ? prog + 1 : argv[0];

    const char *want_prefix = NULL;
    if (strcmp(prog, "flashdrive") == 0) {
        want_prefix = "/dev/pmem";
    } else if (strcmp(prog, "nvram") == 0) {
        want_prefix = "/dev/uio";
    } else if (strcmp(prog, "memoryrange") != 0) {
        fprintf(stderr, "%s: must be invoked as flashdrive, nvram, or memoryrange\n", prog);
        return 1;
    }

    enum field mode = FIELD_NOTSET;
    const char *label = NULL;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "--device") == 0) {
            if (set_mode(prog, &mode, FIELD_DEVICE) < 0) {
                return 1;
            }
        } else if (strcmp(a, "--start") == 0) {
            if (set_mode(prog, &mode, FIELD_START) < 0) {
                return 1;
            }
        } else if (strcmp(a, "--length") == 0) {
            if (set_mode(prog, &mode, FIELD_LENGTH) < 0) {
                return 1;
            }
        } else if (a[0] == '-') {
            fprintf(stderr, "%s: unknown option '%s'\n", prog, a);
            return 1;
        } else if (label == NULL) {
            label = a;
        } else {
            fprintf(stderr, "%s: too many arguments\n", prog);
            return 1;
        }
    }
    if (label == NULL) {
        fprintf(stderr, "usage: %s [--device|--start|--length] <label>\n", prog);
        return 1;
    }
    if (mode == FIELD_NOTSET) {
        mode = FIELD_DEVICE;
    }

    struct labelinfo info;
    if (labelinfo_lookup(prog, label, &info) < 0) {
        return 1;
    }

    if (want_prefix != NULL && strncmp(info.device, want_prefix, strlen(want_prefix)) != 0) {
        fprintf(stderr, "%s: label '%s' resolves to '%s', which is not a '%s' device\n",
            prog, label, info.device, want_prefix);
        return 1;
    }

    switch (mode) {
        case FIELD_DEVICE:
            printf("%s\n", info.device);
            break;
        case FIELD_START:
            printf("0x%" PRIx64 "\n", info.start);
            break;
        case FIELD_LENGTH:
            printf("0x%" PRIx64 "\n", info.length);
            break;
        case FIELD_NOTSET:
            break;
    }
    return 0;
}
