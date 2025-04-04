#pragma once

/// stl
#include <cassert>
#include <fstream>
#include <map>
#include <string>

/// math
#include "Vector.h"

///====================================================================================
// BinaryFile の Input / Output
///====================================================================================

class BinaryWriter {
public:
    BinaryWriter(const std::string& _directory, const std::string& _fileName) : directory_(_directory), fileName_(_fileName) {}
    ~BinaryWriter() {};

    void WriteBegin();
    void WriteEnd();

    void WriteBeginGroup(const std::string& _groupName) {
        groupName_ = _groupName;
    }
    void WriteEndGroup() {
        groupName_ = "";
    }

    void WriteLine(const std::string& _line);

    template <typename T>
    void Write(const std::string& _label, const T& _data) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

        WriteLabel(groupName_ + _label);

        size_t length = sizeof(T);
        fileStream_.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
        fileStream_.write(reinterpret_cast<const char*>(&_data), length);
        if (!fileStream_) {
            throw std::runtime_error("Failed to write to the file.");
        }
    };

    template <int dim, typename valueType>
    void Write(const std::string& _label, const Vector<dim, valueType>& _data);

protected:
    void WriteLabel(const std::string& _label) {
        size_t length = _label.size();
        fileStream_.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
        if (length < 0) {
            assert(false);
            return;
        }
        fileStream_.write(_label.c_str(), length);
    }

private:
    std::string directory_;
    std::string fileName_;

    std::ofstream fileStream_;
    bool isOpen_ = false;

    std::string groupName_;

public:
    const std::string& getDirectory() const { return directory_; }
    const std::string& getFileName() const { return fileName_; }

    bool isOpen() const { return isOpen_; }

    const std::string& getGroupName() const { return groupName_; }
};

class BinaryReader {
public:
    BinaryReader(const std::string& _directory, const std::string& _fileName) : directory_(_directory), fileName_(_fileName) {}
    ~BinaryReader() {}

    bool ReadFile();

    std::string ReadLine();
    void ReadBeginGroup(const std::string& _groupName) {
        groupName_ = _groupName;
    }
    void ReadEndGroup() {
        groupName_ = "";
    }
    template <typename T>
    void Read(const std::string& _label, T& _data) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");
        const std::string& dataStr = readMap_[groupName_ + _label];
        if (dataStr.size() != sizeof(T)) {
            throw std::runtime_error("Size mismatch when reading data.");
        }
        std::memcpy(&_data, dataStr.data(), sizeof(T));
    }

    template <int dim, typename valueType>
    void Read(const std::string& _label, Vector<dim, valueType>& _data);
    // ----------------------------------------------

private:
    std::string directory_;
    std::string fileName_;
    std::ifstream readStream_;

    std::string groupName_;

    std::map<std::string, std::string> readMap_;

public:
    const std::string& getDirectory() const { return directory_; }
    const std::string& getFileName() const { return fileName_; }

    const std::string& getGroupName() const { return groupName_; }
};

// std::string専用の関数を追加
#pragma region "std::string"
template <>
inline void BinaryWriter::Write<std::string>(const std::string& _label, const std::string& _data) {
    WriteLabel(groupName_ + _label);
    size_t length = _data.size();
    fileStream_.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
    fileStream_.write(_data.c_str(), length);
};
template <>
inline void BinaryReader::Read<std::string>(const std::string& _label, std::string& _data) {
    _data = readMap_[groupName_ + _label];
}
#pragma endregion "std::string"

#pragma region "Vector"
template <int dim, typename valueType>
inline void BinaryWriter::Write(const std::string& _label, const Vector<dim, valueType>& _data) {
    WriteLabel(groupName_ + _label);

    size_t length = sizeof(valueType) * dim;
    fileStream_.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
    fileStream_.write(reinterpret_cast<const char*>(&_data.v[0]), length);
}

template <int dim, typename valueType>
inline void BinaryReader::Read(const std::string& _label, Vector<dim, valueType>& _data) {
    const std::string& dataStr = readMap_[groupName_ + _label];
    if (dataStr.size() != sizeof(valueType) * dim) {
        throw std::runtime_error("Size mismatch when reading Vector data.");
    }
    std::memcpy(&_data.v[0], dataStr.data(), sizeof(valueType) * dim);
}
#pragma endregion "Vector"
