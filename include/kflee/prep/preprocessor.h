#pragma once

#include "kflee/data/types.h"

#include <cstdint>
#include <vector>

// 前向声明 OpenCV 类型，避免在公共头文件中引入 OpenCV
// 使用者需要自行包含 opencv2/core.hpp
namespace cv {
class Mat;
}

namespace kflee::prep {

/**
 * Tensor 内存布局枚举
 */
enum class TensorLayout {
    NCHW,  // [batch, channel, height, width]
    NHWC,  // [batch, height, width, channel]
};

/**
 * 预处理配置
 */
struct PreprocessConfig {
    /** 模型输入宽度 */
    int input_width = 640;
    /** 模型输入高度 */
    int input_height = 640;
    /** 模型输入通道数（1=灰度, 3=RGB） */
    int input_channels = 3;
    /** 归一化到 [0,1] 还是保持 [0,255] */
    bool normalize = true;
    /** 输出 tensor 的内存布局 */
    TensorLayout layout = TensorLayout::NCHW;
};

/**
 * 预处理结果
 *
 * 包含预处理后的 tensor 数据以及 letterbox 元信息，
 * 供推理和后处理阶段使用。
 */
struct PreprocessResult {
    /** 预处理后的浮点 tensor 数据 */
    std::vector<float> tensor;
    /** letterbox 元信息，用于后处理时坐标逆映射 */
    kflee::data::LetterboxInfo letterbox;
};

/**
 * 图像预处理器
 *
 * 将 OpenCV 图像转换为模型输入 tensor，流程：
 * 1. BGR -> RGB 色彩空间转换
 * 2. 等比例缩放 + letterbox 填充
 * 3. 归一化到 [0, 1]
 * 4. 按指定布局（NCHW/NHWC）输出
 */
class Preprocessor {
public:
    /**
     * 构造预处理器
     *
     * @param config 预处理配置参数
     */
    explicit Preprocessor(const PreprocessConfig& config);

    /**
     * 对单张图像执行预处理
     *
     * @param bgr_image OpenCV 读取的 BGR 格式图像（CV_8UC3）
     * @return 预处理后的 tensor 数据和 letterbox 元信息
     * @throws std::invalid_argument 如果输入图像为空
     */
    PreprocessResult process(const cv::Mat& bgr_image) const;

    /**
     * 获取当前预处理配置（只读）
     */
    [[nodiscard]] const PreprocessConfig& config() const;

private:
    PreprocessConfig config_;

    /**
     * 将预处理后的图像按 NCHW 布局写入输出 buffer
     */
    void write_nchw(const cv::Mat& padded, std::vector<float>& output) const;

    /**
     * 将预处理后的图像按 NHWC 布局写入输出 buffer
     */
    void write_nhwc(const cv::Mat& padded, std::vector<float>& output) const;
};

}  // namespace kflee::prep
