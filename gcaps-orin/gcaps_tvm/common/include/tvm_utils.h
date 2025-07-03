#ifndef TVM_UTILS_H_
#define TVM_UTILS_H_

#include <fstream>
#include <vector>
#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

#include <tvm/runtime/module.h>
#include <tvm/runtime/registry.h>
#include <tvm/runtime/relax_vm/vm.h>
#include <tvm/runtime/relax_vm/executable.h>
#include <tvm/runtime/container/map.h>
#include <tvm/runtime/container/string.h>
#include <tvm/runtime/file_utils.h>
#include <tvm/runtime/device_api.h>
#include <tvm/runtime/container/shape_tuple.h>
#include <dmlc/memory_io.h>
#include <dlpack/dlpack.h>

cv::Mat loadImage(const std::string& image_path);

std::vector<float> preprocessImage(
    const cv::Mat& image, 
    int resize_size = 256, 
    int crop_size = 224,
    const std::vector<float>& mean = {0.485f, 0.456f, 0.406f},
    const std::vector<float>& std = {0.229f, 0.224f, 0.225f}
);

tvm::runtime::NDArray convertToNDArray(
    const std::vector<float>& tensor_data,
    const std::vector<int64_t>& shape,
    const DLDevice& device
);

std::vector<std::string> loadLabels(const std::string& file_path);

std::string getLabel(const tvm::runtime::NDArray& output, const std::vector<std::string>& labels);

std::string readFileToString(const std::string& filePath);

#endif