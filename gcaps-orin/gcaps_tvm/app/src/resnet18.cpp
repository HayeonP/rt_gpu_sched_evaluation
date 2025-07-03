#include <resnet18.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <sys/ioctl.h>
#include <cuda.h>
#include <debug.h>

ResNet18::ResNet18(Task task, int fd, bool ioctl_enabled)
    : task(task), fd(fd), ioctl_enabled(ioctl_enabled) {
    pid = getpid();
        
    cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking);
    cudaEventCreateWithFlags(&start, cudaEventBlockingSync);
    cudaEventCreateWithFlags(&stop, cudaEventBlockingSync);    
    
    // If executing a gcaps macro before other task's stream initliazation, blocking can be occur
    // This sleep prevent this problem.
    std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(5000));

    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegBegin "<<now_str()<<std::endl;
    gcapsGpuSegBegin(fd, pid, false, ioctl_enabled);
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegBegin "<<now_str()<<std::endl;

    // Init tvm
    dev = DLDevice{kDLCUDA, 0};

    std::string library_path{"../model/resnet18/resnet18.so"};
    auto ex = tvm::runtime::LoadExecutableModule(library_path);
    vm = tvm::runtime::InitVirtualMachine(dev, ex);
 

    std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(3000));

    std::string binary_path{"../model/resnet18/resnet18.bin"};
    auto params = tvm::runtime::LoadParamsAsNDArrayList(binary_path);

    std::string segment_info_path{"../model/resnet18/segments_info"};
    std::string segments_info = readFileToString(segment_info_path);

    // Load segment runner
    segments_length = vm->SegmentRunnerLoad(segments_info);

    // Set parameters
    for (auto& param : params) gpu_params.push_back(param.CopyTo(dev));    

    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegEnd "<<now_str()<<std::endl;
    gcapsGpuSegEnd(fd, pid, false, stream, ioctl_enabled);
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegEnd "<<now_str()<<std::endl;
}

void ResNet18::run() {
    // Load image
    cv::Mat image = loadImage("../data/dog.jpg");
    std::vector<float> preprocessed_image = preprocessImage(image, 256, 224);
    std::vector<int64_t> shape = {1, 3, 224, 224};

    // Set input
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegBegin "<<now_str()<<std::endl;
    gcapsGpuSegBegin(fd, pid, false, ioctl_enabled);
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegBegin "<<now_str()<<std::endl;

    tvm::runtime::NDArray input = convertToNDArray(preprocessed_image, shape, dev);    
    tvm::runtime::NDArray gpu_input = input.CopyTo(dev);    
    vm->SegmentRunnerSetInput(gpu_input, gpu_params);

    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegEnd "<<now_str()<<std::endl;
    gcapsGpuSegEnd(fd, pid, false, stream, ioctl_enabled);
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegEnd "<<now_str()<<std::endl;

    // Segment runner
    for (int j = 0; j < segments_length; j++) {
        // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegBegin "<<now_str()<<std::endl;
        gcapsGpuSegBegin(fd, pid, false, ioctl_enabled);
        // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegBegin "<<now_str()<<std::endl;
        
        vm->SegmentRunnerRun(j);
        
        // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegEnd "<<now_str()<<std::endl;
        gcapsGpuSegEnd(fd, pid, false, stream, ioctl_enabled);
        // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegEnd "<<now_str()<<std::endl;
    }
    
    
    // Get output
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegBegin "<<now_str()<<std::endl;
    gcapsGpuSegBegin(fd, pid, false, ioctl_enabled);
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegBegin "<<now_str()<<std::endl;
    
    std::vector<tvm::runtime::NDArray> gpu_output = vm->SegmentRunnerGetOutput();
    tvm::runtime::NDArray output = gpu_output[0].CopyTo(DLDevice{kDLCPU, 0});
    
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] Before gcapsGpuSegEnd "<<now_str()<<std::endl;
    gcapsGpuSegEnd(fd, pid, false, stream, ioctl_enabled);
    // print_tab(task.id); std::cout<<"[task:"<<task.id<<"] After gcapsGpuSegEnd "<<now_str()<<std::endl;
}

void ResNet18::finish(){
    std::cout<<"[FINISH] task id: "<<task.id<<std::endl;
    cudaStreamSynchronize(stream);
    cudaStreamDestroy(stream);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    // cuCtxDetach(ctx);                               
    

    return;
}