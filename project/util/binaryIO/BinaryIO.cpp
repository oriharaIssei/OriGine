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
bool BinaryReader::ReadFile() {
    // ファイルを開く
    std::string path = directory_ + "/" + fileName_;
    readStream_.open(path, std::ios::binary);

    if (!readStream_.is_open()) {
        MessageBoxA(nullptr, ("Failed to open file: " + path).c_str(), "Error", MB_OK);
        return false;
    }

    while (true) {
        if (readStream_.eof()) {
            break;
        }
        if (readStream_.fail()) {
            MessageBoxA(nullptr, ("Failed to read from file: " + path).c_str(), "Error", MB_OK);
            return false;
        }

        std::string label;
        { // ラベルの読み込み
            size_t length = 0;
            readStream_.read(reinterpret_cast<char*>(&length), sizeof(size_t));
            if (length < 0) {
                assert(false);
                break;
            }
            label.resize(length);

            readStream_.read(&label[0], length);
        }

        { // データの読み込み
            size_t length = 0;
            readStream_.read(reinterpret_cast<char*>(&length), sizeof(size_t));
            std::string data(length, '\0');
            readStream_.read(&data[0], length);
            readMap_[label] = data;
        }
    }

    readStream_.close();
    return true;
}

std::string BinaryReader::ReadLine() {
    std::string line;
    std::getline(readStream_, line);
    return line;
}
#pragma endregion
