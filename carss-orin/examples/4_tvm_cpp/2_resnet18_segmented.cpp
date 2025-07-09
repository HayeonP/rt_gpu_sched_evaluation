#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <iomanip>
#include <tag_gpu.h>
#include <sstream>
#include "tvm_utils.h"

// 타임스탬프 문자열 반환 함수 (날짜 없이)
std::string now_time_only() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_time_t = system_clock::to_time_t(now);
    auto now_us = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;

    std::tm tm_now;
    localtime_r(&now_time_t, &tm_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%H:%M:%S")
        << "." << std::setfill('0') << std::setw(6) << now_us.count();
    return oss.str();
}

void dummy_job(int idx) {
    for (int i = 0; i < 350000000; ++i) {
        int a = 1;
        a = a + 1;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "[" << now_time_only() << "] Start" << std::endl;

    if (argc != 2) {
        std::cout << "[" << now_time_only() << "] Usage: " << argv[0] << " <period_ms>" << std::endl;
        return 1;
    }

    double period_ms = std::atof(argv[1]);
    if (period_ms <= 0) {
        std::cout << "[" << now_time_only() << "] Period must be positive." << std::endl;
        return 1;
    }

    /* Init TVM ***************************************************************/
        std::vector<std::string> image_path_list;
    image_path_list.push_back("dog.jpg");
    image_path_list.push_back("Gatto_europeo4.jpg");
    image_path_list.push_back("960px-African_Bush_Elephant.jpg");

    // dev = tvm.device("cuda", 0)
    DLDevice dev{kDLCUDA, 0}; // OK
    
    // ex = tvm.runtime.load_module("resnet18.so")   
    std::string library_apth{"resnet18.so"};
    auto ex = tvm::runtime::LoadExecutableModule(library_apth);
    
    // params = load_params("resnet18.bin")    
    std::string binary_path{"resnet18.bin"};
    std::vector<tvm::runtime::NDArray> params = tvm::runtime::LoadParamsAsNDArrayList(binary_path); 

    // vm = relax.VirtualMachine
    auto vm = tvm::runtime::InitVirtualMachine(dev, ex);

    // segments_length = segment_runner.load(segments_info)
    std::string segments_info = readFileToString("segments_info");
    int segments_length = vm->SegmentRunnerLoad(segments_info);

    // gpu_params = [tvm.nd.array(p, dev) for p in params["main"]]
    std::vector<tvm::runtime::NDArray> gpu_params;
    for(auto& param : params){
        gpu_params.push_back(param.CopyTo(dev));
    }

    //  labels = get_imagenet_labels()
    std::vector<std::string> labels = loadLabels("imagenet_classes.txt");
    /**************************************************************************/

    pid_t pid = getpid();
    pid_t tid = gettid();
    std::string taskname = "periodic_task_" + std::to_string(pid);
    int64_t period_us = std::stoll(argv[1]) * 1000;
    int64_t deadline_us = std::stoll(argv[1]) * 1000;
    int64_t slacktime = 0;
    bool first_flag = false;
    bool shareable_flag = false;
    uint64_t required_mem = 0;

    int i = 0;
    while (true) {
        std::string release_ts = now_time_only();
        auto start_time = std::chrono::high_resolution_clock::now();
        std::cout<<deadline_us << std::endl;

        
        // orig_image, image_tensor = preprocess_image(image_path)
        cv::Mat image = loadImage(image_path_list[i%3]);
        std::vector<float> preprocessed_image = preprocessImage(image, 256, 224);
        
        std::vector<int64_t> shape = {1, 3, 224, 224}; // NCHW
        
        // gpu_input = tvm.nd.array(image_tensor.astype("float32"), dev)
        tvm::runtime::NDArray input = tvm::runtime::convertVecToNDArray(preprocessed_image, shape);
        tvm::runtime::NDArray gpu_input = input.CopyTo(dev);

        // segment_runner.set_input(gpu_input, *gpu_params)
        vm->SegmentRunnerSetInput(gpu_input, gpu_params);

        // for i in range(segments_length):
            // print("Run Segment", i)
            // segment_runner.run(i)
        for(int i = 0; i < segments_length; i++){
            std::cout<<"Segment "<<i<<std::endl;
            tag_job_begin(pid, tid, taskname.c_str(), period_us, deadline_us, slacktime, first_flag, shareable_flag, 0);
            vm->SegmentRunnerRun(i);
            tag_job_end(pid, tid, taskname.c_str());
        }
        
        // output = segment_runner.get_output()
        std::vector<tvm::runtime::NDArray> gpu_output = vm->SegmentRunnerGetOutput();
        tvm::runtime::NDArray output = gpu_output[0].CopyTo(DLDevice{kDLCPU, 0});
        std::cout << getLabel(output, labels) << std::endl;


        auto end_time = std::chrono::high_resolution_clock::now();
        std::string finish_ts = now_time_only();
        std::chrono::duration<double> response_time = end_time - start_time;

        // 한 줄에 release, finish, response time 출력
        std::cout << "[release: " << release_ts << "] "
                  << "[finish: " << finish_ts << "] "
                  << "[response: " << std::fixed << std::setprecision(6) << response_time.count() << " sec]"
                  << std::endl;

        double sleep_time = (period_ms / 1000.0) - response_time.count();
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
        }
        ++i;
    }

    return 0;
}
