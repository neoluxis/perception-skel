#pragma once

#include "kflee/dnn/inferer.h"
#include "kflee/pipeline/stage.h"

#include <string>

namespace kflee::pipeline {

/**
 * DNN 推理阶段
 *
 * 将预处理后的切片张量送入推理器，获取原始推理输出。
 * 通过抽象接口 Inferer 接收任意后端实现（ONNX、TensorRT 等）。
 *
 * 读取 frame.tensors，写入 frame.infer_results。
 */
class InferStage : public Stage {
public:
    /**
     * @param inferer DNN 推理器引用（抽象接口，可注入任意后端实现）
     */
    explicit InferStage(kflee::dnn::Inferer& inferer);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::dnn::Inferer& inferer_;
};

}  // namespace kflee::pipeline
