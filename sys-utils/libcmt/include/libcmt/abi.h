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
/** @file
 * @defgroup libcmt_abi abi
 *
 * This is a C library to encode and decode Ethereum Virtual Machine (EVM)
 * calldata. This format is used to interacts with contracts in the Ethereum
 * ecosystem.
 *
 * We will cover the basic concepts required to use the API we provide, but for
 * a complete reference consult the solidity specification, it can be found
 * here: https://docs.soliditylang.org/en/latest/abi-spec.html
 *
 * ## Function Selector {#funsel}
 *
 * 4 bytes that identify the function name and parameter types. This is used to
 * distinguish between different formats. To compute it, take the four first
 * bytes of the `keccak` digest of the solidity function declaration. It should
 * respect a canonical format and such as having no the variable names, check
 * docs for details. For reference, for a hypotetical "FunctionName" function,
 * it should look something like this:
 * `keccak("FunctionName(type1,type2,...,typeN)");`
 *
 * ## Data Sections {#sections}
 *
 * After the function selector, we can start encoding the function parameters.
 * One by one, left to right in two sections. `Static` for fixed sized values.
 * Think ints, bools, addresses, offsets, etc. Then `Dynamic` for variable
 * sized data such as the contents of @b bytes. Values that need a dynamic
 * section will also have an entry in the static section.
 *
 * ### Static Section {#static-section}
 *
 * [uint](@ref cmt_abi_put_uint), [bool](@ref cmt_abi_put_bool) and
 * [address](@ref cmt_abi_put_address) values are encoded directly in the
 * static section. In addition to those, @b bytes gets an entry in both
 * sections. The static part is done with [this](@ref cmt_abi_put_dyn_head) call.
 *
 * ### Dynamic Section {#dynamic-section}
 *
 * The Dynamic section encodes the contents of variable sized types. Every entry
 * in this section requires a corresponding entry in the static section as well.
 *
 * So types with variable size are encoded in both sections.
 *
 * - `static` section gets a reference / offset to the dynamic section.
 * - `dynamic` section gets the actual contents
 *
 * In more concrete terms, the @b bytes type is encoded first with a call to @ref
 * cmt_abi_put_dyn_head for its `static` section part and then with a call to
 * @ref cmt_abi_put_dyn_tail for its `dynamic` section part.
 *
 * ## Encoder
 *
 * Lets look at some code starting with a simple case. A function that encodes
 * the function selector and a single @b address value into the buffer:
 *
 * @includelineno "examples/abi_encode_000.c"
 *
 * For @b bytes, we need both sections. static and dynamic.
 *
 * @includelineno "examples/abi_encode_001.c"
 *
 * For multiple dynamic values, do the "heads" first, then the "tails" as
 * described in the solidity specification.
 *
 * @includelineno "examples/abi_encode_002.c"
 *
 * ## Decoder
 *
 * Lets look at code that decodes the examples above. In this case we can
 * choose one of _get_ or _check_ funsel calls. _get_ is for retrieving the
 * encoded value. While _check_ is more adequate in this case. We passes in a
 * known value that must match what is in the buffer. After that we retrive
 * the address for later processing.
 *
 * @includelineno "examples/abi_decode_000.c"
 *
 * There are two options when dealing with bytes. _get_ a copy of the contents
 * into a user provided buffer. And _view_ to retrieve pointers into the
 * underlying @p rx buffer. This makes the API very lightweight and fast but
 * requires care in its usage. If @p rx gets free'd or reused while there is
 * still a reference to @p data, we'll get memory corruption. For the rollup
 * API, the buffer contents reset for new inputs. If in doublt create a copy of
 * @p data and use it instead.
 *
 * @includelineno "examples/abi_decode_001.c"
 *
 * For multiple dynamic values, do the "heads" first, then the "tails" as
 * described in the solidity specification. Similar to the encoding process.
 *
 * @includelineno "examples/abi_decode_002.c"
 *
 * ## Complete
 *
 * Lets look at some code on how to tie everything together. We'll build a @b
 * echo of sorts that decodes the contents of @p rd and re-encodes it into @p
 * wr. We'll use previous examples as a starting point to implement encode_echo
 * and decode_echo. With the entrypoint being @p f. We'll use @ref
 * cmt_abi_peek_funsel to switch on the message function selector, only one
 * valid case in this example. Decode and if we succeed, encode it back. If the
 * caller needs the encoded size, we can compute it by storing @p wr at the
 * start and compute the difference.
 *
 * @includelineno "examples/abi_multi.c"
 *
 * @ingroup libcmt
 * @{ */
#ifndef CMT_ABI_H
#define CMT_ABI_H
#include "buf.h"
#include <stdbool.h>

enum {
    CMT_ABI_U256_LENGTH = 32,    /**< length of a evm word in bytes */
    CMT_ABI_ADDRESS_LENGTH = 20, /**< length of a evm address in bytes */
};

/** Compile time equivalent to @ref cmt_abi_funsel
 * @note don't port. use @ref cmt_abi_funsel instead */
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#define CMT_ABI_FUNSEL(A, B, C, D)                                                                                     \
    (((uint32_t) (D) << 000) | ((uint32_t) (C) << 010) | ((uint32_t) (B) << 020) | ((uint32_t) (A) << 030))
#else
#define CMT_ABI_FUNSEL(A, B, C, D)                                                                                     \
    (((uint32_t) (A) << 000) | ((uint32_t) (B) << 010) | ((uint32_t) (C) << 020) | ((uint32_t) (D) << 030))
#endif

/** EVM address */
typedef struct cmt_abi_address {
    uint8_t data[CMT_ABI_ADDRESS_LENGTH];
} cmt_abi_address_t;

/** EVM u256 in big endian format */
typedef struct cmt_abi_u256 {
    uint8_t data[CMT_ABI_U256_LENGTH];
} cmt_abi_u256_t;

/** EVM bytes32 */
typedef struct cmt_abi_bytes32 {
    uint8_t data[CMT_ABI_U256_LENGTH];
} cmt_abi_bytes32_t;

typedef struct cmt_abi_frame {
    cmt_buf_t range[1];
} cmt_abi_frame_t;

/** Helper struct for encodings that require a dynamic section  */
typedef struct cmt_abi_dyn_state {
    cmt_abi_frame_t frame[1];
    cmt_buf_t offset[1];
    cmt_buf_t length[1];
} cmt_abi_dyn_state_t;

/** Create a function selector from an array of bytes
 * @param [in] funsel function selector bytes
 * @return
 * - function selector converted to big endian (as expected by EVM) */
uint32_t cmt_abi_funsel(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

/** Create a frame for the dynamic section. Read the EVM ABI for the details.
 * @param [in]  me     reader or writer buffer
 * @param [out] frame  start of the parameters frame
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value |
 */
int cmt_abi_mark_frame(const cmt_buf_t *me, cmt_abi_frame_t *frame);

// put section ---------------------------------------------------------------

/** Encode a function selector into the buffer @p wr.
 *
 * @param [in,out] wr     a initialized buffer working as iterator
 * @param [in]     funsel function selector
 *
 * @return
 * |   |                             |
 * |--:|-----------------------------|
 * |  0| success                     |
 * |< 0| failure with a -errno value |
 *
 * @note A function selector can be compute it with: @ref cmt_keccak_funsel.
 * It is always represented in big endian. */
int cmt_abi_put_funsel(cmt_buf_t *wr, uint32_t funsel);

/** Encode a native endianness unsigned integer of up to 32bytes of data into
 * the buffer
 *
 * @param [in,out] wr   a initialized buffer working as iterator
 * @param [in]     n    size of @p data in bytes
 * @param [in]     data pointer to a integer
 *
 * @return
 * |        |                                          |
 * |-------:|------------------------------------------|
 * |       0| success                                  |
 * |-ENOBUFS| no space left in @p wr                   |
 * |   -EDOM| integer not representable in @p 32 bytes |
 *
 *
 * @code
 * ...
 * cmt_buf_t it = ...;
 * uint64_t x = UINT64_C(0xdeadbeef);
 * cmt_abi_put_uint(&it, sizeof x, &x);
 * ...
 * @endcode */
int cmt_abi_put_uint(cmt_buf_t *wr, size_t length, const void *data);

/** Encode a big endian unsigned integer of up to 32bytes of data into the
 * buffer
 *
 * @param [in,out] wr     a initialized buffer working as iterator
 * @param [in]     length size of @p data in bytes
 * @param [in]     data   pointer to a integer
 *
 * @return
 * |        |                                                   |
 * |-------:|---------------------------------------------------|
 * |       0| success                                           |
 * |-ENOBUFS| no space left in @p wr                            |
 * |   -EDOM| integer not representable in @p data_length bytes |
 *
 * @code
 * ...
 * cmt_buf_t it = ...;
 * uint8_t small[] = {
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
 * };
 * cmt_abi_put_uint(&it, sizeof small, &small);
 * ...
 * uint8_t big[] = {
 *     0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 * };
 * cmt_abi_put_uint(&it, sizeof big, &big);
 * @endcode
 * @note This function takes care of endianness conversions */
int cmt_abi_put_uint_be(cmt_buf_t *wr, size_t length, const void *data);

/** Encode a @ref cmt_abi_u256_t into the buffer
 *
 * @param [in,out] wr     a initialized buffer working as iterator
 * @param [in]     data   pointer to a @ref cmt_abi_u256_t
 *
 * @return
 * |        |                                                   |
 * |-------:|---------------------------------------------------|
 * |       0| success                                           |
 * |-ENOBUFS| no space left in @p wr                            | */
int cmt_abi_put_uint256(cmt_buf_t *wr, const cmt_abi_u256_t *value);

/** Encode a fixed-size byte array (left-aligned, padded to 32 bytes).
 *
 * This is the encoding for Solidity's bytes<M> type where M ≤ 32.
 * Unlike integers which are right-aligned, bytes<M> is left-aligned.
 *
 * @param [in,out] wr    initialized buffer
 * @param [in]     n     number of bytes (1 ≤ n ≤ CMT_ABI_U256_LENGTH)
 * @param [in]     data  pointer to the byte array
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |   -EDOM| n > 32                 |
 * |-ENOBUFS| no space left in @p wr | */
int cmt_abi_put_bytesN(cmt_buf_t *wr, size_t n, const void *data);

/** Encode a bool into the buffer
 *
 * @param [in,out] wr    a initialized buffer working as iterator
 * @param [in]     value boolean value
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p wr |
 *
 * @code
 * ...
 * cmt_buf_t it = ...;
 * cmt_abi_put_bool(&it, true);
 * ...
 * @endcode
 * @note This function takes care of endianness conversions */
int cmt_abi_put_bool(cmt_buf_t *wr, bool value);

/** Encode @p address (exactly @ref CMT_ABI_ADDRESS_LENGTH bytes) into the buffer
 *
 * @param [in,out] wr      initialized buffer
 * @param [in]     address a value of type @ref cmt_abi_address_t
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p wr | */
int cmt_abi_put_address(cmt_buf_t *wr, const cmt_abi_address_t *address);

/** Encode the static part (head) of dynamically sized value,
 * bytes/strings/arrays. Used in conjunction with @ref cmt_abi_put_dyn_tail,
 * @ref cmt_abi_reserve_dyn_tail and @ref cmt_abi_commit_dyn_tail
 *
 * @param [in,out] wr     initialized buffer
 * @param [out]    state  uninitialized helper struct
 * @param [in]     frame  initialized frame, usually after funsel
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p wr | */
int cmt_abi_put_dyn_head(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, cmt_abi_frame_t *frame);

/** Reserve all unused space at the end of @p wr encoder.
 *
 * Then encodes the dynamic item,
 * then commits the space used with @ref cmt_abi_commit_dyn_tail.
 *
 * @param [in,out] wr        initialized buffer
 * @param [in]     state     initialized helper struct
 * @param [out]    available slice of bytes at end of @p wr
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p me |
 *
 * @note: @p available must outlive @p wr
 */
int cmt_abi_reserve_dyn_tail(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, cmt_buf_t *available);

/** Commit previously reserved dynamic tail space after writing `count` items
 * of data, each `size` bytes long, to the buffer obtained from a previous
 * @ref cmt_abi_reserve_dyn_tail call.
 *
 * @param [in,out] wr         initialized writer buffer; its cursor is advanced
 *                            by the total committed size (including 32-byte
 *                            padding)
 * @param [in]     state      helper struct initialized by @ref
 *                            cmt_abi_put_dyn_head or @ref cmt_abi_reserve_dyn_tail;
 *                            must not be modified between reservation and commit
 * @param [in]     size       size in bytes of each item (1 for bytes/strings,
 *                            32 for uint256, etc.); total data size is
 *                            @p size * @p count
 * @param [in]     count      number of items of @p size bytes each; the data
 *                            for these items must have been written to the
 *                            buffer obtained from @ref cmt_abi_reserve_dyn_tail
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-EINVAL | invalid arguments        |
 * |-ENOBUFS| no space left in @p wr |
 *
 * NOTE: Consider using @ref cmt_abi_put_dyn_tail if the length is already known.
 */
int cmt_abi_commit_dyn_tail(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, size_t size, size_t count);

/** Encode the contents of @p data as a dynamic array into the message.
 * The element count @c n is derived as @c cmt_buf_length(data)/size .
 *
 * Combination @ref cmt_abi_reserve_dyn + @ref memcpy + @ref cmt_abi_commit_dyn
 *
 * @param [in,out] wr         initialized buffer
 * @param [in,out] state      initialized helper struct
 * @param [in]     size       size in bytes of each item (1 for bytes/strings, 32 for uint256, etc.)
 * @param [in]     data       buffer whose contents are copied to the dynamic tail
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p me | */
int cmt_abi_put_dyn_tail(cmt_buf_t *wr, cmt_abi_dyn_state_t *state, size_t size, cmt_buf_t data);

// get section ---------------------------------------------------------------

/** Read the funsel without consuming it from the buffer @p me, 0 is returned
 * if there are less than 4 bytes in the buffer.
 *
 * @return
 * |        |                         |
 * |-------:|-------------------------|
 * |    != 0| function selector value |
 * |       0| failure                 |
 *
 * @code
 * ...
 * if (cmt_buf_length(*it) < 4)
 * 	return EXIT_FAILURE;
 * switch (cmt_abi_peek_funsel(it) {
 * case CMT_ABI_FUNSEL(...): // known type, try to parse it
 * case CMT_ABI_FUNSEL(...): // known type, try to parse it
 * default:
 * 	return EXIT_FAILURE;
 * }
 * @endcode */
uint32_t cmt_abi_peek_funsel(cmt_buf_t *me);

/** Consume funsel from the buffer @p me and ensure it matches @p expected_funsel
 *
 * @param [in,out] me       initialized buffer
 * @param [in]     expected expected function selector
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p me |
 * |-EBADMSG| funsel mismatch       | */
int cmt_abi_check_funsel(cmt_buf_t *me, uint32_t expected);

/** Decode a @ref cmt_abi_u256_t from the buffer
 *
 * @param [in,out] rd     initialized buffer
 * @param [out]    data   value of type @ref cmt_abi_u256_t
 *
 * @return
 * |        |                                                   |
 * |-------:|---------------------------------------------------|
 * |       0| success                                           |
 * |-ENOBUFS| no space left in @p rd                            | */
int cmt_abi_get_uint256(cmt_buf_t *rd, cmt_abi_u256_t *value);

/** Decode a fixed-size byte array from ABI format.
 *
 * Reads 32 bytes from the buffer and copies the first @p n bytes to @p data.
 *
 * @param [in,out] rd    initialized buffer
 * @param [in]     n     number of bytes to extract (1 ≤ n ≤ 32)
 * @param [out]    data  pointer to output byte array
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |   -EDOM| n > 32                 |
 * |-ENOBUFS| no space left in @p rd | */
int cmt_abi_get_bytesN(cmt_buf_t *rd, size_t n, void *data);

/** Decode a unsigned integer of up to 32bytes, in native endianness, from the buffer
 *
 * @param [in,out] rd     initialized buffer
 * @param [in]     n      size of @p data in bytes
 * @param [out]    data   pointer to a integer
 *
 * @return
 * |        |                                                   |
 * |-------:|---------------------------------------------------|
 * |       0| success                                           |
 * |-ENOBUFS| no space left in @p rd                            |
 * |   -EDOM| integer not representable in @p data_length bytes | */
int cmt_abi_get_uint(cmt_buf_t *rd, size_t n, void *data);

/** Decode @p length big-endian bytes, up to 32, from the buffer into @p data
 *
 * @param [in,out] rd     initialized buffer
 * @param [in]     length size of @p data in bytes
 * @param [out]    data   pointer to a integer
 *
 * @return
 * |        |                                                   |
 * |-------:|---------------------------------------------------|
 * |       0| success                                           |
 * |-ENOBUFS| no space left in @p rd                            |
 * |   -EDOM| integer not representable in @p data_length bytes | */
int cmt_abi_get_uint_be(cmt_buf_t *rd, size_t n, void *data);

/** Consume and decode a bool from the buffer
 *
 * @param [in,out] rd    a initialized buffer working as iterator
 * @param [out]    value boolean value
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p rd |
 *
 * @code
 * ...
 * cmt_buf_t it = ...;
 * bool value;
 * int rc = cmt_abi_put_bool(&it, &value);
 * assert(rc == 0);
 * ...
 * @endcode
 * @note This function takes care of endianness conversions */
int cmt_abi_get_bool(cmt_buf_t *rd, bool *value);

/** Consume and decode @b address from the buffer
 *
 * @param [in,out] rd      initialized buffer
 * @param [out]    address value of type @ref cmt_abi_address_t
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p rd | */
int cmt_abi_get_address(cmt_buf_t *rd, cmt_abi_address_t *address);

/** Consume and decode the offset of the dynamic value
 *
 * @param [in,out] rd     initialized buffer
 * @param [out]    state  uninitialized dynamic state
 * @param [in]     frame  initialized frame, usually after funsel
 *
 * @return
 * |        |                        |
 * |-------:|------------------------|
 * |       0| success                |
 * |-ENOBUFS| no space left in @p rd | */
int cmt_abi_get_dyn_head(cmt_buf_t *rd, cmt_abi_dyn_state_t *state, cmt_abi_frame_t *frame);

/** Create a view into `data`, encoded in a buffer. The `size` in bytes of each
 * item must be known and provided, `n` (the count) is computed from it and returned in `n`.
 *
 * @param [in]  state      initialized dynamic state
 * @param [in]  size       size in bytes of each item (1 for bytes/strings, 32 for uint256, etc.)
 * @param [out] data       memory range with contents
 *
 * @return
 * |        |                             |
 * |-------:|-----------------------------|
 * |       0| success                     |
 * |     < 0| failure with a -errno value |
 *
 * @note @p state can be initialized by calling @ref cmt_abi_get_dyn_head */
int cmt_abi_view_dyn_tail(cmt_abi_dyn_state_t *state, size_t size, cmt_buf_t *data);

/** Copy @b bytes from the buffer into the user provided buffer
 *
 * @param [in]  state      initialized dynamic state
 * @param [in]  size       size in bytes of each item (1 for bytes/strings, 32 for uint256, etc.)
 * @param [in]  max        capacity of @p data in bytes. Fails with `-ENOBUFS` if `size x n` exceeds this limit.
 * @param [out] data       pre-allocated buffer
 *
 * @return
 * |        |                             |
 * |-------:|-----------------------------|
 * |       0| success                     |
 * |-ENOBUFS| `data` buffer is too small  |
 * |     < 0| failure with a -errno value |
 */
int cmt_abi_get_dyn_tail(cmt_abi_dyn_state_t *state, size_t size, size_t max, void *data);

#endif /* CMT_ABI_H */
/** @} */
