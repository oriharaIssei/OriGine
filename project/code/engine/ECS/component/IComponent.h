#pragma once

/// stl
// utility
#include <concepts>

///< summary>
/// 1コンポーネントを表すクラス(基底クラス)
///</summary>
class IComponent {
public:
    IComponent();
    virtual ~IComponent();

    virtual void Init()     = 0;
    virtual void Finalize() = 0;
};

/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = std::derived_from<componentType, IComponent>;
/*
↓ 基本 コンポーネントを編集できるようにするには
    以下の関数をしようすること
*/

///< summary>
/// コンポーネントを編集する
///</summary>
template <IsComponent componentType>
bool EditComponent(componentType* _editComponent) {
    return false;
}
