#include "ExtensionMappingRule.h"

#include <filesystem>

OriGine::ExtensionMappingRule::ExtensionMappingRule(const std::string& _fromExtension, const std::string& _toExtension)
    : fromExtension_(_fromExtension), toExtension_(_toExtension) {}

std::filesystem::path OriGine::ExtensionMappingRule::Apply(const std::filesystem::path& _directory) const {
    if (_directory.extension() == fromExtension_) {
        std::filesystem::path newPath = _directory;
        newPath.replace_extension(toExtension_);
        return newPath;
    }
    return _directory;
}
