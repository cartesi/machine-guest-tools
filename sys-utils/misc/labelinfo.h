// Copyright Cartesi and individual authors (see AUTHORS)
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

#ifndef CARTESI_MISC_LABELINFO_H
#define CARTESI_MISC_LABELINFO_H

#include <limits.h>
#include <stdint.h>

// Information about a memory range configured by the Cartesi machine and
// looked up via its label in the device tree /aliases node.
struct labelinfo {
    uint64_t start;            // physical start address of the memory range
    uint64_t length;           // size of the memory range in bytes
    char device[PATH_MAX + 1]; // e.g. "/dev/pmem0" or "/dev/uio0"
};

// Resolves <label> via /proc/device-tree/aliases and fills *out with the
// device path and the start/length of the associated memory range. On
// failure, prints an error message prefixed by <prog> to stderr and returns
// -1. On success, returns 0.
int labelinfo_lookup(const char *prog, const char *label, struct labelinfo *out);

#endif // CARTESI_MISC_LABELINFO_H
