#!/bin/sh

function print_help() {
    echo
    echo "Usage:"
    echo "    make_suniv.sh /dev/sdX [pocketgo | trimui | fc3000]"
    echo
    echo "Notes:"
    echo "    fc3000 -> fc3000_tft1, fc3000_tft2, fc3000_ips1, fc3000_ips2"
    echo
    exit
}

if [ "$#" -ne 2 ]; then
    print_help
fi

export PATH=/opt/miyoo/bin:$PATH
echo "bootz 0x80008000 - 0x80C00000" > param.cmd
./tools/mkimage -C none -A arm -T script -d param.cmd param.scr &&
./tools/bin2header param.scr hex_boot > fs/hex_boot.h &&
./tools/bin2header ../kernel/arch/arm/boot/zImage hex_kernel > fs/hex_kernel.h &&
./tools/bin2header ../kernel/arch/arm/boot/dts/$2.dtb hex_dtb > fs/hex_dtb.h &&
ARCH=arm CROSS_COMPILE=arm-linux- make -j4 &&
sudo dd if=u-boot-sunxi-with-spl.bin of=$1 bs=1024 seek=8 conv=notrunc && 
rm -rf param.cmd param.scr &&
sync
echo "task done !"
