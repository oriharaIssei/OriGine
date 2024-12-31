#pragma once
#include "module/IModule.h"

class IEditor
    : public IModule {
public:
    IEditor()
        : IModule() {}
    virtual ~IEditor() {}

    virtual void Update() = 0;
};
