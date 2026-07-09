#pragma once

#include "kflee/perception/yolov8_postproc.h"
#include "kflee/pipeline/stage.h"

#include <string>

namespace kflee::pipeline {

/**
 * YOLOv8 后处理阶段（逐切片解码 + 坐标映射）
 *
 * 对每个切片的推理输出执行 DFL 解码、坐标逆映射，
 * 并将检测框从切片坐标系映射回原图坐标系。
 *
 * 读取 frame.infer_results、frame.letterboxes、frame.slices，
 * 写入 frame.detections（合并后、NMS 前）。
 */
class PostprocStage : public Stage {
public:
    /**
     * @param postprocessor YOLOv8 后处理器引用
     */
    explicit PostprocStage(kflee::perception::YoloV8Postprocessor& postprocessor);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::perception::YoloV8Postprocessor& postprocessor_;

    /**
     * 将 dnn::InferResult 转换为 perception::TensorData 列表
     */
    static std::vector<kflee::perception::TensorData> convert_to_tensor_data(
        const kflee::dnn::InferResult& result);
};

}  // namespace kflee::pipeline
