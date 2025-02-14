#pragma once

///< summary>
/// 1コンポーネントを表すクラス(基底クラス)
///</summary>
class IComponent {
public:
    IComponent();
    virtual ~IComponent();

    virtual void Init()   = 0;
    virtual void Update() = 0;
};
