#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>

/// base
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/renderer/primitive/base/PrimitiveType.h"

namespace OriGine {

/// <summary>
/// PrimitiveTypeに応じたPrimitiveMeshRendererの生成関数を管理するシングルトンファクトリ。
/// </summary>
class PrimitiveMeshFactory {
public:
    static PrimitiveMeshFactory* GetInstance() {
        static PrimitiveMeshFactory instance;
        return &instance;
    }

public:
    /// <summary>
    /// PrimitiveTypeから対応するPrimitiveMeshRendererを生成する(shared_ptr版)
    /// </summary>
    /// <param name="_type"></param>
    /// <returns></returns>
    std::shared_ptr<PrimitiveMeshRendererBase> CreatePrimitiveMeshBy(PrimitiveType _type);

private:
    PrimitiveMeshFactory() {
        if (!initialized_) {
            Initialize();
            initialized_ = true;
        }
    }
    ~PrimitiveMeshFactory()                                      = default;
    PrimitiveMeshFactory(const PrimitiveMeshFactory&)            = delete;
    PrimitiveMeshFactory& operator=(const PrimitiveMeshFactory&) = delete;

    /// 各PrimitiveTypeに対応する生成関数をprimitiveMeshFactory_へ登録する
    void Initialize();

private:
    bool initialized_ = false; // 呼び出し回数
    std::unordered_map<PrimitiveType, std::function<std::shared_ptr<PrimitiveMeshRendererBase>()>>
        primitiveMeshFactory_; // PrimitiveTypeごとの生成関数を保持するマップ

public:
    /// <summary>
    /// 指定したPrimitiveTypeに対応する生成関数を登録する。既に登録済みの場合は警告を出して何もしない。
    /// </summary>
    /// <param name="_type">登録対象のプリミティブ種別</param>
    /// <param name="_func">生成関数</param>
    void RegistryFactoryFunction(PrimitiveType _type, std::function<std::shared_ptr<PrimitiveMeshRendererBase>()> _func);
};

} // namespace OriGine
