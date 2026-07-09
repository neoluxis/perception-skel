#pragma once

#include "kflee/filter/filter.hpp"

namespace kflee::filter_kf {

/**
 * 标准线性 Kalman 滤波器（恒定速度模型）
 *
 * 状态向量 (8 维)：
 *   [cx, cy, a, h, vcx, vcy, va, vh]
 *   cx, cy   — 边界框中心坐标
 *   a        — 宽高比 (width / height)
 *   h        — 高度
 *   v*       — 各分量速度
 *
 * 观测向量 (4 维，内部转换)：
 *   外部接口接受 [tlx, tly, w, h]，内部转换为 [cx, cy, a, h]
 *   再送入线性 KF 更新步骤。
 *
 * 运动模型：恒定速度
 *   cx' = cx + vcx*dt, cy' = cy + vcy*dt,
 *   a'  = a  + va*dt,  h'  = h  + vh*dt
 *
 * 噪声参数与 ByteTrack 论文一致：
 *   - 位置标准差权重 = 1/20
 *   - 速度标准差权重 = 1/160
 *
 * 通过 FilterRegistry 注册为 "kf"。
 */
class KfFilter : public kflee::filter::Filter {
public:
    KfFilter();

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

    /** 预计算的运动转移矩阵 F (8x8)，含 dt */
    float motion_mat_[64] = {};
    /** 预计算的观测矩阵 H (4x8)，固定 */
    float update_mat_[32] = {};

    static constexpr float kStdWeightPos = 1.0F / 20.0F;
    static constexpr float kStdWeightVel = 1.0F / 160.0F;

    /** 构建运动转移矩阵 F（含 dt）*/
    void build_motion_matrix(float dt);

    /** 构建过程噪声协方差 Q（根据当前状态高度自适应缩放）*/
    void build_process_noise(float Q[64], float dt);

    /** 构建观测噪声协方差 R (4x4 对角矩阵）*/
    void build_measurement_noise(float R[16]);

    /** 坐标转换 tlwh -> xyah */
    static void tlwh_to_xyah(const float tlwh[4], float xyah[4]);
    /** 坐标转换 xyah -> tlwh */
    static void xyah_to_tlwh(const float xyah[4], float tlwh[4]);
};

/**
 * 向全局 FilterRegistry 注册 "kf" 滤波器
 *
 * 应用启动时调用一次即可。重复调用安全（会覆盖之前的注册）。
 */
void register_filter();

}  // namespace kflee::filter_kf
