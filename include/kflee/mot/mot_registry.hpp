#pragma once

#include "kflee/mot/mot.h"
#include "kflee/mot/mot_config.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace kflee::mot {

/**
 * 跟踪器注册工厂
 *
 * 维护跟踪器名称到工厂函数的映射。各跟踪器通过 MotRegistrar 模板
 * 自动注册自身（库加载时静态构造），应用通过 create() 按名称创建实例。
 *
 * 使用方式：
 *   // 注册（在跟踪器 .cpp 中，一行即可，库加载时自动执行）
 *   static const kflee::mot::MotRegistrar<ByteMot> kRegistrar("byte_track");
 *
 *   // 创建（在应用层，仅一次）
 *   auto tracker = MotRegistry::instance().create("byte_track", cfg);
 *
 * 线程安全：当前为单线程使用设计。如有多线程需求，需加互斥锁。
 */
class MotRegistry {
public:
    /** 跟踪器工厂函数签名：接受 MotConfig，返回 unique_ptr<Mot> */
    using Factory = std::function<std::unique_ptr<Mot>(const MotConfig&)>;

    /** 获取全局单例 */
    static MotRegistry& instance();

    /**
     * 注册跟踪器工厂
     *
     * @param name     跟踪器名称（如 "byte_track"），重复注册会覆盖
     * @param factory  工厂函数
     */
    void register_mot(const std::string& name, Factory factory);

    /**
     * 按名称创建跟踪器实例
     *
     * @param name  跟踪器名称
     * @param cfg   跟踪器配置参数
     * @return      跟踪器实例；名称未注册时返回 nullptr
     */
    [[nodiscard]] std::unique_ptr<Mot> create(const std::string& name,
                                                   const MotConfig& cfg) const;

    /**
     * 检查名称是否已注册
     *
     * @param name  跟踪器名称
     * @return      true 表示已注册
     */
    [[nodiscard]] bool has(const std::string& name) const;

private:
    MotRegistry() = default;
    std::map<std::string, Factory> factories_;
};

/**
 * 跟踪器自动注册辅助模板
 *
 * 每个跟踪器实现文件只需声明一个 static const 实例，
 * 其构造函数会在库加载时自动将工厂注册到 MotRegistry。
 *
 * 使用方式（在任意 .cpp 文件中）：
 *   static const kflee::mot::MotRegistrar<ByteMot> kRegistrar("byte_track");
 *
 * 线程安全：静态初始化阶段单线程执行，安全。
 */
template <typename T>
struct MotRegistrar {
    explicit MotRegistrar(const std::string& name) {
        MotRegistry::instance().register_mot(name,
            [](const MotConfig& cfg) -> std::unique_ptr<Mot> {
                return std::make_unique<T>(cfg);
            });
    }
};

}  // namespace kflee::mot
