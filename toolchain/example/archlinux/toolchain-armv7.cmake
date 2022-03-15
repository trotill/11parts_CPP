# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /embedded/docker/archlinux/cross/prebuild/x-tools7h/arm-unknown-linux-gnueabihf/bin/arm-unknown-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /embedded/docker/archlinux/cross/prebuild/x-tools7h/arm-unknown-linux-gnueabihf/bin/arm-unknown-linux-gnueabihf-g++)
#SET(CMAKE_LIBRARY_PATH /embedded/tmp/mnt/usr/lib)
#SET(CMAKE_PREFIX_PATH /embedded/tmp/mnt/usr)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH  /embedded/tmp/mnt)
set(CMAKE_SYSROOT "/embedded/tmp/mnt")
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})

include_directories(${CMAKE_SYSROOT}/usr/local/include)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


