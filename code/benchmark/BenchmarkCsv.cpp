#include "benchmark/BenchmarkCsv.h"

/// stl
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>

/// externals
#include "logger/Logger.h"

namespace OriGine::Benchmark {

namespace {

/// <summary>
/// パスの拡張子の手前にサフィックスを挿入する("out/run1.csv", ".summary" -> "out/run1.summary.csv")
/// </summary>
std::string InsertSuffixBeforeExtension(const std::string& _path, const std::string& _suffix) {
    const std::filesystem::path p(_path);
    const std::filesystem::path result = p.parent_path() / (p.stem().string() + _suffix + p.extension().string());
    return result.string();
}

/// <summary>親ディレクトリが存在しない場合は作成する</summary>
bool EnsureParentDirectory(const std::string& _path) {
    const std::filesystem::path p(_path);
    if (p.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
        if (ec) {
            LOG_ERROR("WriteBenchmarkCsv: failed to create directory '{}': {}", p.parent_path().string(), ec.message());
            return false;
        }
    }
    return true;
}

/// <summary>
/// ロケールに依存しない("." 固定、桁区切りなし)固定小数フォーマットでCSVファイルを開く
/// </summary>
std::ofstream OpenCsv(const std::string& _path) {
    std::ofstream ofs(_path, std::ios::out | std::ios::trunc);
    ofs.imbue(std::locale::classic());
    ofs << std::fixed << std::setprecision(4);
    return ofs;
}

} // namespace

bool WriteBenchmarkCsv(
    const std::string& _basePath,
    const BenchmarkSummary& _summary,
    const std::vector<BenchmarkScopeStat>& _scopes,
    const std::vector<BenchmarkFrameRecord>& _frames) {

    if (_basePath.empty()) {
        LOG_ERROR("WriteBenchmarkCsv: base path is empty.");
        return false;
    }

    const std::string summaryPath = InsertSuffixBeforeExtension(_basePath, ".summary");
    const std::string scopesPath  = InsertSuffixBeforeExtension(_basePath, ".scopes");
    const std::string framesPath  = InsertSuffixBeforeExtension(_basePath, ".frames");

    if (!EnsureParentDirectory(summaryPath) || !EnsureParentDirectory(scopesPath) || !EnsureParentDirectory(framesPath)) {
        return false;
    }

    // --- サマリ ---
    {
        std::ofstream ofs = OpenCsv(summaryPath);
        if (!ofs.is_open()) {
            LOG_ERROR("WriteBenchmarkCsv: failed to open '{}'.", summaryPath);
            return false;
        }
        ofs << "entities,extent,radius,seed,frames,warmup,sample_frame_count,"
               "avg_frame_ms,p99_frame_ms,max_frame_ms,avg_alloc_count_per_frame,avg_alloc_bytes_per_frame\n";
        ofs << _summary.config_.entityCount << ","
            << _summary.config_.extent << ","
            << _summary.config_.radius << ","
            << _summary.config_.seed << ","
            << _summary.config_.frames << ","
            << _summary.config_.warmup << ","
            << _summary.sampleFrameCount_ << ","
            << _summary.avgFrameMs_ << ","
            << _summary.p99FrameMs_ << ","
            << _summary.maxFrameMs_ << ","
            << _summary.avgAllocCountPerFrame_ << ","
            << _summary.avgAllocBytesPerFrame_ << "\n";
    }

    // --- スコープ別集計 ---
    {
        std::ofstream ofs = OpenCsv(scopesPath);
        if (!ofs.is_open()) {
            LOG_ERROR("WriteBenchmarkCsv: failed to open '{}'.", scopesPath);
            return false;
        }
        ofs << "scope_name,avg_call_count,avg_total_ms,avg_self_ms,sample_frame_count\n";
        for (const BenchmarkScopeStat& s : _scopes) {
            const double denom = s.frameSampleCount_ > 0 ? static_cast<double>(s.frameSampleCount_) : 1.0;
            ofs << s.name_ << ","
                << (static_cast<double>(s.callCount_) / denom) << ","
                << (s.totalMsSum_ / denom) << ","
                << (s.selfMsSum_ / denom) << ","
                << s.frameSampleCount_ << "\n";
        }
    }

    // --- フレーム時系列(スパイク解析用) ---
    {
        std::ofstream ofs = OpenCsv(framesPath);
        if (!ofs.is_open()) {
            LOG_ERROR("WriteBenchmarkCsv: failed to open '{}'.", framesPath);
            return false;
        }
        ofs << "frame_index,frame_ms,alloc_count,alloc_bytes\n";
        for (const BenchmarkFrameRecord& f : _frames) {
            ofs << f.frameIndex_ << "," << f.frameMs_ << "," << f.allocCount_ << "," << f.allocBytes_ << "\n";
        }
    }

    LOG_INFO("WriteBenchmarkCsv: wrote '{}', '{}', '{}'.", summaryPath, scopesPath, framesPath);
    return true;
}

} // namespace OriGine::Benchmark
