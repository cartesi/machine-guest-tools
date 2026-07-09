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

#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern "C" {
#include "libcmt/codec.h"
#include "libcmt/rollup.h"
};

#include "json.hpp"

// RAII file descriptor implementation
class rollup {
public:
    rollup(bool load_merkle = true) : rollup(NULL, load_merkle) {}
    rollup(cmt_buf_t *tx, bool load_merkle = true) {
        if (cmt_rollup_init(&m_rollup, tx))
            throw std::system_error(errno, std::generic_category(),
                "Unable to initialize. Try runnning again with CMT_DEBUG=yes'");
        if (load_merkle)
            cmt_merkle_load(cmt_rollup_get_merkle(&m_rollup), "/tmp/merkle.dat");
    }
    ~rollup() {
        cmt_merkle_save(cmt_rollup_get_merkle(&m_rollup), "/tmp/merkle.dat");
        cmt_rollup_fini(&m_rollup);
    }
    operator cmt_rollup_t *(void) {
        return &m_rollup;
    }
    uint64_t get_leaf_count(void) {
        return cmt_merkle_get_leaf_count(cmt_rollup_get_merkle(&m_rollup));
    }

private:
    cmt_rollup_t m_rollup;
};

// Print help message with program usage
static void print_help(void) {
    std::cerr <<
        R"(Usage:
    rollup [command]

  where [command] is one of

    call-voucher
      emit a call voucher read from stdin as a JSON object in the format
        {
            "destination": <address>,
            "value": <hex-uint256>,
            "payload": <hex-data>,
            "app_context": <hex-uint256>
        }
      where
        <address> contains a 20-byte EVM address in hex,
        <hex-uint256> contains a big-endian 32-byte unsigned integer in hex, and
        <hex-data> contains arbitrary data in hex
      if successful, prints to stdout a JSON object in the format
        {"index": <number> }
      where field "index" is the index allocated for the call voucher

    erc1155-batch-transfer
      emit an ERC-1155 batch transfer read from stdin as a JSON object in the format
        {
            "recipient": <address>,
            "token": <address>,
            "items": <hex-data>,
            "app_context": <hex-uint256>
        }
      where
        <address> contains a 20-byte EVM address in hex,
        <hex-data> contains arbitrary data in hex (each item is 64 bytes)
      if successful, prints to stdout a JSON object in the format
        {"index": <number> }
      where field "index" is the index allocated for the transfer

    erc1155-transfer
      emit an ERC-1155 transfer read from stdin as a JSON object in the format
        {
            "recipient": <address>,
            "token": <address>,
            "token_id": <hex-uint256>,
            "value": <hex-uint256>,
            "app_context": <hex-uint256>
        }
      where
        <address> contains a 20-byte EVM address in hex,
        <hex-uint256> contains a big-endian 32-byte unsigned integer in hex
      if successful, prints to stdout a JSON object in the format
        {"index": <number> }
      where field "index" is the index allocated for the transfer

    erc20-transfer
      emit an ERC-20 transfer read from stdin as a JSON object in the format
        {
            "recipient": <address>,
            "token": <address>,
            "value": <hex-uint256>,
            "app_context": <hex-uint256>
        }
      where
        <address> contains a 20-byte EVM address in hex,
        <hex-uint256> contains a big-endian 32-byte unsigned integer in hex
      if successful, prints to stdout a JSON object in the format
        {"index": <number> }
      where field "index" is the index allocated for the transfer

    erc721-transfer
      emit an ERC-721 transfer read from stdin as a JSON object in the format
        {
            "recipient": <address>,
            "token": <address>,
            "token_id": <hex-uint256>,
            "app_context": <hex-uint256>
        }
      where
        <address> contains a 20-byte EVM address in hex,
        <hex-uint256> contains a big-endian 32-byte unsigned integer in hex
      if successful, prints to stdout a JSON object in the format
        {"index": <number> }
      where field "index" is the index allocated for the transfer

    notice
      emit a notice read from stdin as a JSON object in the format
        {"payload": <hex-data>, "app_context": <hex-uint256> }
      where
        <hex-data> contains arbitrary data in hex
      if successful, prints to stdout a JSON object in the format
        {"index": <number> }
      where field "index" is the index allocated for the notice

    report
      emit a report read from stdin as a JSON object in the format
        {"payload": <hex-data> }
      where
        <hex-data> contains arbitrary data in hex

    finish
      accept or reject the previous request based on a JSON object
      read from stdin in the format
        {"status": <string> }
      where "status" is either "accept" or "reject".

      print the next request to stdout as a JSON object in the format
        {"request_type": <request-type>, "data": <request-data>}

      when field "request_type" contains "advance_state",
      field "data" contains a JSON object in the format
        {
          "chain_id": <number>,
          "app_contract": <address>,
          "msg_sender": <address>,
          "block_number": <number>,
          "block_timestamp": <number>
          "prev_randao": <hex-uint256>,
          "index": <number>,
          "payload": <hex-data>
        },
      where
        <address> contains a 20-byte EVM address in hex,
        <hex-uint256> contains a big-endian 32-byte unsigned integer in hex, and
        <hex-data> contains arbitrary data in hex

      when field "request_type" contains "inspect_state",
      field "data" contains a JSON object in the format
        {"payload": <hex-data> }
      where
        <hex-data> contains arbitrary data in hex

    accept
      a shortcut for finish with implied input
        {"status": "accept" }
      no input is read from stdin

    reject
      a shortcut for finish with implied input
        {"status": "reject" }
      no input is read from stdin

    exception
      throw an exception read from stdin as a JSON object in the format
        {"payload": <hex-data> }
      where
        <hex-data> contains arbitrary data in hex

)";
}

// Read all stdin input into a string
static std::string read_input(void) {
    std::istreambuf_iterator<char> begin(std::cin), end;
    return std::string(begin, end);
}

// Convert a hex character into its corresponding nibble {0..15}
static uint8_t hexnibble(char a) {
    if (a >= 'a' && a <= 'f') {
        return a - 'a' + 10;
    }
    if (a >= 'A' && a <= 'F') {
        return a - 'A' + 10;
    }
    if (a >= '0' && a <= '9') {
        return a - '0';
    }
    throw std::invalid_argument{"invalid hex character"};
    return 0;
}

// Convert two hex character into its corresponding byte {0..255}
static uint8_t hexbyte(char a, char b) {
    return hexnibble(a) << 4 | hexnibble(b);
}

// Convert an hex string into the corresponding bytes
static std::string unhex(const std::string &s) {
    if (s.find_first_not_of("abcdefABCDEF0123456789", 2) != std::string::npos) {
        throw std::invalid_argument{"invalid character in address"};
    }
    std::string res;
    res.reserve(20 + 1);
    for (unsigned i = 2; i < s.size(); i += 2) {
        res.push_back(hexbyte(s[i], s[i + 1]));
    }
    return res;
}

static std::string unhex20(const std::string &s) {
    if (s.size() != 2 + 40) {
        throw std::invalid_argument{"incorrect address size"};
    }
    return unhex(s);
}

static std::string unhex32(const std::string &s) {
    if (s.size() > 2 + 64) {
        throw std::invalid_argument{"incorrect value size"};
    }
    return unhex(s);
}

// Convert binary data into hex string
static std::string hex(const uint8_t *data, uint64_t length) {
    static const char t[] = "0123456789abcdef";
    std::stringstream ss;
    ss << "0x";
    for (uint64_t i = 0; i < length; ++i) {
        char hi = t[(data[i] >> 4) & 0x0f];
        char lo = t[(data[i] >> 0) & 0x0f];
        ss << std::hex << hi << lo;
    }
    return ss.str();
}

// Read input for call voucher data, issue call voucher, write result to output
static int write_call_voucher(void) try {
    cmt_buf_t tx;
    rollup r(&tx);
    auto ji = nlohmann::json::parse(read_input());
    auto payload_bytes = unhex(ji["payload"].get<std::string>());
    auto destination_bytes = unhex20(ji["destination"].get<std::string>());
    auto value_bytes = unhex32(ji["value"].get<std::string>());

    cmt_call_voucher_args_t voucher = {};
    voucher.payload = cmt_buf_make(payload_bytes.size(), reinterpret_cast<unsigned char *>(payload_bytes.data()));
    memcpy(voucher.destination.data, reinterpret_cast<unsigned char *>(destination_bytes.data()),
        destination_bytes.size());
    memcpy(voucher.value.data, reinterpret_cast<unsigned char *>(value_bytes.data()), value_bytes.size());

    if (ji.contains("app_context")) {
        auto app_context_bytes = unhex32(ji["app_context"].get<std::string>());
        memcpy(&voucher.app_context, app_context_bytes.data(), app_context_bytes.size());
    }

    cmt_buf_t out = {};
    uint64_t index = r.get_leaf_count(); // valid on emit_output success
    if (cmt_call_voucher_encode(tx, &out, &voucher) < 0 || cmt_rollup_emit_output(r, out)) {
        return 1;
    }

    nlohmann::json j = {{"index", index}};
    std::cout << j.dump(2) << '\n';

    return 0;
} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return 1;
}

// Read input for notice data, issue notice, write result to output
static int write_notice(void) try {
    cmt_buf_t tx;
    rollup r(&tx);
    auto ji = nlohmann::json::parse(read_input());
    auto payload_bytes = unhex(ji["payload"].get<std::string>());
    cmt_notice_args_t notice = {};
    notice.payload = cmt_buf_make(payload_bytes.size(), reinterpret_cast<unsigned char *>(payload_bytes.data()));

    if (ji.contains("app_context")) {
        auto app_context_bytes = unhex32(ji["app_context"].get<std::string>());
        memcpy(&notice.app_context, app_context_bytes.data(), app_context_bytes.size());
    }

    cmt_buf_t out = {};
    uint64_t index = r.get_leaf_count(); // valid on emit_output success
    if (cmt_notice_encode(tx, &out, &notice) < 0 || cmt_rollup_emit_output(r, out) < 0) {
        return 1;
    }

    nlohmann::json j = {{"index", index}};
    std::cout << j.dump(2) << '\n';

    return EXIT_SUCCESS;

} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return EXIT_FAILURE;
}

// Read input for erc1155-batch-transfer data, issue transfer, write result to
// output
static int write_erc1155_batch_transfer(void) try {
    cmt_buf_t tx;
    rollup r(&tx);
    auto ji = nlohmann::json::parse(read_input());
    auto recipient_bytes = unhex20(ji["recipient"].get<std::string>());
    auto token_bytes = unhex20(ji["token"].get<std::string>());
    auto items_bytes = unhex(ji["items"].get<std::string>());

    cmt_erc1155_batch_transfer_args_t transfer = {};
    memcpy(transfer.recipient.data, reinterpret_cast<unsigned char *>(recipient_bytes.data()), recipient_bytes.size());
    memcpy(transfer.token.data, reinterpret_cast<unsigned char *>(token_bytes.data()), token_bytes.size());
    transfer.items = cmt_buf_make(items_bytes.size(), reinterpret_cast<unsigned char *>(items_bytes.data()));

    if (ji.contains("app_context")) {
        auto app_context_bytes = unhex32(ji["app_context"].get<std::string>());
        memcpy(&transfer.app_context, app_context_bytes.data(), app_context_bytes.size());
    }

    cmt_buf_t out = {};
    uint64_t index = r.get_leaf_count(); // valid on emit_output success
    if (cmt_erc1155_batch_transfer_encode(tx, &out, &transfer) < 0 || cmt_rollup_emit_output(r, out) < 0) {
        return 1;
    }

    nlohmann::json j = {{"index", index}};
    std::cout << j.dump(2) << '\n';

    return EXIT_SUCCESS;

} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return EXIT_FAILURE;
}

// Read input for erc1155-transfer data, issue transfer, write result to output
static int write_erc1155_transfer(void) try {
    cmt_buf_t tx;
    rollup r(&tx);
    auto ji = nlohmann::json::parse(read_input());
    auto recipient_bytes = unhex20(ji["recipient"].get<std::string>());
    auto token_bytes = unhex20(ji["token"].get<std::string>());
    auto token_id_bytes = unhex32(ji["token_id"].get<std::string>());
    auto value_bytes = unhex32(ji["value"].get<std::string>());

    cmt_erc1155_transfer_args_t transfer = {};
    memcpy(transfer.recipient.data, reinterpret_cast<unsigned char *>(recipient_bytes.data()), recipient_bytes.size());
    memcpy(transfer.token.data, reinterpret_cast<unsigned char *>(token_bytes.data()), token_bytes.size());
    memcpy(transfer.token_id.data, reinterpret_cast<unsigned char *>(token_id_bytes.data()), token_id_bytes.size());
    memcpy(transfer.value.data, reinterpret_cast<unsigned char *>(value_bytes.data()), value_bytes.size());

    if (ji.contains("app_context")) {
        auto app_context_bytes = unhex32(ji["app_context"].get<std::string>());
        memcpy(&transfer.app_context, app_context_bytes.data(), app_context_bytes.size());
    }

    cmt_buf_t out = {};
    uint64_t index = r.get_leaf_count(); // valid on emit_output success
    if (cmt_erc1155_transfer_encode(tx, &out, &transfer) < 0 || cmt_rollup_emit_output(r, out) < 0) {
        return 1;
    }

    nlohmann::json j = {{"index", index}};
    std::cout << j.dump(2) << '\n';

    return EXIT_SUCCESS;

} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return EXIT_FAILURE;
}

// Read input for erc20-transfer data, issue transfer, write result to output
static int write_erc20_transfer(void) try {
    cmt_buf_t tx;
    rollup r(&tx);
    auto ji = nlohmann::json::parse(read_input());
    auto recipient_bytes = unhex20(ji["recipient"].get<std::string>());
    auto token_bytes = unhex20(ji["token"].get<std::string>());
    auto value_bytes = unhex32(ji["value"].get<std::string>());

    cmt_erc20_transfer_args_t transfer = {};
    memcpy(transfer.recipient.data, reinterpret_cast<unsigned char *>(recipient_bytes.data()), recipient_bytes.size());
    memcpy(transfer.token.data, reinterpret_cast<unsigned char *>(token_bytes.data()), token_bytes.size());
    memcpy(transfer.value.data, reinterpret_cast<unsigned char *>(value_bytes.data()), value_bytes.size());

    if (ji.contains("app_context")) {
        auto app_context_bytes = unhex32(ji["app_context"].get<std::string>());
        memcpy(&transfer.app_context, app_context_bytes.data(), app_context_bytes.size());
    }

    cmt_buf_t out = {};
    uint64_t index = r.get_leaf_count(); // valid on emit_output success
    if (cmt_erc20_transfer_encode(tx, &out, &transfer) < 0 || cmt_rollup_emit_output(r, out) < 0) {
        return 1;
    }

    nlohmann::json j = {{"index", index}};
    std::cout << j.dump(2) << '\n';

    return EXIT_SUCCESS;

} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return EXIT_FAILURE;
}

// Read input for erc721-transfer data, issue transfer, write result to output
static int write_erc721_transfer(void) try {
    cmt_buf_t tx;
    rollup r(&tx);
    auto ji = nlohmann::json::parse(read_input());
    auto recipient_bytes = unhex20(ji["recipient"].get<std::string>());
    auto token_bytes = unhex20(ji["token"].get<std::string>());
    auto token_id_bytes = unhex32(ji["token_id"].get<std::string>());

    cmt_erc721_transfer_args_t transfer = {};
    memcpy(transfer.recipient.data, reinterpret_cast<unsigned char *>(recipient_bytes.data()), recipient_bytes.size());
    memcpy(transfer.token.data, reinterpret_cast<unsigned char *>(token_bytes.data()), token_bytes.size());
    memcpy(transfer.token_id.data, reinterpret_cast<unsigned char *>(token_id_bytes.data()), token_id_bytes.size());

    if (ji.contains("app_context")) {
        auto app_context_bytes = unhex32(ji["app_context"].get<std::string>());
        memcpy(&transfer.app_context, app_context_bytes.data(), app_context_bytes.size());
    }

    cmt_buf_t out = {};
    uint64_t index = r.get_leaf_count(); // valid on emit_output success
    if (cmt_erc721_transfer_encode(tx, &out, &transfer) < 0 || cmt_rollup_emit_output(r, out) < 0) {
        return 1;
    }

    nlohmann::json j = {{"index", index}};
    std::cout << j.dump(2) << '\n';

    return EXIT_SUCCESS;

} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return EXIT_FAILURE;
}

// Read input for report data, issue report
static int write_report(void) try {
    rollup r;
    auto ji = nlohmann::json::parse(read_input());
    auto payload_bytes = unhex(ji["payload"].get<std::string>());
    const void *data = reinterpret_cast<unsigned char *>(payload_bytes.data());
    size_t length = payload_bytes.size();
    return cmt_rollup_emit_report(r, cmt_buf_make(length, data));
} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return 1;
}

// Read input for exception data, throw exception
static int throw_exception(void) try {
    rollup r;
    auto ji = nlohmann::json::parse(read_input());
    auto payload_bytes = unhex(ji["payload"].get<std::string>());
    const void *data = reinterpret_cast<unsigned char *>(payload_bytes.data());
    size_t length = payload_bytes.size();
    return cmt_rollup_emit_exception(r, cmt_buf_make(length, data));
} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return 1;
}

// Read advance state data from driver, write to output
static int write_advance_state(cmt_buf_t *rx) {
    cmt_evm_advance_args_t advance;

    int n = cmt_evm_advance_decode(*rx, &advance);
    if (n < 0) {
        return n;
    }

    nlohmann::json j = {{"request_type", "advance_state"},
        {"data",
            {
                {"chain_id", advance.chain_id},
                {"app_contract", hex(advance.app_contract.data, std::size(advance.app_contract.data))},
                {"msg_sender", hex(advance.msg_sender.data, std::size(advance.msg_sender.data))},
                {"block_number", advance.block_number},
                {"block_timestamp", advance.block_timestamp},
                {"prev_randao", hex(advance.prev_randao.data, std::size(advance.prev_randao.data))},
                {"index", advance.index},
                {"payload", hex((uint8_t *) cmt_buf_begin(advance.payload), cmt_buf_length(advance.payload))},
            }}};
    std::cout << j.dump(2) << '\n';
    return 0;
}

// Read inspect state data from driver, write to output
static int write_inspect_state(cmt_buf_t *rx) {
    nlohmann::json j = {{"request_type", "inspect_state"},
        {"data",
            {{
                "payload",
                hex((uint8_t *) cmt_buf_begin(*rx), cmt_buf_length(*rx)),
            }}}};
    std::cout << j.dump(2) << '\n';
    return 0;
}

// Finish current request and get next
static int finish_request_and_get_next(bool accept) try {
    rollup r;
    cmt_buf_t rx[1] = {};

    long req_type = cmt_rollup_wait_for_input(r, accept, rx);
    if (req_type < 0) {
        return 1;
    }

    switch (req_type) {
        case CMT_ROLLUP_REQ_TYPE_ADVANCE:
            return write_advance_state(rx);
        case CMT_ROLLUP_REQ_TYPE_INSPECT:
            return write_inspect_state(rx);
        default:
            return 1; // or should we ignore?
    }
} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return 1;
}

// Accept current request and get next
static int accept_request(void) {
    return finish_request_and_get_next(true);
}

// Reject current request and get next
static int reject_request(void) {
    return finish_request_and_get_next(false);
}

// Finish current request and get next
static int finish_request(void) try {
    auto ji = nlohmann::json::parse(read_input());
    auto status = ji["status"].get<std::string>();
    if (status == "accept") {
        return finish_request_and_get_next(true);
    } else if (status == "reject") {
        return finish_request_and_get_next(false);
    } else {
        std::cerr << "invalid status '" << status << "'\n";
        return 1;
    }
} catch (std::exception &x) {
    std::cerr << x.what() << '\n';
    return 1;
}

// Figure out command and run it
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        exit(1);
    }
    const char *command = argv[1];
    if (strcmp(command, "call-voucher") == 0) {
        return write_call_voucher();
    } else if (strcmp(command, "erc1155-batch-transfer") == 0) {
        return write_erc1155_batch_transfer();
    } else if (strcmp(command, "erc1155-transfer") == 0) {
        return write_erc1155_transfer();
    } else if (strcmp(command, "erc20-transfer") == 0) {
        return write_erc20_transfer();
    } else if (strcmp(command, "erc721-transfer") == 0) {
        return write_erc721_transfer();
    } else if (strcmp(command, "notice") == 0) {
        return write_notice();
    } else if (strcmp(command, "report") == 0) {
        return write_report();
    } else if (strcmp(command, "exception") == 0) {
        return throw_exception();
    } else if (strcmp(command, "finish") == 0) {
        return finish_request();
    } else if (strcmp(command, "accept") == 0) {
        return accept_request();
    } else if (strcmp(command, "reject") == 0) {
        return reject_request();
    } else if (strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0) {
        print_help();
        return 0;
    } else {
        std::cerr << "Unexpected command '" << command << "'\n\n";
        return 1;
    }
    return 0;
}
