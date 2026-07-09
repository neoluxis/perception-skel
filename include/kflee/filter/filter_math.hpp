#pragma once

#include <cmath>
#include <cstring>

namespace kflee::filter::math {

/**
 * 滤波器共用矩阵运算原语（固定尺寸，无堆分配，inline 实现）
 *
 * 被 libfilter_kf 和 libfilter_ekf 共享使用，消除代码重复。
 * 所有函数均为 inline，零开销。
 */

// ============================================================================
// Cholesky 分解 (4x4)
// ============================================================================

/** 4x4 Cholesky 分解 A = L * L^T，L 为下三角 */
inline bool cholesky_4x4(const float A[16], float L[16]) {
    std::memset(L, 0, sizeof(float) * 16);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j <= i; ++j) {
            float sum = A[i * 4 + j];
            for (int k = 0; k < j; ++k) {
                sum -= L[i * 4 + k] * L[j * 4 + k];
            }

            if (i == j) {
                if (sum <= 0.0F) {
                    if (sum < 1e-12F) {
                        sum = 1e-12F;
                    } else {
                        return false;
                    }
                }
                L[i * 4 + i] = std::sqrt(sum);
            } else {
                L[i * 4 + j] = sum / L[j * 4 + j];
            }
        }
    }
    return true;
}

/** 解 L * X = B，L 为 4x4 下三角 */
inline void solve_4x4_lower(const float L[16], float B[4]) {
    for (int i = 0; i < 4; ++i) {
        float sum = B[i];
        for (int j = 0; j < i; ++j) {
            sum -= L[i * 4 + j] * B[j];
        }
        B[i] = sum / L[i * 4 + i];
    }
}

/** 解 U * X = B，U 为 4x4 上三角（输入 L 为 Cholesky 下三角，实际解 L^T * X = B） */
inline void solve_4x4_upper(const float L[16], float B[4]) {
    for (int i = 3; i >= 0; --i) {
        float sum = B[i];
        for (int j = i + 1; j < 4; ++j) {
            sum -= L[j * 4 + i] * B[j];
        }
        B[i] = sum / L[i * 4 + i];
    }
}

// ============================================================================
// 矩阵乘法原语
// ============================================================================

/** C(8x8) = A(8x8) * B(8x8) */
inline void mat_mul_8x8(const float A[64], const float B[64], float C[64]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 8; ++k) {
                sum += A[i * 8 + k] * B[k * 8 + j];
            }
            C[i * 8 + j] = sum;
        }
    }
}

/** C(4x4) = A(4x8) * B(8x4) */
inline void mat_mul_4x8_8x4(const float A[32], const float B[32], float C[16]) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 8; ++k) {
                sum += A[i * 8 + k] * B[k * 4 + j];
            }
            C[i * 4 + j] = sum;
        }
    }
}

/** C(8x4) = A(8x8) * B(8x4) */
inline void mat_mul_8x8_8x4(const float A[64], const float B[32], float C[32]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 8; ++k) {
                sum += A[i * 8 + k] * B[k * 4 + j];
            }
            C[i * 4 + j] = sum;
        }
    }
}

/** C(4x8) = A(4x8) * B(8x8) */
inline void mat_mul_4x8_8x8(const float A[32], const float B[64], float C[32]) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 8; ++k) {
                sum += A[i * 8 + k] * B[k * 8 + j];
            }
            C[i * 8 + j] = sum;
        }
    }
}

/** B(8x8) = A(8x8)^T */
inline void mat_transpose_8x8(const float A[64], float AT[64]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            AT[j * 8 + i] = A[i * 8 + j];
        }
    }
}

/** B(8x4) = A(4x8)^T */
inline void mat_transpose_4x8(const float A[32], float AT[32]) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            AT[j * 4 + i] = A[i * 8 + j];
        }
    }
}

/** KH(8x8) = K(8x4) * H(4x8) */
inline void mat_mul_KH(const float K[32], const float H[32], float KH[64]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 4; ++k) {
                sum += K[i * 4 + k] * H[k * 8 + j];
            }
            KH[i * 8 + j] = sum;
        }
    }
}

/** C(8x4) = A(8x4) * B(4x4) */
inline void mat_mul_8x4_4x4(const float A[32], const float B[16], float C[32]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 4; ++k) {
                sum += A[i * 4 + k] * B[k * 4 + j];
            }
            C[i * 4 + j] = sum;
        }
    }
}

/** C(8x8) = A(8x4) * B(4x8) */
inline void mat_mul_8x4_4x8(const float A[32], const float B[32], float C[64]) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            float sum = 0.0F;
            for (int k = 0; k < 4; ++k) {
                sum += A[i * 4 + k] * B[k * 8 + j];
            }
            C[i * 8 + j] = sum;
        }
    }
}

// ============================================================================
// Cholesky 分解 (8x8)
// ============================================================================

/** 8x8 Cholesky 分解 A = L * L^T，L 为下三角 */
inline bool cholesky_8x8(const float A[64], float L[64]) {
    std::memset(L, 0, sizeof(float) * 64);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j <= i; ++j) {
            float sum = A[i * 8 + j];
            for (int k = 0; k < j; ++k) {
                sum -= L[i * 8 + k] * L[j * 8 + k];
            }

            if (i == j) {
                if (sum <= 0.0F) {
                    if (sum < 1e-12F) {
                        sum = 1e-12F;
                    } else {
                        return false;
                    }
                }
                L[i * 8 + i] = std::sqrt(sum);
            } else {
                L[i * 8 + j] = sum / L[j * 8 + j];
            }
        }
    }
    return true;
}

/** 解 L * X = B，L 为 8x8 下三角 */
inline void solve_8x8_lower(const float L[64], float B[8]) {
    for (int i = 0; i < 8; ++i) {
        float sum = B[i];
        for (int j = 0; j < i; ++j) {
            sum -= L[i * 8 + j] * B[j];
        }
        B[i] = sum / L[i * 8 + i];
    }
}

/** 解 U * X = B，U 为 8x8 上三角（输入 L 为 Cholesky 下三角，实际解 L^T * X = B） */
inline void solve_8x8_upper(const float L[64], float B[8]) {
    for (int i = 7; i >= 0; --i) {
        float sum = B[i];
        for (int j = i + 1; j < 8; ++j) {
            sum -= L[j * 8 + i] * B[j];
        }
        B[i] = sum / L[i * 8 + i];
    }
}

}  // namespace kflee::filter::math
