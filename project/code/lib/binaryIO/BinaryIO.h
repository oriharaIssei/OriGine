#pragma once

#include <cassert>
#include <fstream>
#include <string>

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
        fileStream_.write(reinterpret_cast<const char*>(&_data), sizeof(T));
    };

private:
    std::string directory_;
    std::string fileName_;

    std::ofstream fileStream_;
    bool isOpen_ = false;
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
        readStream_.read(reinterpret_cast<char*>(&_data), sizeof(T));
    };

private:
    std::string directory_;
    std::string fileName_;

    std::ifstream readStream_;
    bool isOpen_ = false;
};
