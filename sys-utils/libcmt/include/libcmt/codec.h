/* Copyright Cartesi and individual authors (see AUTHORS)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CMT_CODEC_H
#define CMT_CODEC_H
#include <errno.h>

#include <libcmt/abi.h>
#include <libcmt/buf.h>
#include <libcmt/util.h>

// -----------------------------------------------------------------------------
// CallVoucher(bytes32,address,uint256,bytes)
typedef enum cmt_call_voucher_funsel : uint32_t {
    cmt_call_voucher_funsel = 0xe2d56260U
} cmt_call_voucher_funsel_t;

typedef struct cmt_call_voucher_args {
    cmt_abi_bytes32_t app_context;
    cmt_abi_address_t destination;
    cmt_abi_u256_t value;
    cmt_buf_t payload; // bytes
} cmt_call_voucher_args_t;

int cmt_call_voucher_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_call_voucher_args_t *args);
int cmt_call_voucher_decode(cmt_buf_t rx, cmt_call_voucher_args_t *args);

// -----------------------------------------------------------------------------
// Erc1155BatchTransfer(bytes32,address,address,(uint256,uint256)[])
typedef enum cmt_erc1155_batch_transfer_funsel : uint32_t {
    cmt_erc1155_batch_transfer_funsel = 0x2e3c5a2dU
} cmt_erc1155_batch_transfer_funsel_t;

typedef struct cmt_erc1155_batch_transfer_args {
    cmt_abi_bytes32_t app_context;
    cmt_abi_address_t recipient;
    cmt_abi_address_t token;
    cmt_buf_t items; // each 64 size in bytes
} cmt_erc1155_batch_transfer_args_t;

int cmt_erc1155_batch_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc1155_batch_transfer_args_t *args);
int cmt_erc1155_batch_transfer_decode(cmt_buf_t rx, cmt_erc1155_batch_transfer_args_t *args);

// -----------------------------------------------------------------------------
// Erc1155Transfer(bytes32,address,address,uint256,uint256)
typedef enum cmt_erc1155_transfer_funsel : uint32_t {
    cmt_erc1155_transfer_funsel = 0x832f11baU
} cmt_erc1155_transfer_funsel_t;

typedef struct cmt_erc1155_transfer_args {
    cmt_abi_bytes32_t app_context;
    cmt_abi_address_t recipient;
    cmt_abi_address_t token;
    cmt_abi_u256_t token_id;
    cmt_abi_u256_t value;
} cmt_erc1155_transfer_args_t;

int cmt_erc1155_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc1155_transfer_args_t *args);
int cmt_erc1155_transfer_decode(cmt_buf_t rx, cmt_erc1155_transfer_args_t *args);

// -----------------------------------------------------------------------------
// Erc20Transfer(bytes32,address,address,uint256)
typedef enum cmt_erc20_transfer_funsel : uint32_t {
    cmt_erc20_transfer_funsel = 0x8cd05bcdU
} cmt_erc20_transfer_funsel_t;

typedef struct cmt_erc20_transfer_args {
    cmt_abi_bytes32_t app_context;
    cmt_abi_address_t recipient;
    cmt_abi_address_t token;
    cmt_abi_u256_t value;
} cmt_erc20_transfer_args_t;

int cmt_erc20_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc20_transfer_args_t *args);
int cmt_erc20_transfer_decode(cmt_buf_t rx, cmt_erc20_transfer_args_t *args);

// -----------------------------------------------------------------------------
// Erc721Transfer(bytes32,address,address,uint256)
typedef enum cmt_erc721_transfer_funsel : uint32_t {
    cmt_erc721_transfer_funsel = 0xfd2181c4U
} cmt_erc721_transfer_funsel_t;

typedef struct cmt_erc721_transfer_args {
    cmt_abi_bytes32_t app_context;
    cmt_abi_address_t recipient;
    cmt_abi_address_t token;
    cmt_abi_u256_t token_id;
} cmt_erc721_transfer_args_t;

int cmt_erc721_transfer_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_erc721_transfer_args_t *args);
int cmt_erc721_transfer_decode(cmt_buf_t rx, cmt_erc721_transfer_args_t *args);

// -----------------------------------------------------------------------------
// EvmAdvance(uint64,address,address,uint64,uint64,uint256,uint64,bytes)
typedef enum cmt_evm_advance_funsel : uint32_t {
    cmt_evm_advance_funsel = 0xbf0e3a23U
} cmt_evm_advance_funsel_t;

typedef struct cmt_evm_advance_args {
    uint64_t chain_id;
    cmt_abi_address_t app_contract;
    cmt_abi_address_t msg_sender;
    uint64_t block_number;
    uint64_t block_timestamp;
    cmt_abi_u256_t prev_randao;
    uint64_t index;
    cmt_buf_t payload; // bytes
} cmt_evm_advance_args_t;

int cmt_evm_advance_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_evm_advance_args_t *args);
int cmt_evm_advance_decode(cmt_buf_t rx, cmt_evm_advance_args_t *args);

// -----------------------------------------------------------------------------
// Notice(bytes32,bytes)
typedef enum cmt_notice_funsel : uint32_t {
    cmt_notice_funsel = 0x53742addU
} cmt_notice_funsel_t;

typedef struct cmt_notice_args {
    cmt_abi_bytes32_t app_context;
    cmt_buf_t payload; // bytes
} cmt_notice_args_t;

int cmt_notice_encode(cmt_buf_t tx, cmt_buf_t *out, const cmt_notice_args_t *args);
int cmt_notice_decode(cmt_buf_t rx, cmt_notice_args_t *args);

#endif /* CMT_CODEC_H */
