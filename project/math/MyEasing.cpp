#include "MyEasing.h"

/// stl
#include <memory>

/// math
#include <cmath>
#include <mathEnv.h>

namespace OriGine {

float Liner(float _t) {
    return _t;
}

float EaseInSine(float _time) {
    float easedT = 1.0f - ::std::cosf((_time * kPi) / 2.0f);
    return easedT;
}

float EaseOutSine(float _t) {
    float easedT = ::std::sinf(_t * kHalfPi);
    return easedT;
}

float EaseInOutSine(float _t) {
    float easedT = -(::std::cosf(_t * kPi) - 1.0f) / 2.0f;
    return easedT;
}

float EaseInQuad(float _t) {
    float easedT = _t * _t;
    return easedT;
}

float EaseOutQuad(float _t) {
    float easedT = 1.0f - (1.0f - _t) * (1.0f - _t);
    return easedT;
}

float EaseInOutQuad(float _t) {
    float easedT = 0.0f;
    if (_t < 0.5f) {
        easedT = 2.0f * _t * _t;
    } else {
        easedT = 1.0f - ::std::powf(-2.0f * _t + 2.0f, 2.0f) / 2;
    }
    return easedT;
}

float EaseInCubic(float _t) {
    float easedT = _t * _t * _t;
    return easedT;
}

float EaseOutCubic(float _t) {
    float easedT = 1.0f - ::std::powf(1.0f - _t, 3.0f);
    return easedT;
}

float EaseInOutCubic(float _t) {
    float easedT = 0.0f;
    if (_t < 0.5f) {
        easedT = 4.0f * _t * _t * _t;
    } else {
        easedT = 1.0f - ::std::powf(-2.0f * _t + 2.0f, 3.0f) / 2.0f;
    }
    return easedT;
}

float EaseInQuart(float _t) {
    float easedT = _t * _t * _t * _t;
    return easedT;
}

float EaseOutQuart(float _t) {
    float easedT = 1.0f - ::std::powf(1.0f - _t, 4.0f);
    return easedT;
}

float EaseInOutQuart(float _t) {
    float easedT = 0.0f;
    if (_t < 0.5f) {
        easedT = 8.0f * _t * _t * _t * _t;
    } else {
        easedT = 1.0f - ::std::powf(-2.0f * _t + 2.0f, 4.0f) / 2.0f;
    }
    return easedT;
}

float EaseInBack(float _t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    float easedT = c3 * _t * _t * _t - c1 * _t * _t;
    return easedT;
}

float EaseOutBack(float _t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    float easedT = 1.0f + c3 * ::std::powf(_t - 1.0f, 3.0f) + c1 * ::std::powf(_t - 1.0f, 2.0f);
    return easedT;
}

float EaseInOutBack(float _t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    float easedT   = 0.0f;
    if (_t < 0.5f) {
        easedT = (::std::powf(2.0f * _t, 2.0f) * ((c2 + 1.0f) * 2.0f * _t - c2)) / 2.0f;
    } else {
        easedT = (::std::powf(2.0f * _t - 2.0f, 2.0f) * ((c2 + 1.0f) * (_t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }
    return easedT;
}

float EaseInElastic(float _t) {
    const float c4 = (2.0f * kPi) / 3.0f;
    float easedT   = 0.0f;
    if (_t == 0) {
        easedT = 0.0f;
    } else if (_t == 1.0f) {
        easedT = 1.0f;
    } else {
        easedT = -::std::powf(2.0f, 10.0f * _t - 10.0f) * ::std::sinf((_t * 10.0f - 10.75f) * c4);
    }

    return easedT;
}

float EaseOutElastic(float _t) {
    const float c4 = (2.0f * kPi) / 3.0f;

    float easedT = 0.0f;
    if (_t == 0) {
        easedT = 0.0f;
    } else if (_t == 1.0f) {
        easedT = 1.0f;
    } else {
        easedT = ::std::powf(2.0f, -10.0f * _t) * ::std::sinf((_t * 10.0f - 0.75f) * c4) + 1.0f;
    }

    return easedT;
}

float EaseInOutElastic(float _t) {
    const float c5 = (2.0f * kPi) / 4.5f;

    float easedT = 0.0f;
    if (_t == 0.0f) {
        easedT = 0.0f;
    } else if (_t == 1.0f) {
        easedT = 1.0f;
    } else if (_t < 0.5f) {
        easedT = -(::std::powf(2.0f, 20.0f * _t - 10.0f) * ::std::sinf((20.0f * _t - 11.125f) * c5)) / 2.0f;
    } else {
        easedT = (::std::powf(2.0f, -20.0f * _t + 10.0f) * ::std::sinf((20.0f * _t - 11.125f) * c5)) / 2.0f + 1.0f;
    }
    return easedT;
}

float EaseInBounce(float _t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    float resultT  = 0.0f;
    float time     = 1.0f - _t;

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

float EaseOutBounce(float _t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    float easedT = 0.0f;
    if (_t < 1.0f / d1) {
        easedT = n1 * _t * _t;
    } else if (_t < 2.0f / d1) {
        easedT = n1 * (_t -= 1.5f / d1) * _t + 0.75f;
    } else if (_t < 2.5f / d1) {
        easedT = n1 * (_t -= 2.25f / d1) * _t + 0.9375f;
    } else {
        easedT = n1 * (_t -= 2.625f / d1) * _t + 0.984375f;
    }

    return easedT;
}

float EaseInOutBounce(float _t) {
    float easedT = 0.0f;
    if (_t < 0.5f) {
        EaseOutBounce(1.0f - 2.0f * _t);
        easedT = (1.0f - easedT) / 2.0f;
    } else {
        EaseOutBounce(2.0f * _t - 1.0f);
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

void OriGine::EasingComboGui(const ::std::string& _label, OriGine::EaseType& _easeType) {
    if (ImGui::BeginCombo(_label.c_str(), OriGine::EasingNames[_easeType].c_str())) {
        bool isSelected   = false;
        int32_t easeIndex = 0;
        for (const auto& [type, name] : OriGine::EasingNames) {
            isSelected = name == OriGine::EasingNames[_easeType];
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                if (_easeType != type) {
                    // command 発行
                    auto command = ::std::make_unique<SetterCommand<OriGine::EaseType>>(&_easeType, type);
                    OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                }
                _easeType = type;
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
