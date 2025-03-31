#pragma once

/// stl
#include <cassert>
#include <fstream>
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

    void WriteLine(const std::string& _line);

    template <typename T>
    void Write(const T& _data) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

        fileStream_.write(reinterpret_cast<const char*>(&_data), sizeof(T));
        if (!fileStream_) {
            throw std::runtime_error("Failed to write to the file.");
        }
    };


    template <int dim, typename valueType>
    void Write(const Vector<dim, valueType>& _data);

private:
    std::string directory_;
    std::string fileName_;

    std::ofstream fileStream_;
    bool isOpen_ = false;

public:
    const std::string& getDirectory() const { return directory_; }
    const std::string& getFileName() const { return fileName_; }

    bool isOpen() const { return isOpen_; }
};

class BinaryReader {
public:
    BinaryReader(const std::string& _directory, const std::string& _fileName) : directory_(_directory), fileName_(_fileName) {}
    ~BinaryReader() {
        assert(!isOpen_);
    }

    void ReadBegin();
    void ReadEnd();

    std::string ReadLine();
    template <typename T>
    void Read(T& _data) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");
        readStream_.read(reinterpret_cast<char*>(&_data), sizeof(T));
    };

    template <int dim, typename valueType>
    void Read(Vector<dim, valueType>& _data);

private:
    std::string directory_;
    std::string fileName_;

    std::ifstream readStream_;
    bool isOpen_ = false;
};

// std::string専用の関数を追加
#pragma region "std::string"
template <>
inline void BinaryWriter::Write<std::string>(const std::string& _data) {
    size_t length = _data.size();
    fileName_;
    fileStream_.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
    if (length > 0) {
        fileStream_.write(_data.c_str(), length);
    }
};

template <>
inline void BinaryReader::Read<std::string>(std::string& _data) {
    size_t length = 0;

    readStream_.read(reinterpret_cast<char*>(&length), sizeof(size_t));
    _data.resize(length);

    if (length > 0) {
        char* buff = new char[length];

        readStream_.read(buff, length);
        _data = std::string(buff, length);

        delete[] buff;
    }
};
#pragma endregion "std::string"

#pragma region "Vector"
// Vector
template <int dim, typename valueType>
inline void BinaryWriter::Write(const Vector<dim, valueType>& _data) {
    for (size_t i = 0; i < dim; i++) {
        this->Write<valueType>(_data.v[i]);
    }
}

template <int dim, typename valueType>
inline void BinaryReader::Read(Vector<dim, valueType>& _data) {
    for (size_t i = 0; i < dim; i++) {
        this->Read<valueType>(_data.v[i]);
    }
}
#pragma endregion "Vector"
