// Copyright Cartesi and individual authors (see AUTHORS)
// SPDX-License-Identifier: Apache-2.0

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "base64.hpp"

static void check(bool condition, const char *message) {
    if (!condition) {
        throw std::runtime_error{message};
    }
}

template <typename E, typename F>
static void check_throws(F &&function, const char *message) {
    try {
        std::forward<F>(function)();
    } catch (const E &) {
        return;
    }
    throw std::runtime_error{message};
}

int main() try {
    constexpr std::array vectors{
        std::pair{std::string_view{""}, std::string_view{""}},
        std::pair{std::string_view{"f"}, std::string_view{"Zg=="}},
        std::pair{std::string_view{"fo"}, std::string_view{"Zm8="}},
        std::pair{std::string_view{"foo"}, std::string_view{"Zm9v"}},
        std::pair{std::string_view{"foob"}, std::string_view{"Zm9vYg=="}},
        std::pair{std::string_view{"fooba"}, std::string_view{"Zm9vYmE="}},
        std::pair{std::string_view{"foobar"}, std::string_view{"Zm9vYmFy"}},
    };
    for (const auto &[plain, encoded] : vectors) {
        check(cartesi::encode_base64(plain) == encoded, "known encoding mismatch");
        check(cartesi::decode_base64(encoded) == plain, "known decoding mismatch");
    }

    std::array<uint8_t, 256> all{};
    for (size_t i = 0; i < all.size(); ++i) {
        all[i] = static_cast<uint8_t>(i);
    }
    const auto all_encoded = cartesi::encode_base64(all);
    check(cartesi::decode_base64(all_encoded) == std::string_view(reinterpret_cast<const char *>(all.data()), all.size()),
        "all-byte round trip mismatch");
    check(cartesi::decode_base64(std::string_view{" \tZm9v\r\nYmFy\v\f"}) == "foobar",
        "whitespace decoding mismatch");

    const std::array<uint8_t, 3> bytes{{'f', 'o', 'o'}};
    std::array<char, 4> digits{};
    cartesi::encode_base64_digits(std::as_bytes(std::span{bytes}), digits);
    check(std::string_view{digits.data(), digits.size()} == "Zm9v", "bulk encoding mismatch");
    std::array<uint8_t, 3> decoded{};
    const auto decoded_size =
        cartesi::decode_base64_digits(std::as_bytes(std::span{digits}), std::as_writable_bytes(std::span{decoded}));
    check(decoded_size == decoded.size(), "bulk decoded size mismatch");
    check(decoded == bytes, "bulk decoding mismatch");

    check_throws<std::invalid_argument>(
        [&] { cartesi::encode_base64_digits(std::as_bytes(std::span{bytes}), std::span<char>{digits}.first(3)); },
        "bulk encoder accepted an invalid output size");
    check_throws<std::invalid_argument>(
        [&] {
            cartesi::decode_base64_digits(
                std::as_bytes(std::span{digits}), std::as_writable_bytes(std::span{decoded}.first(2)));
        },
        "bulk decoder accepted an invalid output size");
    check_throws<std::length_error>(
        [] { static_cast<void>(cartesi::encoded_base64_size(std::numeric_limits<size_t>::max())); },
        "encoded size accepted an overflow");

    for (const auto malformed :
        {"A", "AAA", "====", "=AAA", "A===", "AA=A", "AA==AA==", "Zm=8", "AB==", "AAB=", "Zm9v!"}) {
        check_throws<std::domain_error>(
            [malformed] { static_cast<void>(cartesi::decode_base64(std::string_view{malformed})); },
            "decoder accepted malformed Base64");
    }
    return 0;
} catch (const std::exception &error) {
    fprintf(stderr, "base64-test: %s\n", error.what());
    return 1;
}
