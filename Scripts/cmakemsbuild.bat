set CMAKE_TOOLCHAIN_FILE=C:/Users/Queen/.ezvcpkg/af2287382b1991dbdcb7e5112d236f3323b9dd7a/scripts/buildsystems/vcpkg.cmake

cd ..
rmdir /q cmakemsbuild
mkdir cmakemsbuild
cmake -S . -B cmakemsbuild

cd cmakemsbuild
MSBuild WalnutApp.sln -t:Rebuild -p:Configuration=Release;WarningLevel=2;OutDir=bin\Release\  
