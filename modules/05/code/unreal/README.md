# Building the LlamaDemo Unreal Project

## Prerequisites

Before you start, follow the instructions in src/05/code/README.md to build llama.cpp for your platform.

You'll also need to download the Gemma 3 gguf used in the demo project. To do this, `cd` into the `code` folder and run the following commands:

```
mkdir -p gguf
cd gguf
curl -OL https://huggingface.co/unsloth/gemma-3-1b-it-GGUF/resolve/main/gemma-3-1b-it-Q4_K_M.gguf?download=true
```

## Building for Mac

### Copy libraries

`cd` into the `llama.cpp/build` folder and run the following command. This will copy the compiled libraries into the Unreal plug-in folder.

```
find . -name "*.a" -exec cp {} ../../unreal/LlamaDemo/Plugins/LlamaCppPlugin/Source/ThirdParty/llama.cpp/ \;
```

### Copy headers

`cd` into the `llama.cpp/include` folder and run the following command. This will copy the required headers into the Unreal plug-in folder.

```
find . -name "*.h" -exec cp {} ../../unreal/LlamaDemo/Plugins/LlamaCppPlugin/Source/ThirdParty/llama.cpp/include/ \;
```

Do the same for the ggml headers. `cd` into the `llama.cpp/ggml/include` folder and run the following command.

```
cp *.h ../../../unreal/LlamaDemo/Plugins/LlamaCppPlugin/Source/ThirdParty/llama.cpp/include/
```

### Build the plug-in

`cd` into the `LlamaDemo` folder and run the following command. This will build the plug-in. (It can take a few minutes, so be patient!)

```
/Users/Shared/Epic\ Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh LlamaDemoEditor Mac Development -project="$(pwd)/LlamaDemo.uproject"
```