VERSION=$(cat Makefile | grep EXTRAV | grep 0533 | awk -F"-" '{print $2"-"$3}')
make distclean
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510ab0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ab0101-$VERSION
