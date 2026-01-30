#pragma once
#include "math/Vector4.h"

namespace OriGine {

namespace Config {
// WinApp / Window
namespace Window {
constexpr int32_t kDefaultClientWidth  = 1280;
constexpr int32_t kDefaultClientHeight = 720;
}

// Input
namespace Input {
constexpr size_t kHistoryCount = 10;
constexpr float kStickMax      = 32767.0f;
constexpr float kStickMin      = -32767.0f;
constexpr float kTriggerMax    = 255.0f;
}

// DirectX12 / Rendering
namespace Rendering {
constexpr uint32_t kSwapChainBufferCount = 2;

// DescriptorHeap counts
constexpr uint32_t kDefaultSrvHeapCount = 512;
constexpr uint32_t kDefaultRtvHeapCount = 32;
constexpr uint32_t kDefaultDsvHeapCount = 8;

// Depth / Stencil
constexpr float kDefaultDepthClear     = 1.0f;
constexpr uint8_t kDefaultStencilClear = 0;
constexpr float kMinDepth              = 0.0f;
constexpr float kMaxDepth              = 1.0f;

// Colors
constexpr Vec4f kDefaultClearColor = Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
}

// Billboard
namespace Billboard {
constexpr float kThreshold = 0.99f;
}

// Camera
namespace Camera {
constexpr float kDefaultFov      = 0.45f;
constexpr float kDefaultNearClip = 0.1f;
constexpr float kDefaultFarClip  = 100.0f;
}

// UI
namespace UI {
constexpr float kDefaultFontSize = 16.0f;
} // namespace UI

// Audio
namespace Audio {
constexpr float kMinVolume     = 0.0f;
constexpr float kMaxVolume     = 2.0f;
constexpr float kDefaultVolume = 0.5f;
}

// Debug
namespace Debug {
constexpr float kJointScale        = 0.01f;
constexpr float kVelocitySideAngle = 0.2f;
constexpr float kVelocityRate      = 0.3f;
constexpr float kVelocityScale     = 0.5f;
}

// Logger
namespace Logger {
constexpr size_t kMaxLogFileSize = 1048576 * 5; // 5MB
constexpr size_t kMaxLogFiles    = 3;
}

// Raytracing
namespace Raytracing {
constexpr uint32_t kDefaultInstanceMask = 0xFF;
}

// Physics / Collision
namespace Physics {
constexpr float kEpsilon = 1e-6f;
}

// Time / Frame
namespace Time {
constexpr float kMaxDeltaTime    = 1.0f / 30.0f;
constexpr size_t kFpsHistorySize = 60;
}
}

} // namespace OriGine
