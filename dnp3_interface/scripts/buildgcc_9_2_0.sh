#cd /home/build
GCC_VERSION=9.2.0
if [ ! -f gcc-${GCC_VERSION}.tar.gz ] ; then 
   wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz
fi
#exit
tar xzvf gcc-${GCC_VERSION}.tar.gz
rm -rf  obj.gcc-${GCC_VERSION}
mkdir -p obj.gcc-${GCC_VERSION}
cd gcc-${GCC_VERSION}
./contrib/download_prerequisites
cd ../obj.gcc-${GCC_VERSION}
../gcc-${GCC_VERSION}/configure --prefix=/usr/local/gcc-${GCC_VERSION} --disable-multilib --enable-languages=c,c++
make -j $(nproc)
echo " build finished please install when ready" 
echo 'sudo make install'

