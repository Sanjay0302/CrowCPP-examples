# aarch64-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Optional: set the sysroot if needed
# set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)

# Optional: set search paths for libraries and includes
# set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)