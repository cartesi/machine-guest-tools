export CMT_INPUTS=0:0.bin
export CMT_DEBUG=yes

./ioctl-echo-loop-mock --notices=2
./ioctl-echo-loop-mock --call-vouchers=2
./ioctl-echo-loop-mock --erc1155-batch-transfers=2
./ioctl-echo-loop-mock --erc1155-transfers=2
./ioctl-echo-loop-mock --erc20-transfers=2
./ioctl-echo-loop-mock --erc721-transfers=2
./ioctl-echo-loop-mock --reports=2
