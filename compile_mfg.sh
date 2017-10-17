VERSION=$(cat Makefile | grep EXTRAV | grep 0541 | awk -F"-" '{print $2"-"$3}')
#make distclean
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0500aa0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0500aa0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0500ab0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0500ab0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0500ac0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0500ac0101-$VERSION
