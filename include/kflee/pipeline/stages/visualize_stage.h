#pragma once

#include "kflee/display/visualizer.h"
#include "kflee/pipeline/stage.h"

#include <string>

namespace kflee::pipeline {

/**
 * 可视化阶段
 *
 * 在原图上绘制检测框和标签。
 *
 * 读取 frame.image 和 frame.detections，
 * 写入 frame.output_image。
 */
class VisualizeStage : public Stage {
public:
    /**
     * @param visualizer 可视化器引用
     */
    explicit VisualizeStage(kflee::display::Visualizer& visualizer);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::display::Visualizer& visualizer_;
};

}  // namespace kflee::pipeline
