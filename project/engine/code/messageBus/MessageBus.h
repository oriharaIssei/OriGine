#pragma once
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

/// <summary>
/// メッセージバス
/// </summary>
class MessageBus {
public:
    static MessageBus* GetInstance() {
        static MessageBus instance;
        return &instance;
    }

public:
    /// <summary>
    /// Listenerのエントリ
    /// </summary>
    struct ListenerEntry {
        std::function<void(const void*)> callback = nullptr;
    };

public:
    /// <summary>
    /// Subscribe (購読登録)
    /// </summary>
    /// <typeparam name="Event"></typeparam>
    /// <param name="callback"></param>
    /// <returns></returns>
    template <typename Event>
    size_t Subscribe(std::function<void(const Event&)> callback) {
        auto& vec  = listeners_[std::type_index(typeid(Event))].entries;
        auto& free = listeners_[std::type_index(typeid(Event))].freeList;

        size_t id;

        // 空きスロット利用
        if (!free.empty()) {
            id = free.back();
            free.pop_back();
            vec[id].callback =
                [=](const void* ptr) {
                    callback(*static_cast<const Event*>(ptr));
                };
        } else {
            id = vec.size();
            vec.push_back({[=](const void* ptr) {
                callback(*static_cast<const Event*>(ptr));
            }});
        }

        return id;
    }

    /// <summary>
    /// Emit (発行)
    /// </summary>
    /// <typeparam name="Event"></typeparam>
    /// <param name="event"></param>
    template <typename Event>
    void Emit(const Event& event) {
        auto it = listeners_.find(std::type_index(typeid(Event)));
        if (it == listeners_.end()) {
            return;
        }

        auto& vec = it->second.entries;

        for (auto& entry : vec) {
            if (entry.callback) {
                entry.callback(&event);
            }
        }
    }

    /// <summary>
    /// Unsubscribe (購読解除)
    /// </summary>
    /// <typeparam name="Event"></typeparam>
    /// <param name="id"></param>
    template <typename Event>
    void Unsubscribe(size_t id) {
        auto it = listeners_.find(std::type_index(typeid(Event)));
        if (it == listeners_.end()) {
            return;
        }

        auto& data = it->second;

        if (id < data.entries.size()) {
            data.entries[id].callback = nullptr;
            data.freeList.push_back(id);
        }
    }

    /// <summary>
    /// UnsubscribeAll (全解除)
    /// </summary>
    /// <typeparam name="Event"></typeparam>
    template <typename Event>
    void UnsubscribeAll() {
        listeners_.erase(std::type_index(typeid(Event)));
    }

private:
    /// <summary>
    /// リスナーの情報をまとめた構造体(freelist base)
    /// </summary>
    struct ListenerData {
        std::vector<ListenerEntry> entries;
        std::vector<size_t> freeList;
    };

private:
    MessageBus() = default;

private:
    std::unordered_map<std::type_index, ListenerData> listeners_;
};
