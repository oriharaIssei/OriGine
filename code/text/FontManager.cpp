#include "text/FontManager.h"

#include "logger/Logger.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace OriGine {

FontManager* FontManager::GetInstance() {
    static FontManager instance;
    return &instance;
}

std::string FontManager::MakeCacheKey(const std::string& _path, float _fontSize) {
    return _path + "|" + std::to_string(_fontSize);
}

void FontManager::Initialize() {
    nextHandle_    = 0;
    defaultHandle_ = kInvalidFontHandle;
    fonts_.clear();
    cache_.clear();
    fontList_.clear();

    // フォント一覧は JSON から読み込む（ハードコードしない）
    if (!LoadFromConfig("engine/resource/fonts/fonts.json")) {
        LOG_WARN("FontManager: fonts.json not found or empty. Falling back to builtin font.");
    }

    // 1 つも登録できなかった場合はビルトイン ASCII フォントを最低限用意する
    if (fonts_.empty()) {
        FontHandle h    = nextHandle_++;
        auto font       = std::make_unique<BitmapFont>();
        font->GenerateAsciiAtlas(16);
        fonts_[h]       = std::move(font);
        fontList_.push_back({h, "Builtin", "", 16.0f});
        defaultHandle_  = h;
    }
}

void FontManager::Finalize() {
    fonts_.clear();
    cache_.clear();
    fontList_.clear();
    nextHandle_    = 0;
    defaultHandle_ = kInvalidFontHandle;
}

bool FontManager::LoadFromConfig(const std::string& _configPath) {
    std::ifstream file(_configPath);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json data;
    try {
        file >> data;
    } catch (const std::exception& e) {
        LOG_ERROR("FontManager: failed to parse {} : {}", _configPath, e.what());
        return false;
    }

    if (!data.contains("fonts") || !data["fonts"].is_array()) {
        return false;
    }

    std::string defaultName;
    if (data.contains("defaultFont") && data["defaultFont"].is_string()) {
        defaultName = data["defaultFont"].get<std::string>();
    }

    for (const auto& entry : data["fonts"]) {
        if (!entry.contains("name") || !entry.contains("path")) {
            continue;
        }
        std::string name = entry["name"].get<std::string>();
        std::string path = entry["path"].get<std::string>();
        float size       = entry.value("size", 24.0f);

        std::vector<std::string> fallbacks;
        if (entry.contains("fallbacks") && entry["fallbacks"].is_array()) {
            for (const auto& fb : entry["fallbacks"]) {
                if (fb.is_string()) {
                    fallbacks.push_back(fb.get<std::string>());
                }
            }
        }

        FontHandle handle = Load(name, path, size, fallbacks);
        if (handle == kInvalidFontHandle) {
            continue;
        }

        // defaultFont 指定があればそれを、無ければ最初のフォントをデフォルトにする
        if (!defaultName.empty() && name == defaultName) {
            defaultHandle_ = handle;
        } else if (defaultHandle_ == kInvalidFontHandle) {
            defaultHandle_ = handle;
        }
    }

    return !fonts_.empty();
}

FontHandle FontManager::Load(const std::string& _name, const std::string& _path, float _fontSize,
                             const std::vector<std::string>& _fallbackPaths) {
    // 同一 (path, size, fallbacks) は共有する
    std::string key = MakeCacheKey(_path, _fontSize);
    for (const auto& fb : _fallbackPaths) {
        key += "|" + fb;
    }
    auto cached = cache_.find(key);
    if (cached != cache_.end()) {
        return cached->second;
    }

    auto font = std::make_unique<BitmapFont>();
    if (!font->LoadFromFile(_path, _fontSize)) {
        // TrueType 読み込み失敗時はビルトイン ASCII へフォールバック
        LOG_WARN("FontManager: failed to load '{}' ({}). Using builtin ASCII glyphs.", _name, _path);
        font->GenerateAsciiAtlas(static_cast<int>(_fontSize));
    } else {
        // フォールバックフォントは先頭から試し、最初に成功した 1 つを採用する
        for (const auto& fb : _fallbackPaths) {
            if (font->AddFallback(fb)) {
                LOG_INFO("FontManager: '{}' fallback -> {}", _name, fb);
                break;
            }
        }
    }

    FontHandle handle = nextHandle_++;
    fonts_[handle]    = std::move(font);
    cache_[key]       = handle;
    fontList_.push_back({handle, _name, _path, _fontSize});

    if (defaultHandle_ == kInvalidFontHandle) {
        defaultHandle_ = handle;
    }

    return handle;
}

BitmapFont* FontManager::GetFont(FontHandle _handle) {
    auto it = fonts_.find(_handle);
    if (it != fonts_.end()) {
        return it->second.get();
    }

    // 無効ハンドルはデフォルトにフォールバック
    auto def = fonts_.find(defaultHandle_);
    if (def != fonts_.end()) {
        return def->second.get();
    }

    return nullptr;
}

const std::string& FontManager::GetFontName(FontHandle _handle) const {
    for (const auto& entry : fontList_) {
        if (entry.handle == _handle) {
            return entry.name;
        }
    }
    return emptyName_;
}

} // namespace OriGine
