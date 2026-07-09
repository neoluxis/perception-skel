#pragma once

#include "kflee/cropper/sahi_cropper.h"
#include "kflee/pipeline/stage.h"

#include <string>

namespace kflee::pipeline {

/**
 * 图像切片阶段
 *
 * 将大尺寸原图按 SAHI 3x3 网格切割为 9 个 640x640 切片。
 * 读取 frame.image，写入 frame.slices。
 */
class CropStage : public Stage {
public:
    /**
     * @param cropper SAHI 切片裁剪器引用（生命周期需长于本 Stage）
     */
    explicit CropStage(kflee::cropper::SahiCropper& cropper);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::cropper::SahiCropper& cropper_;
};

}  // namespace kflee::pipeline
