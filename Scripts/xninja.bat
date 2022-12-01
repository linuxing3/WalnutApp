cd ..
rmdir /q xninja
mkdir xninja

xmake project -k ninja -m "release,debug" xninja
cd xninja
ninja build
