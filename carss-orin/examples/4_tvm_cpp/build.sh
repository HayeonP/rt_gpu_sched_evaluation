g++ -std=c++17 \
    -o 1_resnet18_e2e 1_resnet18_e2e.cpp \
    -I/home/rubis/workspace/tvm-segment/include \
    -I/home/rubis/workspace/tvm-segment/3rdparty/dmlc-core/include \
    -I/home/rubis/workspace/tvm-segment/3rdparty/dlpack/include \
    -L/home/rubis/workspace/tvm-segment/build \
    -ltvm_allvisible \
    -lcurl \
    -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs \
    -I../../cuMiddleWare/include \
    -L../../cuMiddleWare/lib \
    -I/usr/include/opencv4 \
    -lcarss

g++ -std=c++17 \
    -o 2_resnet18_segmented 2_resnet18_segmented.cpp \
    -I/home/rubis/workspace/tvm-segment/include \
    -I/home/rubis/workspace/tvm-segment/3rdparty/dmlc-core/include \
    -I/home/rubis/workspace/tvm-segment/3rdparty/dlpack/include \
    -L/home/rubis/workspace/tvm-segment/build \
    -ltvm_allvisible \
    -lcurl \
    -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs \
    -I../../cuMiddleWare/include \
    -L../../cuMiddleWare/lib \
    -I/usr/include/opencv4 \
    -lcarss
