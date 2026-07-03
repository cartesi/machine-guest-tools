# Cartesi Machine Tools

Cartesi Machine Tools is a C library to facilitate the development of applications running on the Cartesi Machine.
It handles the IO and communication protocol with the machine emulator.

The high level @ref libcmt\_rollup API provides functions for common operations,
such as emitting outputs (notices/vouchers), emitting reports, throwing exceptions, and retrieving the next input.
Check the [cartesi documentation](https://docs.cartesi.io/) for an explanation of the rollup interaction model.

For lower-level control, @ref libcmt\_io provides a thin abstraction of the Linux kernel driver.

And finally, a couple of utility modules used by the high level API are also exposed.
- @ref libcmt\_abi is an Ethereum Virtual Machine Application Binary Interface (EVM-ABI) encoder / decoder.
- @ref libcmt\_codec provides encoding/decoding for known Solidity ABI types (EvmAdvance, Notice, CallVoucher, ERC20Transfer, ERC721Transfer, ERC1155 transfers).
- @ref libcmt\_buf is a bounds-checking buffer.
- @ref libcmt\_merkle is a sparse merkle tree implementation on top of Keccak.
- @ref libcmt\_buf is a bounds-checking buffer.
- @ref libcmt\_merkle is a sparse merkle tree implementation on top of Keccak.
- @ref libcmt\_keccak is the hashing function used extensively by Ethereum.

The header files and a compiled RISC-V version of this library can be found [here](https://github.com/cartesi/machine-guest-tools/).
We also provide `.pc` (pkg-config) files to facilitate linking.

# mock and testing

This library provides a mock implementation of @ref libcmt\_io that is
able to simulate requests and replies via files on the host machine.

- Build it with: `make mock`.
- Install it with: `make install-mock`, use `PREFIX` to specify the installation path:
    (The command below will install the library and headers on `$PWD/_install` directory)
```
make install-mock PREFIX=$PWD/_install
```

## testing

Use the environment variable @p CMT\_INPUTS to inject inputs into applications compiled with the mock.
Outputs will be written to files with names derived from the input name.

example:
```
CMT_INPUTS="0:advance.bin" ./application
```

The first output will generate the file:
```
advance.output-0.bin
```

The first report will generate the file:
```
advance.report-0.bin
```

The first exception will generate the file:
```
advance.exception-0.bin
```

The (verifiable) outputs root hash:
```
advance.outputs_root_hash.bin
```

Progress updates are printed to stderr (no file is generated).

Inputs must follow this syntax, a comma-separated list of reason number followed by a file path:
```
CMT_INPUTS="<reason-number> ':' <filepath> ( ',' <reason-number> ':' <filepath> ) *"
```

For rollup, available reasons are: `0` is advance and `1` is inspect.

File paths must not contain commas.
When the inputs list is exhausted, `cmt_rollup_wait_for_input` returns `-ENODATA`.

In addition to @p CMT\_INPUTS, there is also the @p CMT\_DEBUG variable.
Enabling it will cause additional debug messages to be displayed.

```
CMT_DEBUG=yes ./application
```

## generating inputs

Advance state inputs and outputs are EVM-ABI encoded. Encoding and decoding
can be achieved in multiple ways, including writing tools with this library. A
simple way to generate testing data is to use the @p cast tool from
[foundry](http://book.getfoundry.sh/reference/cast/cast.html) and `xxd`.

Encoding an @p EvmAdvance:
```
cast calldata "EvmAdvance(uint64,address,address,uint64,uint64,uint256,uint64,bytes)" \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	0x0000000000000000000000000000000000000003 \
	0x0000000000000000000000000000000000000004 \
	0x0000000000000000000000000000000000000005 \
	0x0000000000000000000000000000000000000006 \
	0x0000000000000000000000000000000000000007 \
	0x`echo "advance-0" | xxd -p -c0` | xxd -r -p > 0.bin
```

Inspect inputs are raw bytes (no ABI encoding required).
```
echo -en "inspect-0" > 1.bin
```

## parsing outputs

Outputs use direct Solidity function call encoding (see the [Output Indexing specification](https://github.com/cartesi/rollups-contracts/blob/feature/output-indexing-simpl/docs/output-indexing.md)).
Each output includes an `app_context` field (a `bytes32`), free for applications to use as they see fit. Recipients can filter outputs by this value.
For example, a CALL voucher is encoded as:
```
CallVoucher(address,bytes32,uint256,bytes)
  destination = <20-byte address>
  app_context = <32-byte bytes32>
  value       = <32-byte uint256>
  payload     = <bytes>
```

Decode a CallVoucher with `cast`:
```
cast calldata-decode "CallVoucher(address,bytes32,uint256,bytes)" 0x`xxd -p -c0 "$1"`
```

Decode a Notice with `cast`:
```
cast calldata-decode "Notice(bytes32,bytes)" 0x`xxd -p -c0 "$1"`
```

See the @ref libcmt\_codec module for the full list of supported output types (ERC20Transfer, ERC721Transfer, ERC1155SingleTransfer, ERC1155BatchTransfer). All output types include an `app_context` `bytes32` field.
