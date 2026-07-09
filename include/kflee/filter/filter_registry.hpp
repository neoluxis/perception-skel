#pragma once

#include "kflee/filter/filter.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace kflee::filter {

/**
 * 滤波器注册工厂
 *
 * 维护滤波器名称到工厂函数的映射。各滤波器实现在初始化时
 * 通过 register_filter() 注册自身，追踪器通过 create() 按名称创建实例。
 *
 * 使用方式：
 *   // 注册（在各滤波器库的 register_filter() 函数中）
 *   FilterRegistry::instance().register_filter("kf",
 *       []() { return std::make_unique<KfFilter>(); });
 *
 *   // 创建（在 Track 构造函数中，仅一次）
 *   auto filter = FilterRegistry::instance().create("kf");
 *
 * 线程安全：当前为单线程使用设计。如有多线程需求，需加互斥锁。
 */
class FilterRegistry {
public:
    /** 滤波器工厂函数签名：无参，返回 unique_ptr<Filter> */
    using Factory = std::function<std::unique_ptr<Filter>()>;

    /** 获取全局单例 */
    static FilterRegistry& instance();

    /**
     * 注册滤波器工厂
     *
     * @param name     滤波器名称（如 "kf"、"ekf"），重复注册会覆盖
     * @param factory  工厂函数
     */
    void register_filter(const std::string& name, Factory factory);

    /**
     * 按名称创建滤波器实例
     *
     * @param name  滤波器名称
     * @return      滤波器实例；名称未注册时返回 nullptr
     */
    [[nodiscard]] std::unique_ptr<Filter> create(const std::string& name) const;

    /**
     * 检查名称是否已注册
     *
     * @param name  滤波器名称
     * @return      true 表示已注册
     */
    [[nodiscard]] bool has(const std::string& name) const;

private:
    FilterRegistry() = default;
    std::map<std::string, Factory> factories_;
};

}  // namespace kflee::filter
