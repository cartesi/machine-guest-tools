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
#include <glob.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DT_ALIASES "/proc/device-tree/aliases"
#define DT_BASE    "/proc/device-tree"

static const char *path_basename(const char *path) {
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
}

// Reads up to buf_size bytes from path into buf. Returns bytes read, 0 on empty, -1 on error.
static ssize_t read_file(const char *path, void *buf, size_t buf_size) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) return -1;
    size_t n = fread(buf, 1, buf_size, fp);
    int err = ferror(fp);
    fclose(fp);
    return err ? -1 : (ssize_t) n;
}

// Decode big-endian u64 from 8 bytes at buf+offset.
static uint64_t read_be64(const uint8_t *buf, int offset) {
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) {
        v = (v << 8) | buf[offset + i];
    }
    return v;
}

static int resolve_pmem_dev(const char *prog, uint64_t start, char *dev, size_t dev_size) {
    char pattern[PATH_MAX];
    snprintf(pattern, sizeof(pattern),
        "/sys/devices/platform/%llx.pmem/*/*/*/block/pmem*",
        (unsigned long long) start);
    glob_t g;
    int ret = glob(pattern, 0, NULL, &g);
    if (ret != 0 || g.gl_pathc != 1) {
        fprintf(stderr, "%s: cannot find pmem device at 0x%llx\n", prog, (unsigned long long) start);
        globfree(&g);
        return -1;
    }
    snprintf(dev, dev_size, "/dev/%s", path_basename(g.gl_pathv[0]));
    globfree(&g);
    return 0;
}

static int resolve_uio_dev(const char *prog, uint64_t start, char *dev, size_t dev_size) {
    char pattern[PATH_MAX];
    snprintf(pattern, sizeof(pattern),
        "/sys/devices/platform/%llx.uio/uio/uio*",
        (unsigned long long) start);
    glob_t g;
    int ret = glob(pattern, 0, NULL, &g);
    if (ret != 0 || g.gl_pathc != 1) {
        fprintf(stderr, "%s: cannot find uio device at 0x%llx\n", prog, (unsigned long long) start);
        globfree(&g);
        return -1;
    }
    snprintf(dev, dev_size, "/dev/%s", path_basename(g.gl_pathv[0]));
    globfree(&g);
    return 0;
}

int labelinfo_lookup(const char *prog, const char *label, struct labelinfo *out) {
    // Read alias -> DT path: the file contains a null-terminated string, e.g. "/pmem@80000000\0"
    char alias_path[PATH_MAX];
    snprintf(alias_path, sizeof(alias_path), DT_ALIASES "/%s", label);
    char dt_path[PATH_MAX];
    ssize_t n = read_file(alias_path, dt_path, sizeof(dt_path) - 1);
    if (n < 0) {
        fprintf(stderr, "%s: no such label '%s': %s\n", prog, label, strerror(errno));
        return -1;
    }
    if (n == 0) {
        fprintf(stderr, "%s: empty alias for label '%s'\n", prog, label);
        return -1;
    }
    dt_path[n] = '\0';

    // Read reg: 16 bytes = two big-endian u64s (start, length)
    char reg_path[PATH_MAX];
    snprintf(reg_path, sizeof(reg_path), DT_BASE "%s/reg", dt_path);
    uint8_t reg[16];
    if (read_file(reg_path, reg, sizeof(reg)) != 16) {
        fprintf(stderr, "%s: cannot read reg for label '%s': %s\n", prog, label, strerror(errno));
        return -1;
    }
    out->start  = read_be64(reg, 0);
    out->length = read_be64(reg, 8);

    if (strncmp(dt_path, "/pmem@", 6) == 0) {
        return resolve_pmem_dev(prog, out->start, out->device, sizeof(out->device));
    } else if (strncmp(dt_path, "/uio@", 5) == 0) {
        return resolve_uio_dev(prog, out->start, out->device, sizeof(out->device));
    }
    fprintf(stderr, "%s: label '%s' has unrecognized DT path '%s'\n", prog, label, dt_path);
    return -1;
}
