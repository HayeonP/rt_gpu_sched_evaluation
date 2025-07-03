cmake_minimum_required(VERSION 3.10)
project(gcaps_tvm CXX)

# 1. C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Necessary packages
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui imgcodecs)
find_package(CUDAToolkit REQUIRED)

# 3. TVM headers
set(TVM_BASE_DIR "/home/rubis/workspace/tvm-segment")
set(TVM_INCLUDE_DIRS
    ${TVM_BASE_DIR}/include
    ${TVM_BASE_DIR}/3rdparty/dmlc-core/include
    ${TVM_BASE_DIR}/3rdparty/dlpack/include
)

# 4. Libraries
set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
link_directories(
    ${TVM_BASE_DIR}/build  # TVM 라이브러리 경로
    ${CUDA_TOOLKIT_ROOT_DIR}/lib64
)

# 5. Executable
add_executable(gcaps_tvm
    main.cpp
    common/src/tvm_utils.cpp
    common/src/debug.cpp
    app/src/resnet18.cpp
)

# 6. Include directories
target_include_directories(gcaps_tvm PRIVATE
    ${TVM_INCLUDE_DIRS}
    include
    app/include
    common/include
)

# 7. Link dependencies
target_link_libraries(gcaps_tvm
    PRIVATE
    tvm_allvisible
    ${OpenCV_LIBS}
    cudart
    dl
    pthread
    curl
    cuda
)

# 8. Compile options
target_compile_options(gcaps_tvm
    PRIVATE
    -O3
    -Wall
    -I${CUDA_TOOLKIT_ROOT_DIR}/include
)

target_compile_options(gcaps_tvm PRIVATE)
target_compile_definitions(gcaps_tvm
    PRIVATE
    DMLC_LOG_CUSTOMIZE=1 
)
