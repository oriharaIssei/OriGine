#pragma once

// interface
#include "IRendererComponent.h"
#include "module/IModule.h"

/// stl
#include <memory>
#include <string>
// container
#include <unordered_map>
// utility
#include <concepts>
#include "util/TypeName.h"


template <typename T>
concept IsDerivedRendererController = std::derived_from<T, IRendererComponentController>;
class RenderManager
    : public IModule {
public:
    static RenderManager* getInstance() {
        static RenderManager instance;
        return &instance;
    }

private:
    RenderManager();
    ~RenderManager();

    RenderManager(const RenderManager&)   = delete;
    operator RenderManager&()             = delete;
    operator const RenderManager&() const = delete;
    operator RenderManager&&()            = delete;

public:
    void Init();
    void RenderFrame();
    void Finalize();

private:
    std::map<std::string, std::unique_ptr<IRendererComponentController>> renderControllers_;

public: // ↓ Accessor
    void addMeshController(const std::string& _id, std::unique_ptr<IRendererComponentController> _controller) {
        renderControllers_.emplace(_id, std::move(_controller));
    }
    template <IsDerivedRendererController T>
    void addMeshController(std::unique_ptr<T> _controller) {
        renderControllers_.emplace(getTypeName<T>(), std::move(_controller));
    }

    void removeMeshController(const std::string& _id) {
        renderControllers_.erase(_id);
    }
    template <IsDerivedRendererController T>
    void removeMeshController() {
        renderControllers_.erase(getTypeName<T>());
    }

    IRendererComponentController* getRendererController(const std::string& _id) {
        auto it = renderControllers_.find(_id);
        if (it != renderControllers_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    template <IsDerivedRendererController T>
    T* getRendererController() {
        auto it = renderControllers_.find(getTypeName<T>());
        if (it != renderControllers_.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
};
