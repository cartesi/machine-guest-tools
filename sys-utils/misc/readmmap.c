// Copyright Cartesi and individual authors (see AUTHORS)
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

// readmmap <label> <offset> <length>
//
// Resolves <label> via /run/cartesi/memoryranges/, mmaps the associated
// device, and writes <length> bytes starting at <offset> (within the memory
// range) to stdout. Useful for reading from UIO memory ranges (/dev/uioN)
// where ordinary read()/lseek() do not access the device memory.

#include "labelinfo.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// Parse an auto-base (hex/oct/dec) non-negative integer that must consume the
// entire string.
static int parse_arg(const char *s, long long *out) {
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 0);
    if (errno != 0 || end == s || *end != '\0') {
        return -1;
    }
    *out = (long long)v;
    if (*out < 0) {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *prog = strrchr(argv[0], '/');
    prog = prog ? prog + 1 : argv[0];

    if (argc != 4) {
        fprintf(stderr, "usage: %s <label> <offset> <length>\n", prog);
        return 1;
    }
    const char *label = argv[1];
    long long offset = 0;
    long long length = 0;
    if (parse_arg(argv[2], &offset) < 0) {
        fprintf(stderr, "%s: invalid offset '%s'\n", prog, argv[2]);
        return 1;
    }
    if (parse_arg(argv[3], &length) < 0) {
        fprintf(stderr, "%s: invalid length '%s'\n", prog, argv[3]);
        return 1;
    }

    struct labelinfo info;
    if (labelinfo_lookup(prog, label, &info) < 0) {
        return 1;
    }
    // Overflow-safe bounds check: offset <= info.length and length <= info.length - offset.
    if ((uint64_t)offset > info.length || (uint64_t)length > info.length - (uint64_t)offset) {
        fprintf(stderr,
            "%s: [offset=0x%llx, length=0x%llx] exceeds range '%s' size 0x%" PRIx64 "\n",
            prog, offset, length, label, info.length);
        return 1;
    }
    if (length == 0) {
        return 0;
    }

    int fd = open(info.device, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "%s: open(%s): %s\n", prog, info.device, strerror(errno));
        return 1;
    }
    const size_t map_len = (size_t)offset + (size_t)length;
    void *map = mmap(NULL, map_len, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        fprintf(stderr, "%s: mmap(%s, %zu): %s\n", prog, info.device, map_len, strerror(errno));
        close(fd);
        return 1;
    }
    size_t written = fwrite((char *)map + offset, 1, (size_t)length, stdout);
    munmap(map, map_len);
    close(fd);
    if (written != (size_t)length) {
        fprintf(stderr, "%s: short write to stdout (%zu / %lld)\n", prog, written, length);
        return 1;
    }
    return 0;
}
