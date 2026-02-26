# physarum-visualizer

A Physarum Slime Mold simulation, that runs on the GPU using compute shaders. Can react to audio input similar to a music visualizer.

## Build

Use CMake to compile for Linux or crosscompile for Windows.

### Compilation for Linux

```bash 
cmake ..
```
### Compilation for Windows

```bash 
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake ..
```