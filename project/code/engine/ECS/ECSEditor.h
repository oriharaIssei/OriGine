#pragma once

/// parent
#include "module/editor/IEditor.h"

/// stl
#include <memory>

/// ECS
#include "component/IComponent.h"
#include "ECSManager.h"
#include "Entity.h"
// util
#include "util/nameof.h"



class ECSEditor
    : public IEditor {
public:
    ECSEditor();
    ~ECSEditor();

    void Update() override;

private:
};
