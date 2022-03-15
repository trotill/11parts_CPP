##!!!SET VAR!!!
#COMPILER_ROOT=/embedded/docker/archlinux/cross/prebuild/x-tools7h/
#COMPILER_ROOT_BIN=arm-unknown-linux-gnueabihf/bin/arm-unknown-linux-gnueabihf-
#ORIGINAL_ROOTFS_PATH=/embedded/archlinux/armv7/target/rootfs
#DOCKER_IMAGE_NAME=archqemucross:latest

function dockerMakeCMake {
	dockerCMAKE='
	# this one is important\n
	SET(CMAKE_SYSTEM_NAME Linux)\n
	#this one not so much\n
	SET(CMAKE_SYSTEM_VERSION 1)\n
	# specify the cross compiler\n
	SET(CMAKE_C_COMPILER   /compiler/'$COMPILER_ROOT_BIN'gcc)\n
	SET(CMAKE_CXX_COMPILER /compiler/'$COMPILER_ROOT_BIN'g++)\n

	SET(CMAKE_FIND_ROOT_PATH  /rootfs)\n
	set(CMAKE_SYSROOT /rootfs)\n
	set(ENV{PKG_CONFIG_DIR} "")\n
	set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")\n
	set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})\n

	include_directories(${CMAKE_SYSROOT}/usr/local/include)\n

	# search for programs in the build host directories\n
	SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n
	# for libraries and headers in the target directories\n
	SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)\n
	SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)\n'
	echo -e $dockerCMAKE
	echo -e $dockerCMAKE > cmakeAddition.cmake
}

function dockerMakeBuildScript {
	dockerBUILD_SCRIPT='
		cd /build/build\n
		sudo su\n
		cmake -DCMAKE_TOOLCHAIN_FILE=../cmakeAddition.cmake ../\n
		make -j8\n
	'
	echo -e "$dockerBUILD_SCRIPT"
}

function dockerBuild {
	echo docker run -i --rm -v $(pwd):/build -v ${COMPILER_ROOT}:/compiler -v ${ORIGINAL_ROOTFS_PATH}:/rootfs $DOCKER_IMAGE_NAME bash
	echo -e "$dockerBUILD_SCRIPT"| docker run -i --rm -v $(pwd):/build -v ${COMPILER_ROOT}:/compiler -v ${ORIGINAL_ROOTFS_PATH}:/rootfs $DOCKER_IMAGE_NAME bash
}

function dockerClean {
	rm build/CMakeCache.txt build/cmake_install.cmake build/Cnoda build/libcnoda* build/Makefile build/necron build/ssdpd build/svc build/watchdog_stub 
	rm build/CMakeFiles -r
}

dockerMakeCMake ""
dockerMakeBuildScript ""
case "$1" in
	"clean")	 
	dockerClean ""
	;;
	"all")
	dockerBuild ""
	;;
	"make")
	dockerBuild ""
	;;
	"remake")
	dockerClean ""
	dockerBuild ""
	;;
esac
echo "Build finish"
#dockerBuild ""
#docker run -it --rm -v $(pwd):/build -v ${COMPILER_ROOT}:/compiler -v ${ORIGINAL_ROOTFS_PATH}:/rootfs --name demo $DOCKER_IMAGE_NAME bash
