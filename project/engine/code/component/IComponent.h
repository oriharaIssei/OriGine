#pragma once

/// stl
#include <concepts>
#include <string>

/// lib
#include <binaryIO/BinaryIO.h>

/// utility
#include <util/nameof.h>

class GameEntity;
///< summary>
/// 1コンポーネントを表すクラス(基底クラス)
///</summary>
class IComponent {
public:
    IComponent();
    virtual ~IComponent();

    virtual void Initialize(GameEntity* _entity) = 0;

    virtual bool Edit()                      = 0;
    virtual void Save(BinaryWriter& _writer) = 0;
    virtual void Load(BinaryReader& _reader) = 0;

    virtual void Finalize() = 0;
};

/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = std::derived_from<componentType, IComponent>;
