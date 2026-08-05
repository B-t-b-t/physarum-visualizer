
# physarum-visualizer

A Physarum Slime Mold simulation, that runs on the GPU using compute shaders. Can react to audio input similar to a music visualizer.
<img width="3291" height="1395" alt="Screenshot_20260805_145349" src="https://github.com/user-attachments/assets/51a071a9-410e-4e98-9f55-aee95f66f52b" />

<table>
  <tr>
    <td><img width="2370" height="1395" alt="Screenshot_20260805_145645" src="https://github.com/user-attachments/assets/a9eb9087-1ee9-409d-ad5e-747b62a3acee" /></td>
    <td><img width="2370" height="1395" alt="Screenshot_20260805_150337" src="https://github.com/user-attachments/assets/ac63e6fc-1e11-420c-b1f4-6cf6e00b0b62" /></td>
    <td><img width="2370" height="1395" alt="Screenshot_20260805_151752" src="https://github.com/user-attachments/assets/d1116d68-d128-4d2b-8d67-9307ef7ded18" /></td>
  </tr>
</table>

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
