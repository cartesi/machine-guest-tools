#!/bin/bash
# build a golden version of the different message types with `cast` to test codecs

[ ! -d build/data/ ] && mkdir build/data/

echo "#ifndef DATA_H"
echo "#define DATA_H"
echo "#include <stdint.h>"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_advance_0[] = {"
cast calldata "EvmAdvance(uint256,address,address,uint256,uint256,uint256,uint256,bytes)" \
	0x0000000000000000000000000000000000000001 \
	0x0000000000000000000000000000000000000002 \
	0x0000000000000000000000000000000000000003 \
	0x0000000000000000000000000000000000000004 \
	0x0000000000000000000000000000000000000005 \
	0x0000000000000000000000000000000000000006 \
	0x0000000000000000000000000000000000000007 \
	0x`echo -en "advance-0" | xxd -p -c0` | xxd -r -p | xxd -c16 -i
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
cast calldata "Output1(bytes32[1],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.notice)]" \
	$(cast from-utf8 'notice-0') | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_call_voucher_0[] = {"
cast calldata "Output2(bytes32[2],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.call-voucher),$(cast to-uint256 1)]" \
	$(cast abi-encode '_(uint256,bytes)' $(cast to-uint256 2) $(cast from-utf8 'voucher-0')) | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_delegatecall_voucher_0[] = {"
cast calldata "Output2(bytes32[2],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.delegatecall-voucher),$(cast to-uint256 1)]" \
	$(cast from-utf8 'delegatecall-voucher-0') | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc20_transfer_0[] = {"
cast calldata "Output3(bytes32[3],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.erc20-transfer),$(cast to-uint256 1),$(cast to-uint256 2)]" \
	$(cast to-uint256 3) | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc721_transfer_0[] = {"
cast calldata "Output3(bytes32[3],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.erc721-transfer),$(cast to-uint256 1),$(cast to-uint256 2)]" \
	$(cast to-uint256 3) | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc11551_single_transfer_0[] = {"
cast calldata "Output3(bytes32[3],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.erc1155-single-transfer),$(cast to-uint256 1),$(cast to-uint256 2)]" \
	$(cast abi-encode '_(uint256,uint256)' $(cast to-uint256 3) $(cast to-uint256 4)) | xxd -r -p | xxd -c16 -i
echo "};"
echo
echo "// NOLINTNEXTLINE cppcoreguidelines-avoid-non-const-global-variables"
echo "uint8_t valid_erc11551_batch_transfer_0[] = {"
cast calldata "Output3(bytes32[3],bytes)" \
	"[$(cast keccak256 cartesi.output.v1.erc1155-batch-transfer),$(cast to-uint256 1),$(cast to-uint256 2)]" \
	$(cast abi-encode '_((uint256,uint256)[])' "[($(cast to-uint256 3),$(cast to-uint256 4))]") | xxd -r -p | xxd -c16 -i
echo "};"

echo "#endif /* DATA_H */"
