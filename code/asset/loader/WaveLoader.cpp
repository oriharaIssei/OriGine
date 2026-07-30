#include "WaveLoader.h"

/// stl
#include <cstring>
#include <fstream>

/// util
#include "logger/Logger.h"

using namespace OriGine;

namespace {

/// <summary>
/// チャンクヘッダ
/// </summary>
struct ChunkHeader {
    char id[4];
    int32_t size;
};

/// <summary>
/// RIFFヘッダ
/// </summary>
struct RiffHeader {
    ChunkHeader chunk;
    char type[4];
};

/// <summary>
/// fmtチャンク
/// </summary>
struct FormatChunk {
    ChunkHeader chunk;
    WAVEFORMATEX fmt;
};

} // namespace

/// <summary>
/// 指定されたパスの WAVE ファイルをロードする.
/// RIFF/WAVE 形式のチャンク構造を先頭から順に走査し、"fmt "チャンクから波形フォーマット(WAVEFORMATEX)を、
/// "data"チャンクから実際の音声データ本体を取り出す。両方が見つかるまでファイル終端まで走査し、
/// それ以外の未知のチャンク（メタデータ等）は読み飛ばす。
/// </summary>
/// <param name="_assetPath">読み込む WAVE ファイルのパス</param>
/// <returns>読み込まれた音声アセット（失敗時は空の SoundData）</returns>
SoundAsset WaveLoader::LoadAsset(const std::string& _assetPath) {
    SoundAsset asset;

    std::ifstream file(_assetPath, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: {}", _assetPath);
        return asset;
    }

    RiffHeader riff;
    file.read(reinterpret_cast<char*>(&riff), sizeof(riff));

    if (strncmp(riff.chunk.id, "RIFF", 4) != 0 || strncmp(riff.type, "WAVE", 4) != 0) {
        LOG_ERROR("Invalid RIFF or WAVE header: {}", _assetPath);
        return asset;
    }

    FormatChunk format{};
    ChunkHeader chunk;

    bool foundFmt  = false;
    bool foundData = false;
    uint32_t dataSize = 0;
    std::vector<BYTE> dataBuffer;

    // RIFF チャンクを先頭から順に走査し、fmt チャンク（フォーマット情報）と data チャンク（音声データ本体）を探し出す
    while (file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk))) {
        std::streampos nextChunk = file.tellg();
        nextChunk += chunk.size; // 次のチャンクの開始位置を算出

        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            foundFmt = true;
            file.read(reinterpret_cast<char*>(&format.fmt), chunk.size); // フォーマット情報を読み込む
        } else if (strncmp(chunk.id, "data", 4) == 0) {
            foundData = true;
            dataBuffer.resize(chunk.size);
            file.read(reinterpret_cast<char*>(dataBuffer.data()), chunk.size); // 音声データ本体を読み込む
            dataSize = static_cast<uint32_t>(chunk.size);
        } else {
            // 未使用のチャンクはスキップ
            file.seekg(chunk.size, std::ios::cur);
        }

        file.seekg(nextChunk); // 次のチャンクの位置へシーク
    }

    if (!foundFmt || !foundData) {
        LOG_ERROR("Required fmt or data chunk not found: {}", _assetPath);
        return asset;
    }

    asset.data.wfex       = format.fmt;
    asset.data.pBuffer    = std::move(dataBuffer);
    asset.data.bufferSize = dataSize;

    return asset;
}
