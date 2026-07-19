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

#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include "hex.hpp"

static void check(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

template <typename E, typename F>
static void check_throws(F &&function, std::string_view message) {
    try {
        function();
    } catch (const E &) {
        return;
    }
    throw std::runtime_error{std::string{message}};
}

int main() try {
    const std::array<uint8_t, 8> known{{0x00, 0x01, 0x0f, 0x10, 0xab, 0xcd, 0xef, 0xff}};
    const auto encoded = cartesi::encode_hex(known);
    check(encoded == "0x00010f10abcdefff", "known encoding mismatch");
    check(cartesi::decode_hex(encoded) == std::string_view(reinterpret_cast<const char *>(known.data()), known.size()),
        "known decoding mismatch");
    check(cartesi::decode_hex(std::string_view{"0X00010F10ABCDEFFF"}) ==
            std::string_view(reinterpret_cast<const char *>(known.data()), known.size()),
        "uppercase decoding mismatch");
    check(cartesi::encode_hex(std::string_view{}) == "0x", "empty encoding mismatch");
    check(cartesi::decode_hex(std::string_view{"0x"}).empty(), "empty decoding mismatch");

    std::array<uint8_t, 256> all{};
    for (size_t i = 0; i < all.size(); ++i) {
        all[i] = static_cast<uint8_t>(i);
    }
    const auto all_encoded = cartesi::encode_hex(all);
    check(cartesi::decode_hex(all_encoded) ==
            std::string_view(reinterpret_cast<const char *>(all.data()), all.size()),
        "all-byte round trip mismatch");

    std::array<char, 4> digits{};
    const std::array<uint8_t, 2> bytes{{0xab, 0xcd}};
    cartesi::encode_hex_digits(std::as_bytes(std::span{bytes}), digits);
    check(std::string_view(digits.data(), digits.size()) == "abcd", "bulk encoding mismatch");
    std::array<uint8_t, 2> decoded{};
    cartesi::decode_hex_digits(std::as_bytes(std::span{digits}), std::as_writable_bytes(std::span{decoded}));
    check(decoded == bytes, "bulk decoding mismatch");

    check_throws<std::invalid_argument>(
        [&] { cartesi::encode_hex_digits(std::as_bytes(std::span{bytes}), std::span<char>{digits}.first(3)); },
        "bulk encoding accepted an invalid output size");
    check_throws<std::invalid_argument>(
        [&] {
            cartesi::decode_hex_digits(std::as_bytes(std::span<const char>{digits}.first(3)),
                std::as_writable_bytes(std::span{decoded}));
        },
        "bulk decoding accepted odd input");
    check_throws<std::invalid_argument>(
        [&] {
            cartesi::decode_hex_digits(
                std::as_bytes(std::span{digits}), std::as_writable_bytes(std::span<uint8_t>{decoded}.first(1)));
        },
        "bulk decoding accepted an invalid output size");
    check_throws<std::domain_error>([] { static_cast<void>(cartesi::decode_hex(std::string_view{"0010"})); },
        "decoder accepted a missing prefix");
    check_throws<std::domain_error>([] { static_cast<void>(cartesi::decode_hex(std::string_view{"0x0"})); },
        "decoder accepted odd input");
    check_throws<std::domain_error>([] { static_cast<void>(cartesi::decode_hex(std::string_view{"0x0g"})); },
        "decoder accepted an invalid character");

    return 0;
} catch (const std::exception &e) {
    fprintf(stderr, "hex-test: %s\n", e.what());
    return 1;
}
