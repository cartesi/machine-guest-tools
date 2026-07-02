#!/bin/bash
# build a golden version of the different message types with `cast` to test codecs

[ ! -d build/data/ ] && mkdir build/data/

echo "#ifndef DATA_H"
echo "#define DATA_H"
echo "#include <stdint.h>"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_advance_0[] = {"
cast calldata "EvmAdvance(uint64,address,address,uint64,uint64,uint256,uint64,bytes)" \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	0x0000000000000000000000000000000000000003 \
	0x0000000000000000000000000000000000000004 \
	0x0000000000000000000000000000000000000005 \
	0x0000000000000000000000000000000000000006 \
	0x0000000000000000000000000000000000000007 \
	$(cast from-utf8 "EvmAdvance-0") | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_inspect_0[] = {"
echo -en "inspect-0" | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_report_0[] = {"
echo -en "report-0" | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_exception_0[] = {"
echo -en "exception-0" | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_notice_0[] = {"
cast calldata "Notice(bytes32,bytes)" \
	0x00000000000000000000000000000000000000000000000000000000000000ff \
	$(cast from-utf8 'Notice-0') | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_call_voucher_0[] = {"
cast calldata "CallVoucher(bytes32,address,uint256,bytes)" \
	0x00000000000000000000000000000000000000000000000000000000000000ff \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000000000000000000000000000002 \
	$(cast from-utf8 'CallVoucher-0') | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc20_transfer_0[] = {"
cast calldata "Erc20Transfer(bytes32,address,address,uint256)" \
	0x00000000000000000000000000000000000000000000000000000000000000ff \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	0x0000000000000000000000000000000000000003 | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc721_transfer_0[] = {"
cast calldata "Erc721Transfer(bytes32,address,address,uint256)" \
	0x00000000000000000000000000000000000000000000000000000000000000ff \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	0x0000000000000000000000000000000000000003 | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc11551_single_transfer_0[] = {"
cast calldata "Erc1155Transfer(bytes32,address,address,uint256,uint256)" \
	0x00000000000000000000000000000000000000000000000000000000000000ff \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	0x0000000000000000000000000000000000000003 \
	0x0000000000000000000000000000000000000004 | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc11551_batch_transfer_0[] = {"
cast calldata "Erc1155BatchTransfer(bytes32,address,address,(uint256,uint256)[])" \
	0x00000000000000000000000000000000000000000000000000000000000000ff \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	"[($(cast to-uint256 3),$(cast to-uint256 4))]" | xxd -r -p | xxd -c16 -i
echo "};"

echo "#endif /* DATA_H */"
