#pragma once

#include "kflee/data/types.h"

namespace kflee::filter {

/**
 * 滤波器抽象基类
 *
 * 定义目标跟踪中滤波器的统一接口。所有滤波器（Kalman、EKF、粒子滤波等）
 * 均需实现此接口，以便通过 FilterRegistry 工厂插拔使用。
 *
 * 设计模式：参考项目中 Inferer 抽象接口，派生类注入具体滤波算法，
 * Tracker/LibTracker 仅依赖此抽象接口。
 *
 * 状态向量 (8 维)：由各实现定义语义
 * 观测向量 (4 维)：由各实现定义语义（如 tlwh、xyah 等）
 *
 * 约定：
 *   - initiate/update 的 measurement 格式由各实现自行解释
 *   - 所有方法均为纯虚函数，派生类必须全部实现
 */
class Filter {
public:
    virtual ~Filter() = default;

    /**
     * 从初始测量值初始化滤波器状态
     *
     * 均值设为测量值（格式由实现定义），速度为零。
     * 协方差为对角矩阵，初始不确定性较大。
     *
     * @param measurement  初始测量值 (4 维)
     */
    virtual void initiate(const float measurement[4]) = 0;

    /**
     * 滤波预测步骤
     *
     * 根据运动模型预测下一时刻的状态和协方差。
     *
     * @param dt  时间步长（默认 1 帧）
     */
    virtual void predict(float dt = 1.0F) = 0;

    /**
     * 滤波更新步骤
     *
     * 用测量值修正预测状态。
     *
     * @param measurement  测量值 (4 维，格式由实现定义)
     */
    virtual void update(const float measurement[4]) = 0;

    /** 获取当前状态均值 (8 维数组) */
    [[nodiscard]] virtual const float* mean() const = 0;

    /** 获取当前协方差矩阵 (8x8 行优先数组) */
    [[nodiscard]] virtual const float* covariance() const = 0;

    /** 从当前状态提取边界框 (xyxy 格式) */
    [[nodiscard]] virtual kflee::data::HBox get_bbox() const = 0;

    /** 从当前状态提取边界框 (tlwh 格式) */
    virtual void get_tlwh(float tlwh[4]) const = 0;

    /** 直接设置状态均值 */
    virtual void set_mean(const float mean[8]) = 0;

    /** 直接设置协方差 */
    virtual void set_covariance(const float covariance[64]) = 0;
};

}  // namespace kflee::filter
