#include "BinaryIO.h"

#include <Windows.h>

#pragma region BinaryWriter
void BinaryWriter::WriteBegin() {
    if (isOpen_) {
        assert(false);
        return;
    }
    // ファイルを開く
    std::string path = directory_ + "/" + fileName_;
    fileStream_.open(path, std::ios::binary);
    if (!fileStream_.is_open()) {
        MessageBoxA(nullptr, ("Failed to open file: " + path).c_str(), "Error", MB_OK);
    }
    isOpen_ = true;
}

void BinaryWriter::WriteEnd() {
    if (isOpen_) {
        fileStream_.close();
        isOpen_ = false;
    }
}
void BinaryWriter::WriteLine(const std::string& _line) {
    fileStream_ << _line << std::endl;
}

#pragma endregion

#pragma region BinaryReader
void BinaryReader::ReadBegin() {
    if (isOpen_) {
        assert(false);
        return;
    }

    // ファイルを開く
    std::string path = directory_ + "/" + fileName_;
    readStream_.open(path, std::ios::binary);

    if (!readStream_.is_open()) {
        MessageBoxA(nullptr, ("Failed to open file: " + path).c_str(), "Error", MB_OK);
    }
    isOpen_ = true;
}

void BinaryReader::ReadEnd() {
    if (isOpen_) {
        readStream_.close();
        isOpen_ = false;
    }
}

std::string BinaryReader::ReadLine() {
    std::string line;
    std::getline(readStream_, line);
    return line;
}

bool BinaryReader::Read(const std::string& _expectedLabel, std::string& _data) {
    auto pos = readStream_.tellg();
    std::string label;
    Read<std::string>(label);
    size_t length = 0;
    readStream_.read(reinterpret_cast<char*>(&length), sizeof(size_t));
    if (label != _expectedLabel) {
        readStream_.seekg(pos);
        _data = "";
        return false;
    }
    _data.resize(length);
    readStream_.read(&_data[0], length);
    return true;
}
#pragma endregion
