../newlib-*/configure --prefix=/usr --target=x86_64-monolith64
mkdir -p x86_64-monolith64/newlib/libc/sys/monolith64/.deps
make all -j16
make DESTDIR=~/m64-sysroot install -j16
cp -ar ~/m64-sysroot/usr/x8*/* ~/m64-sysroot/usr/
