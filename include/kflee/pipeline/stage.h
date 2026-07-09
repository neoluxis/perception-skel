#pragma once

#include "kflee/pipeline/frame.h"

#include <string>

namespace kflee::pipeline {

/**
 * 流水线阶段抽象基类
 *
 * 所有具体 Stage（裁剪、预处理、推理、后处理等）均实现此接口。
 * 每个 Stage 读取 Frame 中上游产出的字段，写入自己的产出字段。
 *
 * 设计要点：
 * - 非模板，统一接口 void process(Frame&)，Pipeline 可顺序调用任意 Stage 组合
 * - 所有外部依赖（推理器、预处理器等）通过构造函数注入（引用语义）
 * - Stage 不持有 Frame，由 Pipeline::run() 传入
 */
class Stage {
public:
    virtual ~Stage() = default;

    /**
     * 执行本阶段处理
     *
     * @param frame 管线上下文，包含所有上下游数据
     */
    virtual void process(Frame& frame) = 0;

    /**
     * 获取阶段名称（用于调试和日志）
     */
    [[nodiscard]] virtual std::string name() const = 0;
};

}  // namespace kflee::pipeline
