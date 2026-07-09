#pragma once

#include "kflee/filter/filter.hpp"

namespace kflee::filter_ukf {

/**
 * 无迹卡尔曼滤波器（Unscented Kalman Filter）—— 边界框跟踪
 *
 * UKF 通过确定性采样（sigma 点）处理非线性，无需计算 Jacobian 矩阵。
 * 相比 EKF 在线性化精度上更优（可捕获 2 阶矩），适合高度非线性系统。
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
 *   tlx = cx - a*h/2,  tly = cy - h/2
 *   w   = a*h,          h   = h
 *
 * UKF 参数（Julier 无迹变换）：
 *   - alpha = 1.0：sigma 点扩散系数（lambda = 0，无负中心权重）
 *   - beta = 2.0：高斯分布最优值
 *   - kappa = 0：次缩放参数
 *   - 生成 2n+1 = 17 个 sigma 点
 *
 * 与 EkfFilter 对比：
 *   - EkfFilter：解析 Jacobian 线性化，一阶精度
 *   - UkfFilter：sigma 点传播，精度可达二阶
 *   - 两者共享相同的状态空间和观测模型
 *
 * 通过 FilterRegistry 注册为 "ukf"。
 */
class UkfFilter : public kflee::filter::Filter {
public:
    UkfFilter();

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

    // === UKF 参数 ===
    static constexpr int kN = 8;          /** 状态维度 */
    static constexpr int kM = 4;          /** 观测维度 */
    static constexpr int kNumSigma = 17;  /** sigma 点数量 = 2n + 1 */
    static constexpr float kAlpha = 1.0F; /** sigma 点扩散系数 */
    static constexpr float kBeta = 2.0F;  /** 高斯分布 beta 参数 */
    static constexpr float kKappa = 0.0F; /** 次缩放参数 */
    /** lambda = alpha^2 * (n + kappa) - n */
    static constexpr float kLambda =
        kAlpha * kAlpha * (kN + kKappa) - kN;

    // 权重（构造时预计算）
    float weight_mean_[kNumSigma] = {};  /** 均值重建权重 Wm */
    float weight_cov_[kNumSigma] = {};   /** 协方差重建权重 Wc */

    static constexpr float kStdWeightPos = 1.0F / 20.0F;
    static constexpr float kStdWeightVel = 1.0F / 160.0F;

    // === 运动与观测模型 ===

    /** 恒定速度状态转移：x' = f(x, dt) */
    void process_model(const float x[8], float dt, float x_out[8]) const;

    /** 非线性观测模型：z = h(x)，状态 -> tlwh */
    void observation_model(const float x[8], float z_out[4]) const;

    // === 噪声构建 ===

    void build_process_noise(float Q[64], float dt);
    void build_measurement_noise(float R[16]);

    // === sigma 点生成 ===

    /**
     * 从当前状态分布生成 sigma 点集
     *
     * @param sigma_points  输出：[kNumSigma][8] sigma 点数组
     */
    void generate_sigma_points(float sigma_points[kNumSigma][8]) const;
};

/**
 * 向全局 FilterRegistry 注册 "ukf" 滤波器
 */
void register_filter();

}  // namespace kflee::filter_ukf
