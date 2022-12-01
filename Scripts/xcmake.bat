set CMAKE_TOOLCHAIN_FILE=C:/Users/Queen/.ezvcpkg/af2287382b1991dbdcb7e5112d236f3323b9dd7a/scripts/buildsystems/vcpkg.cmake

cd ..
rmdir /q xcmake 
mkdir xcmake

xmake project -k cmake -m "release,debug" xcmake

cd xcmake
cmake -S . -B .
