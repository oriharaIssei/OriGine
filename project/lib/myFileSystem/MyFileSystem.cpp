#include "MyFileSystem.h"

/// stl
#include <codecvt>
#include <shlobj.h> // Add this include directive
#include <shobjidl.h>

/// engine
#include "logger/Logger.h"

// externals
#include <Windows.h>

// util
#include "util/ConvertString.h"

namespace fs = std::filesystem;

std::list<std::pair<std::string, std::string>> MyFileSystem::searchFile(const std::string& directory, const std::string& extension, bool withoutExtensionOutput) {
    std::list<std::pair<std::string, std::string>> fileList;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ('.' + extension)) {
            if (withoutExtensionOutput) {
                fileList.push_back({entry.path().parent_path().string(), entry.path().filename().stem().string()});
            } else {
                fileList.push_back({entry.path().parent_path().string(), entry.path().filename().string()});
            }
        }
    }
    return fileList;
}

std::list<std::string> MyFileSystem::searchSubFolder(const std::string& directory) {
    std::list<std::string> subFolders;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry)) {
            subFolders.push_back(entry.path().string());
        }
    }
    return subFolders;
}

bool MyFileSystem::createFolder(const std::string& directory) {
    return std::filesystem::create_directories(directory);
}

void MyFileSystem::selectFolderDialog(const std::string& _defaultDirectory, std::string& _outPath) {
    HRESULT hr         = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool coInitialized = SUCCEEDED(hr);

    if (coInitialized || hr == RPC_E_CHANGED_MODE) {
        IFileOpenDialog* pFileOpen = nullptr;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            pFileOpen->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

            // デフォルトディレクトリを設定
            if (!_defaultDirectory.empty()) {
                PIDLIST_ABSOLUTE pidl;
                hr = SHParseDisplayName(std::wstring(_defaultDirectory.begin(), _defaultDirectory.end()).c_str(), NULL, &pidl, 0, NULL);
                if (SUCCEEDED(hr)) {
                    IShellItem* psi;
                    hr = SHCreateShellItem(NULL, NULL, pidl, &psi);
                    if (SUCCEEDED(hr)) {
                        pFileOpen->SetFolder(psi);
                        psi->Release();
                    }
                    CoTaskMemFree(pidl);
                }
            }

            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr)) {
                        // pszFilePath で入手されるのは フルパスなので それを _defaultDirectory からの相対パスに変換する
                        std::wstring wFullPath(pszFilePath);
                        std::string fullPath  = ConvertString(wFullPath);
                        fs::path relativePath = fs::relative(fullPath, _defaultDirectory);
                        _outPath              = relativePath.string();
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }

        if (coInitialized) {
            CoUninitialize();
        }
    } else {
        LOG_ERROR("CoInitializeEx failed with error: " + std::to_string(hr));
    }
}

bool MyFileSystem::selectFileDialog(
    const std::string& defaultDirectory,
    std::string& fileDirectory,
    std::string& filename,
    const std::vector<std::string>& extensions) {
    HRESULT hr         = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool coInitialized = SUCCEEDED(hr);

    if (coInitialized || hr == RPC_E_CHANGED_MODE) {
        IFileOpenDialog* pFileOpen = nullptr;

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            pFileOpen->SetOptions(FOS_FORCEFILESYSTEM);

            // デフォルトディレクトリを設定
            if (!defaultDirectory.empty()) {
                PIDLIST_ABSOLUTE pidl;
                hr = SHParseDisplayName(std::wstring(defaultDirectory.begin(), defaultDirectory.end()).c_str(), NULL, &pidl, 0, NULL);
                if (SUCCEEDED(hr)) {
                    IShellItem* psi;
                    hr = SHCreateShellItem(NULL, NULL, pidl, &psi);
                    if (SUCCEEDED(hr)) {
                        pFileOpen->SetFolder(psi);
                        psi->Release();
                    }
                    CoTaskMemFree(pidl);
                }
            }

            // ファイルフィルターを設定
            if (!extensions.empty()) {
                std::wstring combinedFilter;
                for (const auto& ext : extensions) {
                    std::wstring wext(ext.begin(), ext.end());
                    if (!combinedFilter.empty()) {
                        combinedFilter += L";";
                    }
                    combinedFilter += L"*." + wext;
                }
                COMDLG_FILTERSPEC fileTypeSpec = {L"Supported Files", combinedFilter.c_str()};
                pFileOpen->SetFileTypes(1, &fileTypeSpec);
                pFileOpen->SetFileTypeIndex(1); // デフォルトのファイルタイプを設定
            }

            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr)) {
                        std::wstring wFullPath(pszFilePath);
                        std::string fullPath  = ConvertString(wFullPath);
                        fs::path relativePath = fs::relative(fullPath, defaultDirectory);
                        fileDirectory         = relativePath.parent_path().string();
                        filename              = relativePath.filename().string();
                        CoTaskMemFree(pszFilePath);
                        pItem->Release();
                        pFileOpen->Release();
                        if (coInitialized) {
                            CoUninitialize();
                        }
                        return true;
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }

        if (coInitialized) {
            CoUninitialize();
        }
    } else {
        LOG_DEBUG("CoInitializeEx failed with error: " + std::to_string(hr));
    }
    return false;
}

bool MyFileSystem::removeEmptyFolder(const std::string& directory) {
    return fs::remove(directory);
}

std::uintmax_t MyFileSystem::deleteFolder(const std::string& path) {
    return fs::remove_all(path);
}

std::uintmax_t MyFileSystem::deleteFile(const std::string& filePath) {
    return fs::remove(filePath);
}
