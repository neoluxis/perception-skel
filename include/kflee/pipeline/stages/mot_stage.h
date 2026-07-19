#pragma once

#include "kflee/pipeline/stage.h"
#include "kflee/mot/mot.h"

#include <string>

namespace kflee::pipeline {

/**
 * 多目标跟踪阶段
 *
 * 对 NmsStage 输出的检测框执行帧间目标关联，
 * 原地更新 frame.detections 中的 track_id 字段。
 *
 * 本阶段是有状态的：Mot 实例在帧间保持轨迹记忆。
 * Mot 由调用方创建并通过构造函数注入（引用语义）。
 *
 * 读取 frame.detections，原地修改其中的 track_id。
 *
 * 使用方式：
 * @code
 *   auto mot = kflee::mot::MotRegistry::instance()
 *       .create("byte_mot", cfg);
 *   pipeline.add_stage(
 *       std::make_unique<kflee::pipeline::MotStage>(*mot));
 * @endcode
 */
class MotStage : public Stage {
public:
    /**
     * @param mot  跟踪器实例的引用（由调用方管理生命周期）
     */
    explicit MotStage(kflee::mot::Mot& mot);

    void process(Frame& frame) override;

    [[nodiscard]] std::string name() const override;

private:
    kflee::mot::Mot& mot_;
};

}  // namespace kflee::pipeline
