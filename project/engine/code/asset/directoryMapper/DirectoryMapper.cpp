#include "DirectoryMapper.h"

using namespace OriGine;

DirectoryMapper::DirectoryMapper() {}
DirectoryMapper::~DirectoryMapper() {}

void OriGine::DirectoryMapper::Initialize() {}
void OriGine::DirectoryMapper::Finalize() {
    rules_.clear();
}

void OriGine::DirectoryMapper::AddRule(const std::shared_ptr<IDirectoryMappingRule>& _rule) {
    rules_.emplace_back(_rule);
}

std::filesystem::path OriGine::DirectoryMapper::TryMap(const std::filesystem::path& _directory) const {
    std::filesystem::path mappedDirectory = _directory;
    for (const auto& rule : rules_) {
        mappedDirectory = rule->Apply(mappedDirectory);
    }
    return mappedDirectory;
}
