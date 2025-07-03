#pragma once
#include <vector>
#include <string>
#include <tvm/runtime/cpp_utils.h>
#include <tvm/runtime/cuda/cuda_common.h>

#include <support.h>
#include <linux/nvgpu.h>
#include <tvm_utils.h>
#include <task.h>

class ResNet18 {
public:
    ResNet18(Task task, int fd, bool ioctl_enabled);
    void run();
    void finish();

private:
    Task task;
    int fd;
    bool ioctl_enabled;
    int pid;

    CUdevice device;
    cudaEvent_t start;
    cudaEvent_t stop;
    cudaStream_t stream;

    DLDevice dev;
    std::shared_ptr<tvm::runtime::relax_vm::VirtualMachineImpl> vm;
    std::vector<tvm::runtime::NDArray> gpu_params;
    int segments_length;
};
