#pragma once

#include "kflee/filter/filter.hpp"

namespace kflee::filter_ekf {

/**
 * 扩展卡尔曼滤波器（EKF）—— 边界框跟踪
 *
 * 直接使用检测器原始输出 [tlx, tly, w, h] 作为测量值，
 * 通过非线性观测函数 h(x) = [cx - a*h/2, cy - h/2, a*h, h]
 * 和解析 Jacobian 处理 a*h 非线性项。
 *
 * 状态向量 (8 维)：
 *   [cx, cy, a, h, vcx, vcy, va, vh]
 *   cx, cy — 边界框中心坐标
 *   a      — 宽高比 (width / height)
 *   h      — 高度
 *   v*     — 各分量速度
 *
 * 观测向量 (4 维，直接使用检测器原始输出)：
 *   [tlx, tly, w, h]
 *
 * 运动模型：恒定速度 (线性)
 *   cx' = cx + vcx*dt, cy' = cy + vcy*dt,
 *   a'  = a  + va*dt,  h'  = h  + vh*dt
 *
 * 观测模型（非线性，因为涉及 a*h 项）：
 *   tlx = cx - a*h/2
 *   tly = cy - h/2
 *   w   = a*h
 *   h   = h
 *
 * 与标准 Kalman 滤波器（KfFilter）的关键区别：
 *   - KfFilter 在调用前将 tlwh 转为 [cx,cy,a,h]，然后送入线性 KF
 *     （忽略了 tlwh->xyah 转换引入的非线性噪声相关）
 *   - EkfFilter 直接使用检测器原始 tlwh 作为测量值，观测函数 h(x) 包含
 *     a*h 非线性项，由 EKF 通过 Jacobian 正确传播不确定性
 *
 * 通过 FilterRegistrar 自动注册为 "ekf"。
 */
class EkfFilter : public kflee::filter::Filter {
public:
    EkfFilter() = default;

    void initiate(const float measurement[4]) override;
    void predict(float dt = 1.0F) override;
    void update(const float measurement[4]) override;

    [[nodiscard]] const float* mean() const override { return mean_; }
    [[nodiscard]] const float* covariance() const override { return covariance_; }
    [[nodiscard]] kflee::data::HBox get_bbox() const override;
    void get_tlwh(float tlwh[4]) const override;
    void set_mean(const float mean[8]) override;
    void set_covariance(const float covariance[64]) override;

private:
    float mean_[8] = {};
    float covariance_[64] = {};

    static constexpr float kStdWeightPos = 1.0F / 20.0F;
    static constexpr float kStdWeightVel = 1.0F / 160.0F;

    // === 非线性模型（恒定速度 + tlwh 观测） ===

    /** 非线性状态转移 f(x, dt)：恒定速度 */
    void state_transition(const float x[8], float dt, float x_out[8]) const;

    /** 状态转移 Jacobian F(x, dt) = ∂f/∂x */
    void state_jacobian(const float x[8], float dt, float F[64]) const;

    /** 非线性观测函数 h(x)：状态 -> tlwh */
    void observation(const float x[8], float z_out[4]) const;

    /** 观测 Jacobian H(x) = ∂h/∂x (4x8) */
    void observation_jacobian(const float x[8], float H[32]) const;

    // === 噪声构建 ===

    /** 构建过程噪声协方差 Q（8x8 对角矩阵） */
    void build_process_noise(float Q[64], float dt);

    /** 构建观测噪声协方差 R（4x4 对角矩阵） */
    void build_measurement_noise(float R[16]);
};

}  // namespace kflee::filter_ekf
