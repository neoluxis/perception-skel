#pragma once

#include "kflee/pipeline/stage.h"

#include <memory>
#include <vector>

namespace kflee::pipeline {

/**
 * 推理流水线
 *
 * 管理一组 Stage 并按添加顺序依次执行。
 * 所有 Stage 共享同一个 Frame 上下文，上下游通过 Frame 的字段传递数据。
 *
 * 使用方式：
 * @code
 *   Pipeline pipeline;
 *   pipeline.add_stage(std::make_unique<CropStage>(cropper));
 *   pipeline.add_stage(std::make_unique<InferStage>(inferer));
 *
 *   Frame frame;
 *   frame.image = cv::imread("input.jpg");
 *   pipeline.run(frame);
 *   // frame.detections 即为最终检测结果
 * @endcode
 */
class Pipeline {
public:
    Pipeline() = default;

    /**
     * 添加一个 Stage 到流水线末尾
     *
     * @param stage 阶段实例（所有权转移）
     */
    void add_stage(std::unique_ptr<Stage> stage);

    /**
     * 顺序执行所有 Stage
     *
     * @param frame 管线上下文，输入数据需预先填入
     */
    void run(Frame& frame);

    /**
     * 获取当前 Stage 数量
     */
    [[nodiscard]] std::size_t stage_count() const;

private:
    std::vector<std::unique_ptr<Stage>> stages_;
};

}  // namespace kflee::pipeline
