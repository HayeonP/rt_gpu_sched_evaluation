#include "./tvm_utils.h"

std::string readFileToString(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filePath);
    }
    // 파일 전체를 string으로 읽기
    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    file.close();
    return content;
}

cv::Mat loadImage(const std::string& image_path) {
    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error("Failed to load image: " + image_path);
    }
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    return image;
}

std::vector<float> preprocessImage(
    const cv::Mat& image, int resize_size, int crop_size, const std::vector<float>& mean, const std::vector<float>& std) {
    cv::Mat processed = image.clone();

    // 1. 리사이즈
    double aspect_ratio = static_cast<double>(processed.cols) / processed.rows;
    int new_width, new_height;
    if (processed.cols > processed.rows) {
        new_height = resize_size;
        new_width = static_cast<int>(resize_size * aspect_ratio);
    } else {
        new_width = resize_size;
        new_height = static_cast<int>(resize_size / aspect_ratio);
    }
    cv::resize(processed, processed, cv::Size(new_width, new_height));

    // 2. 크롭
    int x = (processed.cols - crop_size) / 2;
    int y = (processed.rows - crop_size) / 2;
    if (x < 0 || y < 0) {
        throw std::runtime_error("Crop size larger than resized image");
    }
    cv::Rect roi(x, y, crop_size, crop_size);
    processed = processed(roi).clone();

    // 3. float 변환 및 정규화, HWC → CHW 변환
    const int channels = 3;
    const int height = processed.rows;
    const int width = processed.cols;
    if (processed.channels() != channels) {
        throw std::runtime_error("Input image must have 3 channels");
    }

    cv::Mat float_image;
    processed.convertTo(float_image, CV_32FC3, 1.0 / 255.0);

    std::vector<float> tensor_data(1 * channels * height * width);
    const int hw_size = height * width;
    for (int c = 0; c < channels; ++c) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                float value = float_image.at<cv::Vec3f>(h, w)[c];
                tensor_data[c * hw_size + h * width + w] = (value - mean[c]) / std[c];
            }
        }
    }
    return tensor_data;
}

tvm::runtime::NDArray convertToNDArray(
    const std::vector<float>& tensor_data,
    const std::vector<int64_t>& shape,
    const DLDevice& device
) {
    // NDArray 생성 및 데이터 복사
    tvm::runtime::NDArray ndarray = tvm::runtime::NDArray::Empty(
        tvm::runtime::ShapeTuple(shape),
        DLDataType{kDLFloat, 32, 1},
        device
    );
    size_t num_bytes = tensor_data.size() * sizeof(float);
    ndarray.CopyFromBytes(tensor_data.data(), num_bytes);
    return ndarray;
}

std::vector<std::string> loadLabels(const std::string& file_path) {
    std::vector<std::string> labels;
    std::ifstream file(file_path);
    std::string line;
    
    while (std::getline(file, line)) {
        labels.push_back(line);
    }
    return labels;
}

std::string getLabel(const tvm::runtime::NDArray& output, const std::vector<std::string>& labels) {
    // Extract data
    const DLTensor* tensor = output.operator->();
    const float* data = static_cast<float*>(tensor->data);
    int64_t size = 1;
    for (int i = 0; i < tensor->ndim; ++i) {
        size *= tensor->shape[i];
    }

    // argmax
    int max_index = 0;
    float max_value = data[0];
    for (int64_t i = 1; i < size; ++i) {
        if (data[i] > max_value) {
            max_value = data[i];
            max_index = i;
        }
    }
    
    if (max_index >= 0 && max_index < labels.size()) {
        return labels[max_index];
    }

    return "Unknown";
}