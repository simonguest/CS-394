# Downloading and Building llama.cpp

## Clone llama.cpp

```
git clone https://github.com/ggerganov/llama.cpp.git
mkdir build
cd build
```

## Build static libraries (required for Unreal Plugin)

```
cmake .. \                                                                                                                                
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DGGML_METAL=ON

# Build everything
make -j8
```

## Build dynamic libraries (required for custom Unity backend)

```
cmake .. -DBUILD_SHARED_LIBS=ON
cmake --build . --config Release -j 8
```