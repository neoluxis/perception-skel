#pragma once

#include "kflee/typings/slice_types.h"

#include <opencv2/core.hpp>

#include <vector>

namespace kflee::cropper {

/**
 * SAHI 切片裁剪器配置
 *
 * 固定参数：原图 2048x2048，切片 640x640，3x3 网格居中排列。
 */
struct SahiCropperConfig {
    /** 原图宽度（像素） */
    int image_width = 2048;
    /** 原图高度（像素） */
    int image_height = 2048;
    /** 切片宽度（模型输入尺寸） */
    int slice_width = 640;
    /** 切片高度（模型输入尺寸） */
    int slice_height = 640;
    /** 网格列数 */
    int grid_cols = 3;
    /** 网格行数 */
    int grid_rows = 3;
};

/**
 * 切片裁剪结果
 *
 * 包含裁剪出的图像切片及其在原图中的位置信息。
 */
struct SliceResult {
    /** 裁剪出的图像切片（OpenCV Mat，BGR 格式） */
    cv::Mat image;
    /** 切片在原图中的位置元信息 */
    kflee::data::SliceInfo info;
};

/**
 * SAHI 切片裁剪器
 *
 * 将大尺寸图像从中心开始按固定网格切割为多个小切片。
 * 对于 2048x2048 原图，3x3 网格，切片 640x640：
 *   - 网格总尺寸 = 3 * 640 = 1920
 *   - 起始偏移 = (2048 - 1920) / 2 = 64
 *   - 边缘 64 像素被丢弃，不参与推理
 *
 * 坐标全部硬编码，不做动态计算，确保板端运行效率。
 */
class SahiCropper {
public:
    /**
     * 使用默认配置构造（2048x2048，640x640，3x3）
     */
    SahiCropper();

    /**
     * 使用自定义配置构造
     *
     * @param config 切片裁剪配置
     */
    explicit SahiCropper(const SahiCropperConfig& config);

    /**
     * 对图像执行切片裁剪
     *
     * @param image 输入图像（OpenCV Mat），尺寸必须等于配置中的 image_width x image_height
     * @return 所有切片的列表，共 grid_rows * grid_cols 个
     * @throws std::invalid_argument 如果图像尺寸不匹配或图像为空
     */
    std::vector<SliceResult> slice(const cv::Mat& image) const;

    /**
     * 将切片内的检测框坐标映射回原图坐标
     *
     * @param x 切片内的 x 坐标
     * @param y 切片内的 y 坐标
     * @param slice_info 切片元信息
     * @return 原图中的 (x, y) 坐标对
     */
    static std::pair<float, float> map_to_original(
        float x, float y, const kflee::data::SliceInfo& slice_info);

    /**
     * 获取切片数量
     */
    [[nodiscard]] int slice_count() const;

    /**
     * 获取当前配置（只读）
     */
    [[nodiscard]] const SahiCropperConfig& config() const;

private:
    SahiCropperConfig config_;

    /**
     * 预计算所有切片的 SliceInfo 列表
     *
     * 在构造时一次性计算，后续切片时直接复用。
     */
    std::vector<kflee::data::SliceInfo> slice_infos_;

    /** 计算切片起始位置网格 */
    void compute_slice_infos();
};

}  // namespace kflee::cropper
