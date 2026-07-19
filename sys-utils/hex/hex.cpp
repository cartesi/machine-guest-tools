// Copyright Cartesi and individual authors (see AUTHORS)
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <span>

#include "hex.hpp"

static int write_all(const void *data, size_t size) {
    const auto *bytes = static_cast<const uint8_t *>(data);
    while (size > 0) {
        const size_t written = fwrite(bytes, 1, size, stdout);
        if (written == 0) {
            fprintf(stderr, "error writing to stdout: %s\n", strerror(errno));
            return 1;
        }
        bytes += written;
        size -= written;
    }
    return 0;
}

static int encode(bool prefix) {
    std::array<uint8_t, 1024> input{};
    std::array<char, 2 * input.size()> output{};

    if (prefix && write_all("0x", 2) != 0) {
        return 1;
    }
    for (;;) {
        const size_t size = fread(input.data(), 1, input.size(), stdin);
        if (size > 0) {
            const auto input_span = std::as_bytes(std::span{input}.first(size));
            const auto output_span = std::span{output}.first(2 * size);
            cartesi::encode_hex_digits(input_span, output_span);
            if (write_all(output.data(), output_span.size()) != 0) {
                return 1;
            }
        }
        if (size != input.size()) {
            if (ferror(stdin)) {
                fprintf(stderr, "error reading stdin: %s\n", strerror(errno));
                return 1;
            }
            if (feof(stdin)) {
                return 0;
            }
        }
    }
}

static int consume_prefix() {
    const int first = fgetc(stdin);
    const int second = fgetc(stdin);
    if (first != '0' || (second != 'x' && second != 'X')) {
        if (ferror(stdin)) {
            fprintf(stderr, "error reading stdin: %s\n", strerror(errno));
        } else {
            fprintf(stderr, "hex string must start with 0x\n");
        }
        return 1;
    }
    return 0;
}

static int decode(bool prefix) {
    std::array<char, 1024> input{};
    std::array<uint8_t, input.size() / 2> output{};

    if (prefix && consume_prefix() != 0) {
        return 1;
    }

    size_t carry = 0;
    for (;;) {
        const size_t requested = input.size() - carry;
        const size_t size = fread(input.data() + carry, 1, requested, stdin);
        const size_t total = carry + size;
        const size_t digits = total & ~size_t{1};

        if (digits > 0) {
            const auto input_span = std::as_bytes(std::span{input}.first(digits));
            const auto output_span = std::as_writable_bytes(std::span{output}.first(digits / 2));
            cartesi::decode_hex_digits(input_span, output_span);
            if (write_all(output.data(), output_span.size()) != 0) {
                return 1;
            }
        }

        carry = total - digits;
        if (carry != 0) {
            input[0] = input[digits];
        }

        if (size != requested) {
            if (ferror(stdin)) {
                fprintf(stderr, "error reading stdin: %s\n", strerror(errno));
                return 1;
            }
            if (feof(stdin)) {
                if (carry != 0) {
                    fprintf(stderr, "hex string length must be even\n");
                    return 1;
                }
                return 0;
            }
        }
    }
}

static void print_help() {
    fprintf(stderr, R"(Usage:
  hex [options]

    --encode
      encode hex from stdin to stdout

    --decode
      decode hex from stdin to stdout

    --prefix
      output 0x prefix when encoding and expect it when decoding

    --no-prefix
      do not add 0x prefix when encoding or expect 0x when decoding

    --help
      print this help message

)");
}

int main(int argc, char *argv[]) try {
    bool decoding = true;
    bool prefix = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--decode") == 0) {
            decoding = true;
        } else if (strcmp(argv[i], "--encode") == 0) {
            decoding = false;
        } else if (strcmp(argv[i], "--no-prefix") == 0) {
            prefix = false;
        } else if (strcmp(argv[i], "--prefix") == 0) {
            prefix = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        }
    }
    return decoding ? decode(prefix) : encode(prefix);
} catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return 1;
}
