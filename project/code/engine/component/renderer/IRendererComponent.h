#pragma once
#include "IComponent.h"

/// <summary>
/// 描画を行うコンポーネントの基底クラス
/// </summary>
class IRendererComponent
    : public IComponent {
public:
    IRendererComponent();
    ~IRendererComponent();

    virtual void Init()   = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
};
