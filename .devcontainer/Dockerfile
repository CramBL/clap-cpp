# focal (20.04), jammy (22.04), ..
## NOTE: dont specify the date alias, it will break later in the build.
ARG VARIANT="jammy"

# First stage: build base image, just enough to build clap_cpp and run tests
FROM ubuntu:${VARIANT} as build-base

# Restate the variant to use it later on in the llvm and cmake installations
ARG VARIANT

ARG HOST_USER
ARG HOST_UID
ARG HOST_GID

# Ignore "WARNING: Running pip as the 'root' user can result in broken permissions..."
ENV PIP_ROOT_USER_ACTION=ignore

RUN groupadd --gid ${HOST_GID} ${HOST_USER} \
    && useradd --uid ${HOST_UID} --gid ${HOST_GID} -m ${HOST_USER}

# Install necessary packages available from standard repos
RUN touch /in_container \
    && apt-get update -qq \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    # software-properties-common contains add-apt-repository
    software-properties-common \
    wget \
    curl \
    apt-utils \
    file \
    zip \
    openssh-client \
    gpg-agent \
    socat \
    rsync \
    make \
    ninja-build \
    git \
    python3 \
    python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# User-settable versions
ARG GCC_VER="13"
# Add gcc-${GCC_VER}
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get update -qq \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    gcc-${GCC_VER} \
    g++-${GCC_VER} \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set gcc-${GCC_VER} as default gcc
RUN update-alternatives --install /usr/bin/gcc gcc $(which gcc-${GCC_VER}) 100
RUN update-alternatives --install /usr/bin/g++ g++ $(which g++-${GCC_VER}) 100
RUN update-alternatives --install /usr/bin/gcov gcov $(which gcov-${GCC_VER}) 100


# Installation instructions on LLVM: https://apt.llvm.org/
ARG LLVM_VER="17"
# Add clang-${LLVM_VER}
ARG LLVM_URL="http://apt.llvm.org/${VARIANT}/"
ARG LLVM_PKG="llvm-toolchain-${VARIANT}-${LLVM_VER}"

RUN wget --progress=dot:giga -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - 2>/dev/null \
    && add-apt-repository -y "deb ${LLVM_URL} ${LLVM_PKG} main" \
    && apt-get update -qq \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    # LLVM
    libllvm${LLVM_VER} \
    llvm-${LLVM_VER} \
    llvm-${LLVM_VER}-dev \
    # Clang
    clang-${LLVM_VER} \
    clang-tools-${LLVM_VER} \
    libclang-common-${LLVM_VER}-dev \
    libclang-${LLVM_VER}-dev \
    libclang1-${LLVM_VER} \
    clangd-${LLVM_VER} \
    clang-tidy-${LLVM_VER} \
    # Other tools
    lldb-${LLVM_VER} \
    lld-${LLVM_VER} \
    # Compiler runtime contains the sanitizers
    libclang-rt-${LLVM_VER}-dev \
    # libc++
    libc++-${LLVM_VER}-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set the default clang-tidy, so CMake can find it
RUN update-alternatives --install /usr/bin/clang-tidy clang-tidy $(which clang-tidy-${LLVM_VER}) 1

# Set clang-${LLVM_VER} as default clang
RUN update-alternatives --install /usr/bin/clang clang $(which clang-${LLVM_VER}) 100
RUN update-alternatives --install /usr/bin/clang++ clang++ $(which clang++-${LLVM_VER}) 100
# Set ld.lld to point to a matching LLVM version
RUN update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-${LLVM_VER} 100
# Set llvm-cov to point to a matching LLVM version
RUN update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-${LLVM_VER} 100

# Add current cmake/ccmake, from Kitware
ARG CMAKE_URL="https://apt.kitware.com/ubuntu/"
ARG CMAKE_PKG=${VARIANT}
RUN wget --progress=dot:giga -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
    | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
    && apt-add-repository -y "deb ${CMAKE_URL} ${CMAKE_PKG} main" \
    && apt-get update -qq \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install optional dependecies
RUN apt-get update -qq && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    ccache \
    cppcheck \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install mold for much faster linking
RUN apt-get update \
    && apt-get install -y \
    mold \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
ENV PATH=$PATH:/usr/bin/mold
RUN mold --version

# Install Just
ARG JUST_VER 1.25.2
RUN curl --proto '=https' --tlsv1.2 -sSf https://just.systems/install.sh \
    | bash -s -- --tag=${JUST_VER} --to /usr/bin  \
    && just --version

# Allow the user to set compiler defaults
ARG USE_CLANG
# if --build-arg USE_CLANG=1, set CC to 'clang' or set to null otherwise.
ENV CC=${USE_CLANG:+"clang"}
ENV CXX=${USE_CLANG:+"clang++"}
# if CC is null, set it to 'gcc' (or leave as is otherwise).
ENV CC=${CC:-"gcc"}
ENV CXX=${CXX:-"g++"}

# Install gcovr
RUN python3 -m pip install --no-cache-dir --upgrade pip setuptools \
    && python3 -m pip install --no-cache-dir gcovr \
    && gcovr --version

# Install editors
RUN apt-get update -qq \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    neovim \
    emacs \
    nano \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

# Setup completions
RUN apt-get update -qq \
    && apt-get install -y bash-completion \
    && echo "source /etc/profile.d/bash_completion.sh" >> ~/.bashrc \
    && just --completions bash >> /etc/profile.d/just_completions.sh

FROM build-base as extras


# Install include-what-you-use
ENV IWYU /home/iwyu
ENV IWYU_BUILD ${IWYU}/build
ENV IWYU_SRC ${IWYU}/include-what-you-use
RUN mkdir -p ${IWYU_BUILD} \
    && git clone --branch clang_${LLVM_VER} \
    https://github.com/include-what-you-use/include-what-you-use.git \
    ${IWYU_SRC}
RUN CC=clang-${LLVM_VER} \
    CXX=clang++-${LLVM_VER} \
    cmake -S ${IWYU_SRC} \
    -B ${IWYU_BUILD} \
    -G "Unix Makefiles" \
    -DCMAKE_PREFIX_PATH=/usr/lib/llvm-${LLVM_VER} \
    && cmake --build ${IWYU_BUILD} -j \
    && cmake --install ${IWYU_BUILD}

# Per https://github.com/include-what-you-use/include-what-you-use#how-to-install:
# `You need to copy the Clang include directory to the expected location before
#  running (similarly, use include-what-you-use -print-resource-dir to learn
#  exactly where IWYU wants the headers).`
RUN mkdir -p $(include-what-you-use -print-resource-dir 2>/dev/null)
RUN ln -s $(readlink -f /usr/lib/clang/${LLVM_VER}/include) \
    $(include-what-you-use -print-resource-dir 2>/dev/null)/include


# Install optional dependecies
RUN apt-get update -qq && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    doxygen \
    graphviz \
    clang-format-${LLVM_VER} \
    gdb \
    cmake-curses-gui \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install conan
RUN python3 -m pip install --no-cache-dir --upgrade pip setuptools \
    && python3 -m pip install --no-cache-dir conan \
    && conan --version

# By default, anything you run in Docker is done as superuser.
# Conan runs some install commands as superuser, and will prepend `sudo` to
# these commands, unless `CONAN_SYSREQUIRES_SUDO=0` is in your env variables.
ENV CONAN_SYSREQUIRES_SUDO 0
# Some packages request that Conan use the system package manager to install
# a few dependencies. This flag allows Conan to proceed with these installations;
# leaving this flag undefined can cause some installation failures.
ENV CONAN_SYSREQUIRES_MODE enabled


CMD ["/bin/bash"]
