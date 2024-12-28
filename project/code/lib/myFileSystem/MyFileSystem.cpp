#include "MyFileSystem.h"

#include "logger/Logger.h"
#include <codecvt>
#include <shlobj.h> // Add this include directive
#include <shobjidl.h>
#include <Windows.h>

namespace fs = std::filesystem;

std::list<std::pair<std::string, std::string>> MyFileSystem::SearchFile(const std::string& directory, const std::string& extension, bool withoutExtensionOutput) {
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

std::list<std::string> MyFileSystem::SearchSubFolder(const std::string& directory) {
    std::list<std::string> subFolders;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry)) {
            subFolders.push_back(entry.path().string());
        }
    }
    return subFolders;
}

bool MyFileSystem::CreateFolder(const std::string& directory) {
    return std::filesystem::create_directories(directory);
}

void MyFileSystem::SelectFolderDialog(const std::string& _defaultDirectory,std::string& _outPath){
    HRESULT hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool coInitialized = SUCCEEDED(hr);

    if(coInitialized || hr == RPC_E_CHANGED_MODE){
        IFileOpenDialog* pFileOpen = nullptr;

        hr = CoCreateInstance(CLSID_FileOpenDialog,NULL,CLSCTX_ALL,IID_IFileOpenDialog,reinterpret_cast<void**>(&pFileOpen));

        if(SUCCEEDED(hr)){
            pFileOpen->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

            // デフォルトディレクトリを設定
            if(!_defaultDirectory.empty()){
                PIDLIST_ABSOLUTE pidl;
                hr = SHParseDisplayName(std::wstring(_defaultDirectory.begin(),_defaultDirectory.end()).c_str(),NULL,&pidl,0,NULL);
                if(SUCCEEDED(hr)){
                    IShellItem* psi;
                    hr = SHCreateShellItem(NULL,NULL,pidl,&psi);
                    if(SUCCEEDED(hr)){
                        pFileOpen->SetFolder(psi);
                        psi->Release();
                    }
                    CoTaskMemFree(pidl);
                }
            }

            hr = pFileOpen->Show(NULL);

            if(SUCCEEDED(hr)){
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if(SUCCEEDED(hr)){
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH,&pszFilePath);

                    if(SUCCEEDED(hr)){
                        // pszFilePath で入手されるのは フルパスなので それを _defaultDirectory からの相対パスに変換する
                        std::wstring wFullPath(pszFilePath);
                        std::string fullPath = Logger::ConvertString(wFullPath);
                        fs::path relativePath = fs::relative(fullPath,_defaultDirectory);
                        _outPath = relativePath.string();
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }

        if(coInitialized){
            CoUninitialize();
        }
    } else{
        Logger::OutputLog("CoInitializeEx failed with error: " + std::to_string(hr));
    }
}

bool MyFileSystem::removeEmptyFolder(const std::string& directory) {
    return fs::remove(directory);
}

std::uintmax_t MyFileSystem::deleteFolder(const std::string& path) {
    return fs::remove_all(path);
}
