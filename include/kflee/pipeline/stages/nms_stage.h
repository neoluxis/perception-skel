#pragma once

#include "kflee/perception/yolov8_postproc.h"
#include "kflee/pipeline/stage.h"

#include <string>

namespace kflee::pipeline {

/**
 * 全局 NMS 阶段
 *
 * 对所有切片的检测结果执行跨切片非极大值抑制，
 * 消除相邻切片重叠区域产生的重复检测框。
 *
 * 读取 frame.detections，原地替换为 NMS 后的结果。
 */
class NmsStage : public Stage {
public:
    /**
     * @param postprocessor YOLOv8 后处理器引用（使用其 nms() 方法）
     */
    explicit NmsStage(kflee::perception::YoloV8Postprocessor& postprocessor);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::perception::YoloV8Postprocessor& postprocessor_;
};

}  // namespace kflee::pipeline
