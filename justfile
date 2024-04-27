PWD := `pwd`
USE_CLANG := env('USE_CLANG', '1')
GCOV_EXECUTABLE := if USE_CLANG == "1" { "llvm-cov gcov" } else { "gcov" }
CC  := if USE_CLANG == "1" { "clang"   } else { "gcc" }
CXX := if USE_CLANG == "1" { "clang++" } else { "g++" }
DEVCONTAINER_NAME := "clap-devcontainer"
CMD := if path_exists('/in_container') == "true" {
"eval"
} else {
"docker run \
-u ${USER}" \
+ " -e CC=" + CC \
+ " -e CXX=" + CXX \
+ " -v " + PWD + ":/work" \
+ " --rm" \
+ " -t " + DEVCONTAINER_NAME \
+ " /bin/bash -lc "
}

[private]
@_default:
    just --list

# Entry point to ensure commands are run within the container
cmd *ARGS:
    {{CMD}} '{{ ARGS }}'

[no-exit-message, linux]
configure-project: \
    (cmd 'cmake \
        -S . \
        -B build \
        -G "Ninja Multi-Config" \
        -Dclap_cpp_ENABLE_IPO=ON \
        -Dclap_cpp_PACKAGING_MAINTAINER_MODE:BOOL=OFF \
        -DBUILD_TESTING=ON \
        -Dclap_cpp_ENABLE_COVERAGE:BOOL=ON \
        -Dclap_cpp_WARNINGS_AS_ERRORS:BOOL=ON \
        -Dclap_cpp_ENABLE_CLANG_TIDY:BOOL=OFF \
        -Dclap_cpp_ENABLE_CPPCHECK:BOOL=OFF \
        -DUSER_LINKER_OPTION=mold \
        -Dclap_cpp_ENABLE_USER_LINKER:BOOL=ON')

[no-exit-message]
build BUILD_MODE="Debug":
	just cmd cmake --build build --config $( just map-build-mode-str {{BUILD_MODE}} )

[no-exit-message]
test BUILD_MODE="Debug" TEST_ARGS="":
	just cmd "./build/test/$(just map-build-mode-str {{BUILD_MODE}})/tests {{TEST_ARGS}}"

[no-exit-message]
ctest-coverage BUILD_MODE="Debug":
	just cmd "\
		cd build \
			&& ctest -C $( just map-build-mode-str {{ BUILD_MODE }} ) \
			&& gcovr -j 2 \
				--delete \
				--root ../ \
				--print-summary \
				--xml-pretty \
				--xml coverage.xml . \
				--gcov-executable \"{{ GCOV_EXECUTABLE }}\" \
				--filter ../include "

[no-exit-message]
build-devcontainer UBUNTU_VARIANT="jammy":
	docker build \
		-t {{ DEVCONTAINER_NAME }} \
		--build-arg USE_CLANG={{ USE_CLANG }} \
		--build-arg VARIANT={{ UBUNTU_VARIANT }} \
		--build-arg HOST_USER=${USER} \
		--build-arg HOST_GID=$( id -g ) \
		--build-arg HOST_UID=$( id -u ) \
		-f .devcontainer/Dockerfile .

[no-exit-message]
run-devcontainer :
	docker run \
		-u ${USER} \
		-e CC=gcc \
		-e CXX=g++ \
		-v {{ PWD }}:/work \
		-it clap-devcontainer \
		/bin/bash -l

[no-exit-message]
exec-example ARGS BUILD_MODE="Debug":
    ./build/src/example/{{BUILD_MODE}}/example {{ARGS}}

[no-exit-message]
@run-in-container *ARGS:
	docker run \
		-v {{ PWD }}:/work \
		--rm \
		-it clap-devcontainer \
		/bin/bash -lc "{{ARGS}}"

[no-exit-message]
clean:
	cd build \
		&& find . \
			-maxdepth 1 \
			! -name "_deps" \
			-a \
			! -name "." \
			-exec \
				rm -rf '{}' \
			\;

[no-exit-message, confirm("\
Are you sure you want to delete the whole build directory?\n\
Dependencies will have to be redownloaded.\n\n\
Hint: You might have meant to use the `clean` recipe which keeps downloads.")]
clean-all:
    rm -rf build

[private, no-exit-message]
map-build-mode-str BUILD_MODE:
	#!/usr/bin/env bash
	set -eu
	lower_case_build_mode="{{ lowercase(BUILD_MODE) }}"
	case "${lower_case_build_mode}" in
		debug)
			echo Debug
			;;
		release)
			echo "Release"
			;;
		minsizerel)
			echo MinSizeRel
			;;
		relwithdebinfo)
			echo RelWithDebInfo
			;;
		*)
			echo "INVALID BUILD MODE: ${lower_case_build_mode}"
			echo "Valid build modes are: "
			echo -e "\t* Debug"
			echo -e "\t* Release"
			echo -e "\t* MinSizeRel"
			echo -e "\t* RelWithDebInfo"
			exit 1
	esac
