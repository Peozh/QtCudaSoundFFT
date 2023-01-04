# QtCudaSoundFFT

## Preview
![preview](https://user-images.githubusercontent.com/35257391/210584219-59c8571f-63ea-4953-b0a2-dea50a4811f3.png)

## Used Environment
 - Windows 10 64-bit
 - NVIDIA GPU
 - MSVC 64-bit : build dlls
 - MinGW 64-bit : gendef, build Desktop Qt 6.4.1
 - qmake
 - myCudaForQt.dll, lib : build with nvcc.exe in NVIDIA GPU Computing Toolkit
 - OpenAL32.dll, lib : from OpenAL-Soft

## Build CUDA dll & import library
### build .dll
build .dll from .cu, .h\
nvcc.exe from NVIDIA GPU Computing Toolkit
```shell
nvcc.exe -o myCudaForQt.dll --shared myCudaForQt.cu -I "..\Linking\include" -L "..\Linking\lib" -l cufft
```
### make .lib
generate .def file from .dll\
gendef.exe from MinGW
```shell
gendef.exe .\myCudaForQt.dll
```
generate import library from .def file\
lib.exe from MSVC
```shell
lib.exe /def:myCudaForQt.def /out:myCudaForQt.lib /machine:x64
```

## Build OpenAL Soft
https://github.com/kcat/openal-soft

## Build Qt Project
qmake, .pro
