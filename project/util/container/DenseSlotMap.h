#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

/// <summary>
/// 大規模シミュレーション向け高性能エンティティコンテナ
/// </summary>
/// <typeparam name="T">格納するエンティティ型</typeparam>
/// <typeparam name="IDType"> 安定IDの整数型（デフォルト: uint32_t）</typeparam>
/// <remarks>
/// 3つの内部ベクタ（Data / DataIndex / ID）を同期管理し、
/// *-ベクタ同等の反復処理性能（キャッシュフレンドリーな連続メモリ走査）
/// * -O(1) の追加・削除（Swap and Pop）
/// * -安定した外部公開ID（削除・移動でも無効化されない）* を同時に実現する。
/// </remarks>
template <typename T, typename IDType = uint32_t>
class DenseSlotMap {
public:
    static_assert(std::is_unsigned_v<IDType>, "IDType must be an unsigned integer type");

    using value_type      = T;
    using id_type         = IDType;
    using size_type       = std::size_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;

    // ── イテレータ（Data Vector を直接走査） ──────────────────────

    using iterator               = typename std::vector<T>::iterator;
    using const_iterator         = typename std::vector<T>::const_iterator;
    using reverse_iterator       = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

    /// @brief 先頭イテレータを返す
    iterator Begin() noexcept { return data_.begin(); }

    /// @brief 末尾イテレータを返す
    iterator End() noexcept { return data_.end(); }

    /// @brief 先頭 const イテレータを返す
    const_iterator Begin() const noexcept { return data_.begin(); }

    /// @brief 末尾 const イテレータを返す
    const_iterator End() const noexcept { return data_.end(); }

    /// @brief 先頭 const イテレータを返す
    const_iterator CBegin() const noexcept { return data_.cbegin(); }

    /// @brief 末尾 const イテレータを返す
    const_iterator CEnd() const noexcept { return data_.cend(); }

    /// @brief 末尾からの先頭リバースイテレータを返す
    reverse_iterator RBegin() noexcept { return data_.rbegin(); }

    /// @brief 末尾からの末尾リバースイテレータを返す
    reverse_iterator REnd() noexcept { return data_.rend(); }

    /// @brief 末尾からの先頭 const リバースイテレータを返す
    const_reverse_iterator RBegin() const noexcept { return data_.rbegin(); }

    /// @brief 末尾からの末尾 const リバースイテレータを返す
    const_reverse_iterator REnd() const noexcept { return data_.rend(); }

    // ── STL互換 range-for サポート ────────────────────────────────

    /// @brief 先頭イテレータを返す（range-for / STL アルゴリズム用）
    iterator begin() noexcept { return data_.begin(); }

    /// @brief 末尾イテレータを返す（range-for / STL アルゴリズム用）
    iterator end() noexcept { return data_.end(); }

    /// @brief 先頭 const イテレータを返す（range-for / STL アルゴリズム用）
    const_iterator begin() const noexcept { return data_.begin(); }

    /// @brief 末尾 const イテレータを返す（range-for / STL アルゴリズム用）
    const_iterator end() const noexcept { return data_.end(); }

    // ── 定数 ─────────────────────────────────────────────────────

    /// @brief 無効IDを表すセンチネル値
    static constexpr id_type kInvalidId = (std::numeric_limits<id_type>::max)();

    // ── コンストラクタ ───────────────────────────────────────────

    DenseSlotMap() = default;

    /**
     * @brief 初期容量を指定してコンテナを構築する
     *
     * @param reserved 事前確保する要素数
     */
    explicit DenseSlotMap(size_type reserved) {
        data_.reserve(reserved);
        idVector_.reserve(reserved);
    }

    // ── 容量 ─────────────────────────────────────────────────────

    /// @brief コンテナが空かどうかを返す
    [[nodiscard]] bool Empty() const noexcept { return data_.empty(); }

    /// @brief 現在の要素数を返す
    [[nodiscard]] size_type Size() const noexcept { return data_.size(); }

    /**
     * @brief 指定数の要素を格納できるようメモリを事前確保する
     *
     * @param n 確保する要素数
     */
    void Reserve(size_type n) {
        data_.reserve(n);
        idVector_.reserve(n);
    }

    // ── 要素追加 ─────────────────────────────────────────────────

    /**
     * @brief 引数を転送して要素をその場構築し、安定IDを返す
     *
     * @tparam Args コンストラクタ引数の型パック
     * @param  args コンストラクタ引数
     * @return      発行された安定ID
     */
    template <typename... Args>
    id_type Emplace(Args&&... args) {
        const id_type id         = AcquireId();
        const size_type physical = data_.size();
        data_.emplace_back(std::forward<Args>(args)...);
        idVector_.push_back(id);
        dataIndex_[id] = static_cast<id_type>(physical);
        return id;
    }

    /**
     * @brief コピーで要素を追加し、安定IDを返す
     *
     * @param value 追加する値
     * @return      発行された安定ID
     */
    id_type Insert(const T& value) {
        return Emplace(value);
    }

    /**
     * @brief ムーブで要素を追加し、安定IDを返す
     *
     * @param value 追加する値（ムーブ元）
     * @return      発行された安定ID
     */
    id_type Insert(T&& value) {
        return Emplace(std::move(value));
    }

    // ── 要素削除（Swap and Pop） ────────────────────────────────

    /**
     * @brief 安定IDを指定して要素を削除する（O(1)）
     *
     * @details 末尾要素と対象要素を交換してから末尾を削除することで
     *          連続メモリの断片化を防ぐ（Swap and Pop）。
     *
     * @param  targetId 削除対象の安定ID
     * @return          削除が成功したら true、IDが無効なら false
     */
    bool Erase(id_type targetId) {
        if (!IsValid(targetId)) {
            return false;
        }

        const id_type targetIndex = dataIndex_[targetId];
        const id_type lastIndex   = static_cast<id_type>(data_.size() - 1);

        if (targetIndex != lastIndex) {
            // 末尾要素の安定ID
            const id_type lastId = idVector_[lastIndex];

            // 物理データの置換
            data_[targetIndex]     = std::move(data_[lastIndex]);
            idVector_[targetIndex] = lastId;

            // マッピング更新
            dataIndex_[lastId] = targetIndex;
        }

        // リソースの破棄
        data_.pop_back();
        idVector_.pop_back();

        // IDを再利用キューへ
        ReleaseId(targetId);

        return true;
    }

    // ── アクセス ─────────────────────────────────────────────────

    /**
     * @brief 安定IDで要素にアクセスする（境界チェックあり）
     *
     * @param  id アクセスする安定ID
     * @return    対応する要素への参照
     * @throws std::out_of_range IDが無効な場合
     */
    [[nodiscard]] reference At(id_type id) {
        if (!IsValid(id)) {
            throw std::out_of_range("DenseSlotMap::At — invalid ID");
        }
        return data_[dataIndex_[id]];
    }

    /**
     * @brief 安定IDで要素にアクセスする（境界チェックあり、const版）
     *
     * @param  id アクセスする安定ID
     * @return    対応する要素への const 参照
     * @throws std::out_of_range IDが無効な場合
     */
    [[nodiscard]] const_reference At(id_type id) const {
        if (!IsValid(id)) {
            throw std::out_of_range("DenseSlotMap::At — invalid ID");
        }
        return data_[dataIndex_[id]];
    }

    /**
     * @brief 安定IDで要素にアクセスする（境界チェックなし、高速）
     *
     * @param  id アクセスする安定ID（有効であること）
     * @return    対応する要素への参照
     */
    [[nodiscard]] reference operator[](id_type id) noexcept {
        return data_[dataIndex_[id]];
    }

    /**
     * @brief 安定IDで要素にアクセスする（境界チェックなし、高速、const版）
     *
     * @param  id アクセスする安定ID（有効であること）
     * @return    対応する要素への const 参照
     */
    [[nodiscard]] const_reference operator[](id_type id) const noexcept {
        return data_[dataIndex_[id]];
    }

    /**
     * @brief 物理インデックスで直接アクセスする（反復処理と組み合わせて使用）
     *
     * @param  idx 物理インデックス
     * @return     対応する要素への参照
     */
    [[nodiscard]] reference DataAt(size_type idx) noexcept { return data_[idx]; }

    /**
     * @brief 物理インデックスで直接アクセスする（const版）
     *
     * @param  idx 物理インデックス
     * @return     対応する要素への const 参照
     */
    [[nodiscard]] const_reference DataAt(size_type idx) const noexcept { return data_[idx]; }

    /**
     * @brief 物理インデックスから安定IDを逆引きする
     *
     * @param  physicalIndex 物理インデックス
     * @return               対応する安定ID
     */
    [[nodiscard]] id_type IdOf(size_type physicalIndex) const noexcept {
        return idVector_[physicalIndex];
    }

    /// @brief 生のデータポインタを返す
    [[nodiscard]] pointer Data() noexcept { return data_.data(); }

    /// @brief 生のデータポインタを返す（const版）
    [[nodiscard]] const_pointer Data() const noexcept { return data_.data(); }

    // ── クエリ ───────────────────────────────────────────────────

    /**
     * @brief 安定IDが有効かどうかを返す
     *
     * @param  id 検査する安定ID
     * @return    有効なら true
     */
    [[nodiscard]] bool IsValid(id_type id) const noexcept {
        return id < dataIndex_.size() && dataIndex_[id] != kInvalidId;
    }

    /**
     * @brief 安定IDが指す物理インデックスを取得する（デバッグ用途）
     *
     * @param  id 対象の安定ID
     * @return    物理インデックス。無効IDの場合は kInvalidId
     */
    [[nodiscard]] id_type PhysicalIndexOf(id_type id) const noexcept {
        return IsValid(id) ? dataIndex_[id] : kInvalidId;
    }

    // ── 全消去 ───────────────────────────────────────────────────

    /// @brief すべての要素とIDを削除する
    void Clear() noexcept {
        data_.clear();
        idVector_.clear();
        dataIndex_.clear();
        freeIds_.clear();
    }

private:
    // ── データ ───────────────────────────────────────────────────

    std::vector<T> data_; ///< Data Vector:  連続メモリ上のエンティティ群
    std::vector<id_type> idVector_; ///< ID Vector:    物理index → 安定ID の逆引き
    std::vector<id_type> dataIndex_; ///< Data Index:   安定ID   → 物理index のマッピング
    std::vector<id_type> freeIds_; ///< 削除済みID再利用キュー

    // ── ID管理 ───────────────────────────────────────────────────

    /**
     * @brief 再利用可能IDがあればそれを返し、なければ新規発行する
     *
     * @return 取得した安定ID
     */
    id_type AcquireId() {
        if (!freeIds_.empty()) {
            id_type id = freeIds_.back();
            freeIds_.pop_back();
            return id;
        }

        // NOTE: 新規発行時は dataIndex_ を1スロット拡張し、呼び出し元で上書きさせる
        const id_type newId = static_cast<id_type>(dataIndex_.size());
        dataIndex_.push_back(kInvalidId);
        return newId;
    }

    /**
     * @brief IDを再利用キューに戻す
     *
     * @param id 返却する安定ID
     */
    void ReleaseId(id_type id) noexcept {
        dataIndex_[id] = kInvalidId;
        freeIds_.push_back(id);
    }
};
