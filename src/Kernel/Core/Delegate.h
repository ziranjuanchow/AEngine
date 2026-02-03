#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include <mutex>
#include <atomic>

namespace AEngine {

    /**
     * @brief 代理句柄，用于取消订阅
     */
    class FDelegateHandle {
    public:
        FDelegateHandle() : m_id(0) {}
        explicit FDelegateHandle(uint64_t id) : m_id(id) {}
        
        bool IsValid() const { return m_id != 0; }
        void Reset() { m_id = 0; }

        bool operator==(const FDelegateHandle& other) const { return m_id == other.m_id; }
        bool operator!=(const FDelegateHandle& other) const { return m_id != other.m_id; }

    private:
        uint64_t m_id;
    };

    /**
     * @brief 线程安全的多播代理系统
     */
    template<typename... Args>
    class TMulticastDelegate {
    public:
        using FuncType = std::function<void(Args...)>;

        FDelegateHandle Add(FuncType func) {
            std::lock_guard<std::mutex> lock(m_mutex);
            uint64_t id = ++m_nextId;
            m_bindings.emplace_back(id, std::move(func));
            return FDelegateHandle(id);
        }

        template<typename T>
        FDelegateHandle AddRaw(T* instance, void (T::*func)(Args...)) {
            return Add([instance, func](Args... args) {
                (instance->*func)(std::forward<Args>(args)...);
            });
        }

        void Remove(FDelegateHandle& handle) {
            if (!handle.IsValid()) return;
            std::lock_guard<std::mutex> lock(m_mutex);
            m_bindings.erase(std::remove_if(m_bindings.begin(), m_bindings.end(),
                [&](const Binding& b) { return b.HandleId == handle.m_id; }), m_bindings.end());
            handle.Reset();
        }

        void Broadcast(Args... args) {
            std::vector<FuncType> targets;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (const auto& b : m_bindings) {
                    targets.push_back(b.Function);
                }
            }
            for (const auto& target : targets) {
                target(std::forward<Args>(args)...);
            }
        }

        void Clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_bindings.clear();
        }

    private:
        struct Binding {
            uint64_t HandleId;
            FuncType Function;
            Binding(uint64_t id, FuncType f) : HandleId(id), Function(std::move(f)) {}
        };

        std::vector<Binding> m_bindings;
        std::mutex m_mutex;
        static inline std::atomic<uint64_t> m_nextId{0};
    };

    // 辅助宏
    #define AE_DECLARE_MULTICAST_DELEGATE(DelegateName, ...) 
        using DelegateName = AEngine::TMulticastDelegate<__VA_ARGS__>

}