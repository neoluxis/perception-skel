#pragma once

#include "kflee/pipeline/stage.h"
#include "kflee/prep/preprocessor.h"

#include <string>

namespace kflee::pipeline {

/**
 * 图像预处理阶段
 *
 * 对每个切片执行 BGR→RGB、letterbox 缩放、归一化等预处理。
 * 读取 frame.slices，写入 frame.tensors 和 frame.letterboxes。
 */
class PreprocessStage : public Stage {
public:
    /**
     * @param preprocessor 预处理器引用（生命周期需长于本 Stage）
     */
    explicit PreprocessStage(kflee::prep::Preprocessor& preprocessor);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::prep::Preprocessor& preprocessor_;
};

}  // namespace kflee::pipeline
