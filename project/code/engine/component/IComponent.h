#pragma once

/// stl
#include <string>

/// lib
#include <lib/binaryIO/BinaryIO.h>

/// utility
#include <concepts>
#include <util/nameof.h>

class GameEntity;
///< summary>
/// 1コンポーネントを表すクラス(基底クラス)
///</summary>
class IComponent {
public:
    IComponent(GameEntity* _hostEntity);
    virtual ~IComponent();

    virtual void Init() = 0;

    virtual bool Edit()                      = 0;
    virtual void Save(BinaryWriter& _writer) = 0;
    virtual void Load(BinaryReader& _reader) = 0;

    virtual void Finalize() = 0;

protected:
    GameEntity* hostEntity_ = nullptr;

public:
    GameEntity* getHostEntity() { return hostEntity_; }
};

/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = std::derived_from<componentType, IComponent>;
