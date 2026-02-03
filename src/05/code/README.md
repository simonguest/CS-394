# Downloading and Building llama.cpp

## To download llama.cpp

```
git clone https://github.com/ggerganov/llama.cpp.git
mkdir build
cd build
```

## Build for Mac/Metal

```
cmake .. \                                                                                                                                
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DGGML_METAL=ON

# Build everything
make -j8
```

## Copy built libraries to Unreal demo folder

```
find . -name "*.a" -exec cp {} ../../LlamaDemo/Plugins/LlamaCppPlugin/Source/ThirdParty/llama.cpp/ \;
```