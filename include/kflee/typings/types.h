#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace kflee::data {

/**
 * 轻量 NCHW float tensor 视图
 *
 * 平台无关的 tensor 抽象，不同推理引擎只需将自己的输出 buffer
 * 适配为此结构即可与后处理模块对接。
 */
struct TensorView {
    const float* data = nullptr;
    int n = 0;
    int c = 0;
    int h = 0;
    int w = 0;

    /**
     * 计算 tensor 中的元素总数
     */
    [[nodiscard]] std::size_t size() const {
        return static_cast<std::size_t>(n) * c * h * w;
    }

    /**
     * 校验 tensor 视图是否有效（数据指针非空且各维度均为正数）
     */
    [[nodiscard]] bool valid() const {
        return data != nullptr && n > 0 && c > 0 && h > 0 && w > 0;
    }
};

/**
 * letterbox 预处理元信息
 *
 * 记录缩放比例、填充量以及原图和输入图尺寸，供后处理阶段
 * 将检测框坐标从输入空间映射回原图空间。
 */
struct LetterboxInfo {
    float scale = 1.0F;
    int pad_x = 0;
    int pad_y = 0;
    int image_w = 0;
    int image_h = 0;
    int input_w = 640;
    int input_h = 640;
};

/**
 * 水平检测框（原图坐标系下的 xyxy 格式）
 */
struct HBox {
    float x1 = 0.0F;
    float y1 = 0.0F;
    float x2 = 0.0F;
    float y2 = 0.0F;
};

/**
 * 通用检测结果
 *
 * 表示一个目标检测框，包含类别 ID、置信度分数、边界框坐标和跟踪 ID。
 */
struct Detection {
    int class_id = -1;
    float score = 0.0F;
    HBox box;
    /** 跟踪 ID，-1 表示未关联到任何轨迹 */
    int track_id = -1;
};

}  // namespace kflee::data
