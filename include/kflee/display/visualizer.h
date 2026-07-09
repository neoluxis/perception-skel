#pragma once

#include "kflee/typings/types.h"

#include <string>
#include <vector>

// 前向声明
namespace cv {
class Mat;
}

namespace kflee::display {

/**
 * 可视化器配置
 */
struct VisualizerConfig {
    /** 检测框线条颜色 (B, G, R)，默认绿色 */
    int box_color_b = 0;
    int box_color_g = 255;
    int box_color_r = 0;
    /** 检测框线条粗细（像素） */
    int box_thickness = 2;
    /** 标签字体大小 */
    double font_scale = 0.5;
    /** 标签字体粗细 */
    int font_thickness = 1;
    /** 标签背景颜色 (B, G, R)，默认深灰 */
    int label_bg_b = 40;
    int label_bg_g = 40;
    int label_bg_r = 40;
    /** 标签文字颜色 (B, G, R)，默认白色 */
    int label_text_b = 255;
    int label_text_g = 255;
    int label_text_r = 255;
};

/**
 * 检测结果可视化器
 *
 * 在图像上绘制检测框和标签，并支持保存到文件。
 *
 * 使用方式：
 * @code
 *   Visualizer viz(config);
 *   cv::Mat result = viz.draw(image, detections);
 *   viz.save(result, "output.jpg");
 * @endcode
 */
class Visualizer {
public:
    /**
     * 使用默认配置构造
     */
    Visualizer();

    /**
     * 使用自定义配置构造
     */
    explicit Visualizer(const VisualizerConfig& config);

    /**
     * 在图像上绘制所有检测框和标签
     *
     * @param image 输入图像（BGR 格式），不会修改原图
     * @param detections 检测结果列表
     * @return 绘制后的新图像（BGR 格式）
     */
    cv::Mat draw(const cv::Mat& image,
                 const std::vector<kflee::data::Detection>& detections) const;

    /**
     * 将图像保存到文件
     *
     * @param image 待保存的图像
     * @param path 输出文件路径（.jpg / .png / .bmp 等）
     * @throws std::runtime_error 如果保存失败
     */
    static void save(const cv::Mat& image, const std::string& path);

    /**
     * 获取当前配置（只读）
     */
    [[nodiscard]] const VisualizerConfig& config() const;

private:
    VisualizerConfig config_;
};

}  // namespace kflee::display
