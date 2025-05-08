#include "Transform.h"

/// engine
#include <Engine.h>

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "lib/gui/MyGui.h"
#endif // _DEBUG

Transform::Transform() {}

void Transform::Initialize([[maybe_unused]] GameEntity* _entity) {}

void Transform::Update() {
    rotate = Quaternion::Normalize(rotate);
    // 修正: 局所変換行列を作成
    worldMat = MakeMatrix::Affine(scale, rotate, translate);
    // 修正: 親の行列との乗算順序を変更 (親行列を左側に乗算)
    if (parent) {
        worldMat *= parent->worldMat;
    }
}

bool Transform::Edit() {
#ifdef _DEBUG
    bool isChange = false;
    // --------------------------- scale --------------------------- //
    isChange |= InputVectorGuiCommand("Scale", this->scale.v);
    // --------------------------- rotate --------------------------- //
    Vec4f oldRotate = this->rotate;
    if (InputVectorGui("Rotate", oldRotate.v)) {
        isChange     = true;
        EditorGroup::getInstance()->pushCommand(std::make_unique<SetterComamnd<Quaternion>>(&this->rotate, Quaternion::Normalize(oldRotate)));
    }
    // --------------------------- translate --------------------------- //
    isChange |= InputVectorGuiCommand("Translate", this->translate.v);

    if (isChange) {
        this->Update();
    }

    return isChange;
#else
    return false;
#endif // _DEBUG
}
