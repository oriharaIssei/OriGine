#pragma once
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

/// <summary>
/// シングルトンのメッセージバス・エンジン.
/// 異なるシステムやコンポーネント間で、型安全なイベントの発行（Emit）と購読（Subscribe）を可能にする.
/// 疎結合なアーキテクチャを実現するために使用される.
/// </summary>
class MessageBus {
public:
    /// <summary> シングルトンインスタンスの取得. </summary>
    static MessageBus* GetInstance() {
        static MessageBus instance;
        return &instance;
    }

public:
    /// <summary>
    /// 各リスナーのエントリ情報を保持する構造体.
    /// 内部的に型消去されたコールバックを保持する.
    /// </summary>
    struct ListenerEntry {
        std::function<void(const void*)> callback = nullptr;
    };

public:
    /// <summary>
    /// 特定のイベント型を購読登録（Subscribe）する.
    /// </summary>
    /// <typeparam name="Event">購読するイベントの型</typeparam>
    /// <param name="callback">イベント発生時に実行されるコールバック関数</param>
    /// <returns>購読解除に使用する一意な ID</returns>
    template <typename Event>
    size_t Subscribe(std::function<void(const Event&)> callback) {
        auto& vec  = listeners_[std::type_index(typeid(Event))].entries;
        auto& free = listeners_[std::type_index(typeid(Event))].freeList;

        size_t id;

        // 空きスロットがある場合は再利用
        if (!free.empty()) {
            id = free.back();
            free.pop_back();
            vec[id].callback =
                [=](const void* ptr) {
                    callback(*static_cast<const Event*>(ptr));
                };
        } else {
            // 空きがない場合は末尾に追加
            id = vec.size();
            vec.push_back({[=](const void* ptr) {
                callback(*static_cast<const Event*>(ptr));
            }});
        }

        return id;
    }

    /// <summary>
    /// 指定したイベントを発行（Emit）し、登録されているすべてのリスナーに通知する.
    /// </summary>
    /// <typeparam name="Event">発行するイベントの型</typeparam>
    /// <param name="event">通知するイベントオブジェクトのインスタンス</param>
    template <typename Event>
    void Emit(const Event& event) {
        auto it = listeners_.find(std::type_index(typeid(Event)));
        if (it == listeners_.end()) {
            return;
        }

        auto& vec = it->second.entries;

        // すべての有効なコールバックを呼び出す
        for (auto& entry : vec) {
            if (entry.callback) {
                entry.callback(&event);
            }
        }
    }

    /// <summary>
    /// 指定された ID と型情報を用いて、購読を解除（Unsubscribe）する.
    /// </summary>
    /// <typeparam name="Event">解除するイベントの型</typeparam>
    /// <param name="id">Subscribe 時に返された ID</param>
    template <typename Event>
    void Unsubscribe(size_t id) {
        auto it = listeners_.find(std::type_index(typeid(Event)));
        if (it == listeners_.end()) {
            return;
        }

        auto& data = it->second;

        if (id < data.entries.size()) {
            data.entries[id].callback = nullptr;
            data.freeList.push_back(id); // 空きスロットリストに追加
        }
    }

    /// <summary>
    /// 特定のイベント型に関連付けられたすべてのリスナーを解除する.
    /// </summary>
    /// <typeparam name="Event">解除するイベントの型</typeparam>
    template <typename Event>
    void UnsubscribeAll() {
        listeners_.erase(std::type_index(typeid(Event)));
    }

private:
    /// <summary>
    /// イベント型ごとのリスナーリストとフリーリスト（空きスロットの管理）を保持する内部構造体.
    /// </summary>
    struct ListenerData {
        std::vector<ListenerEntry> entries; // リスナーの配列
        std::vector<size_t> freeList; // 空きインデックスのリスト
    };

private:
    MessageBus() = default;

private:
    std::unordered_map<std::type_index, ListenerData> listeners_; // イベント型をキーとした管理マップ
};
