#pragma once

/// stl
#include <functional>
#include <memory>
#include <unordered_map>

/// base
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/renderer/primitive/base/PrimitiveType.h"

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

    void Initialize();

private:
    bool initialized_ = false; // 呼び出し回数
    std::unordered_map<PrimitiveType, std::function<std::shared_ptr<PrimitiveMeshRendererBase>()>>
        primitiveMeshFactory_;

public:
    void RegistryFactoryFunction(PrimitiveType _type, std::function<std::shared_ptr<PrimitiveMeshRendererBase>()> _func);
};
