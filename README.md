
# backwards-chess

## To compile with emscripten
### Update/Install
```
cd emsdk
./emsdk install latest
```
### Compile
```
./emsdk/emsdk activate latest
emcmake cmake -S . -B build
cmake -S . -B build "-DCMAKE_TOOLCHAIN_FILE=./emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
cmake --build build
```