@REM set abi=armeabi-v7a
set abi=arm64-v8a

if not exist %abi% md %abi%
cd %abi%

%ANDROID_SDK_HOME%/cmake/3.10.2.4988404/bin/cmake ^
  -DCMAKE_TOOLCHAIN_FILE=%ANDROID_SDK_HOME%/ndk-bundle/build/cmake/android.toolchain.cmake ^
  -DANDROID_ABI=%abi% ^
  -DANDROID_NDK=%ANDROID_SDK_HOME%/ndk-bundle ^
  -DANDROID_PLATFORM=android-22 ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DANDROID_TOOLCHAIN=clang -DCMAKE_GENERATOR="Ninja" ^
  -DCMAKE_MAKE_PROGRAM=%ANDROID_SDK_HOME%/cmake/3.10.2.4988404/bin/ninja ^
  ..

%ANDROID_SDK_HOME%/cmake/3.10.2.4988404/bin/ninja
cd ..