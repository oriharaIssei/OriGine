#include "MyEasing.h"

/// stl
#include <memory>

/// math
#include <cmath>
#include <MathEnv.h>

namespace OriGine {

float Linear(float t) {
    return t;
}

float EaseInSine(float time) {
    float easedT = 1.0f - ::std::cosf((time * kPi) / 2.0f);
    return easedT;
}

float EaseOutSine(float t) {
    float easedT = ::std::sinf(t * kHalfPi);
    return easedT;
}

float EaseInOutSine(float t) {
    float easedT = -(::std::cosf(t * kPi) - 1.0f) / 2.0f;
    return easedT;
}

float EaseInQuad(float t) {
    float easedT = t * t;
    return easedT;
}

float EaseOutQuad(float t) {
    float easedT = 1.0f - (1.0f - t) * (1.0f - t);
    return easedT;
}

float EaseInOutQuad(float t) {
    float easedT = 0.0f;
    if (t < 0.5f) {
        easedT = 2.0f * t * t;
    } else {
        easedT = 1.0f - ::std::powf(-2.0f * t + 2.0f, 2.0f) / 2;
    }
    return easedT;
}

float EaseInCubic(float t) {
    float easedT = t * t * t;
    return easedT;
}

float EaseOutCubic(float t) {
    float easedT = 1.0f - ::std::powf(1.0f - t, 3.0f);
    return easedT;
}

float EaseInOutCubic(float t) {
    float easedT = 0.0f;
    if (t < 0.5f) {
        easedT = 4.0f * t * t * t;
    } else {
        easedT = 1.0f - ::std::powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }
    return easedT;
}

float EaseInQuart(float t) {
    float easedT = t * t * t * t;
    return easedT;
}

float EaseOutQuart(float t) {
    float easedT = 1.0f - ::std::powf(1.0f - t, 4.0f);
    return easedT;
}

float EaseInOutQuart(float t) {
    float easedT = 0.0f;
    if (t < 0.5f) {
        easedT = 8.0f * t * t * t * t;
    } else {
        easedT = 1.0f - ::std::powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
    }
    return easedT;
}

float EaseInBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    float easedT = c3 * t * t * t - c1 * t * t;
    return easedT;
}

float EaseOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    float easedT = 1.0f + c3 * ::std::powf(t - 1.0f, 3.0f) + c1 * ::std::powf(t - 1.0f, 2.0f);
    return easedT;
}

float EaseInOutBack(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    float easedT   = 0.0f;
    if (t < 0.5f) {
        easedT = (::std::powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f;
    } else {
        easedT = (::std::powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }
    return easedT;
}

float EaseInElastic(float t) {
    const float c4 = (2.0f * kPi) / 3.0f;
    float easedT   = 0.0f;
    if (t == 0) {
        easedT = 0.0f;
    } else if (t == 1.0f) {
        easedT = 1.0f;
    } else {
        easedT = -::std::powf(2.0f, 10.0f * t - 10.0f) * ::std::sinf((t * 10.0f - 10.75f) * c4);
    }

    return easedT;
}

float EaseOutElastic(float t) {
    const float c4 = (2.0f * kPi) / 3.0f;

    float easedT = 0.0f;
    if (t == 0) {
        easedT = 0.0f;
    } else if (t == 1.0f) {
        easedT = 1.0f;
    } else {
        easedT = ::std::powf(2.0f, -10.0f * t) * ::std::sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
    }

    return easedT;
}

float EaseInOutElastic(float t) {
    const float c5 = (2.0f * kPi) / 4.5f;

    float easedT = 0.0f;
    if (t == 0.0f) {
        easedT = 0.0f;
    } else if (t == 1.0f) {
        easedT = 1.0f;
    } else if (t < 0.5f) {
        easedT = -(::std::powf(2.0f, 20.0f * t - 10.0f) * ::std::sinf((20.0f * t - 11.125f) * c5)) / 2.0f;
    } else {
        easedT = (::std::powf(2.0f, -20.0f * t + 10.0f) * ::std::sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }
    return easedT;
}

float EaseInBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    float resultT  = 0.0f;
    float time     = 1.0f - t;

    if (time < 1.0f / d1) {
        resultT = n1 * time * time;
    } else if (time < 2.0f / d1) {
        resultT = n1 * (time -= 1.5f / d1) * time + 0.75f;
    } else if (time < 2.5f / d1) {
        resultT = n1 * (time -= 2.25f / d1) * time + 0.9375f;
    } else {
        resultT = n1 * (time -= 2.625f / d1) * time + 0.984375f;
    }

    float easedT = 1.0f - resultT;
    return easedT;
}

float EaseOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    float easedT = 0.0f;
    if (t < 1.0f / d1) {
        easedT = n1 * t * t;
    } else if (t < 2.0f / d1) {
        easedT = n1 * (t -= 1.5f / d1) * t + 0.75f;
    } else if (t < 2.5f / d1) {
        easedT = n1 * (t -= 2.25f / d1) * t + 0.9375f;
    } else {
        easedT = n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }

    return easedT;
}

float EaseInOutBounce(float t) {
    float easedT = 0.0f;
    if (t < 0.5f) {
        EaseOutBounce(1.0f - 2.0f * t);
        easedT = (1.0f - easedT) / 2.0f;
    } else {
        EaseOutBounce(2.0f * t - 1.0f);
        easedT = (1.0f + easedT) / 2.0f;
    }
    return easedT;
}

} // namespace OriGine

#ifdef _DEBUG
#include "editor/EditorController.h"
#include <cstdint>
#include <editor/IEditor.h>
#include <imgui/imgui.h>
#include <string>

void OriGine::EasingComboGui(const ::std::string& label, OriGine::EaseType& easeType) {
    if (ImGui::BeginCombo(label.c_str(), OriGine::EasingNames[easeType].c_str())) {
        bool isSelected   = false;
        int32_t easeIndex = 0;
        for (const auto& [type, name] : OriGine::EasingNames) {
            isSelected = name == OriGine::EasingNames[easeType];
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                if (easeType != type) {
                    // command 発行
                    auto command = ::std::make_unique<SetterCommand<OriGine::EaseType>>(&easeType, type);
                    OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                }
                easeType = type;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
            ++easeIndex;
        }
        ImGui::EndCombo();
    }
}

#endif // _DEBUG
