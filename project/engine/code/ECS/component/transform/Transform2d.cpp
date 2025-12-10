#include "Transform2d.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void Transform2d::UpdateMatrix() {
    worldMat = MakeMatrix3x3::Scale(scale)
               * MakeMatrix3x3::Rotate(rotate)
               * MakeMatrix3x3::Translate(translate);

    // 親の影響を適用
    if (parent) {
        worldMat *= parent->worldMat;
    }
}

void Transform2d::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    DragGuiVectorCommand("Scale##" + _parentLabel, scale, 0.01f);
    DragGuiCommand("Rotate##" + _parentLabel, rotate, 0.01f);
    DragGuiVectorCommand("Translate##" + _parentLabel, translate, 0.01f);
#endif // _DEBUG
}
