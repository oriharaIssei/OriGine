#include "Logger.h"

/// api
#include <Windows.h>

/// engine
#define RESOURCE_DIRECTORY
#include "directX12/DxDebug.h"
#include "engine/EngineInclude.h"

/// externals
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "ImGuiLogSink.h"
#endif //_DEBUG

#include "myFileSystem/MyFileSystem.h"

/// util
#include "util/StringUtil.h"

namespace OriGine {

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

static std::string GetCurrentConfigString() {
#if defined(_DEBUG)
    return "Debug";
#elif defined(_DEVELOP)
    return "Develop";
#elif defined(NDEBUG)
    return "Release";
#else
    return "Unknown";
#endif
}

#pragma region "Logger"
void Logger::Initialize() {
    try {
        /*
            ファイルローテートして logファイルを保存する.
            ファイルローテートとは, logの出力が 設定したサイズを超過した場合,
            新しいファイルを作成して 出力すること.
            また ファイルの枚数が設定したサイズを超過した場合,
            古いファイルを削除して 新しいファイルを作成する.
        */
        const std::string logFolder   = kEngineResourceDirectory + "/logs";
        const std::string logFileName = TimeToString() + ".log";

        const size_t kMaxFileSize = static_cast<size_t>(1048576) * 5; // 5MB
        const size_t kMaxFiles    = 3; // 3ファイルまで保存

        // ログフォルダの作成
        myfs::createFolder(logFolder);

        // logger の作成 (ファイルも作成してくれる)
        logger_ = spdlog::rotating_logger_mt(
            "defaultLog", // logger名
            logFolder + "/" + logFileName + "_" + GetCurrentConfigString(), // ログファイル名
            kMaxFileSize, // ログファイルサイズ
            kMaxFiles); // ログファイル数

        // ログレベルの設定
        spdlog::set_level(spdlog::level::trace);

        // ログのフォーマットを設定
        /*
        [%Y-%m-%d %H:%M:%S.%e] : 日付と時刻
        [%l] : ログレベル
        [%s:%# %!] : ソースファイル名、行番号、関数名
        %v : ログメッセージ
        */
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%# %!] %v");

        // flush されるレベルを設定
        // debugレベル以上のログが出力された場合に flush する
#ifdef _DEBUG
        logger_->flush_on(spdlog::level::trace);

        // ImGui用のログ sink を追加
        auto imgui_sink = std::make_shared<ImGuiLogSink>();
        imgui_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%# %!] %v");
        logger_->sinks().push_back(imgui_sink);

#else
        logger_->flush_on(spdlog::level::debug);
#endif

    } catch (const spdlog::spdlog_ex& ex) {
        fprintf(stderr, "Logger initialization failed: %s\n", ex.what());
    }
}

void Logger::Finalize() {
    if (logger_) {
        // ロガーの終了処理
        spdlog::drop_all();
        spdlog::shutdown();
        logger_ = nullptr;
    }
}

void Logger::DirectTrace(const std::string& message, const char* file, const char* function, int line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{file, line, function}, spdlog::level::trace, message);
    }
}

void Logger::DirectInfo(const std::string& message, const char* file, const char* function, int line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{file, line, function}, spdlog::level::info, message);
    }
}

void Logger::DirectDebug(const std::string& message, const char* file, const char* function, int line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{file, line, function}, spdlog::level::debug, message);
    }
}

void Logger::DirectWarn(const std::string& message, const char* file, const char* function, int line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{file, line, function}, spdlog::level::warn, message);
    }
}

void Logger::DirectError(const std::string& message, const char* file, const char* function, int line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{file, line, function}, spdlog::level::err, message);
    }
}

void Logger::DirectCritical(const std::string& message, const char* file, const char* function, int line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{file, line, function}, spdlog::level::critical, message);
    }
}

void Logger::DirectXLog(const char* file, const char* function, int line) {
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = DxDebug::GetInstance()->GetInfoQueue();
    if (!infoQueue) {
        LOG_ERROR("ID3D12InfoQueue is null.", file, function, line);
        return;
    }
    UINT64 numMessages = infoQueue->GetNumStoredMessages();

    for (UINT64 i = 0; i < numMessages; ++i) {
        SIZE_T messageLength = 0;
        infoQueue->GetMessage(i, nullptr, &messageLength);
        std::vector<char> messageData(messageLength);
        D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(messageData.data());
        infoQueue->GetMessage(i, message, &messageLength);
        D3D12_MESSAGE_SEVERITY severity = message->Severity;

        std::string massageLevel = "WARNING";
        switch (severity) {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            // 重大な破損
            massageLevel = "CORRUPTION";

            spdlog::critical("[D3D12][{}] {}",
                massageLevel,
                message->pDescription);

            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            // エラー
            massageLevel = "ERROR";

            spdlog::error("[D3D12][{}] {}",
                massageLevel,
                message->pDescription);

            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            // 警告
            massageLevel = "WARNING";

            spdlog::warn("[D3D12][{}] {}",
                massageLevel,
                message->pDescription);

            break;

        case D3D12_MESSAGE_SEVERITY_INFO:
            // 情報
            massageLevel = "INFO";
            spdlog::info("[D3D12][{}] {}",
                massageLevel,
                message->pDescription);
            break;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
            // 通常メッセージ
            massageLevel = "MESSAGE";
            spdlog::info("[D3D12][{}] {}",
                massageLevel,
                message->pDescription);
            break;
        }
    }

    infoQueue->ClearStoredMessages();
}

#pragma endregion

#pragma region "GuiLogger"

GuiLogger::GuiLogger() {}
GuiLogger::~GuiLogger() {}

void GuiLogger::Initialize() {
    logger_ = Logger::logger_;
}

void GuiLogger::Update() {
#ifdef _DEBUG
    if (ImGui::Begin("Log Window", nullptr, ImGuiWindowFlags_MenuBar)) {

        logger_ = Logger::logger_;

        ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX;

        if (ImGui::BeginTable("LogTable", 6, tableFlags)) {
            ImGui::TableSetupColumn("Level");
            ImGui::TableSetupColumn("Message");
            ImGui::TableSetupColumn("File");
            ImGui::TableSetupColumn("Function");
            ImGui::TableSetupColumn("Line");
            ImGui::TableSetupColumn("Time");
            ImGui::TableHeadersRow();

            for (auto& sink : logger_->sinks()) {
                auto imguiSink = std::dynamic_pointer_cast<ImGuiLogSink>(sink);
                if (imguiSink) {
                    const auto& logs = imguiSink->GetLogMessages();
                    for (const auto& line : logs) {
                        // パターン: [level] message [file:line function] [time]
                        std::string time_str, level_str, file_str, line_num_str, func_str, msg_str;

                        // タイムスタンプの抽出
                        size_t time_start = line.find('[');
                        size_t time_end   = line.find(']', time_start);
                        if (time_start != std::string::npos && time_end != std::string::npos) {
                            time_str = line.substr(time_start + 1, time_end - time_start - 1);
                        }

                        // ログレベルの抽出
                        size_t level_start = line.find('[', time_end + 1);
                        size_t level_end   = line.find(']', level_start);
                        if (level_start != std::string::npos && level_end != std::string::npos) {
                            level_str = line.substr(level_start + 1, level_end - level_start - 1);
                        }

                        // ファイル名の抽出
                        size_t file_start = line.find('[', level_end + 1);
                        size_t file_end   = line.find(':', file_start);
                        if (file_start != std::string::npos && file_end != std::string::npos) {
                            file_str = line.substr(file_start + 1, file_end - file_start - 1);
                        }

                        // 行番号の抽出
                        size_t line_start = file_end + 1;
                        size_t line_end   = line.find(' ', line_start);
                        if (line_start != std::string::npos && line_end != std::string::npos) {
                            line_num_str = line.substr(line_start, line_end - line_start);
                        }

                        // 関数名の抽出
                        size_t func_start = line.find(' ', line_end) + 1;
                        size_t func_end   = line.find(']', func_start);
                        if (func_start != std::string::npos && func_end != std::string::npos) {
                            func_str = line.substr(func_start, func_end - func_start);
                        }

                        // メッセージの抽出
                        size_t msg_start = func_end + 2; // "] " の後
                        if (msg_start < line.size()) {
                            msg_str = line.substr(msg_start);
                        }

                        // レベルに応じて色を設定
                        ImVec4 color;
                        if (level_str == "trace") {
                            color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
                        } else if (level_str == "info") {
                            color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                        } else if (level_str == "debug") {
                            color = ImVec4(0.0f, 0.5f, 1.0f, 1.0f); // Blue
                        } else if (level_str == "warn") {
                            color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                        } else if (level_str == "error") {
                            color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
                        } else if (level_str == "critical") {
                            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                        } else {
                            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default White
                        }

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                        ImGui::TextUnformatted(level_str.c_str());
                        ImGui::PopStyleColor();

                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(msg_str.c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(file_str.c_str());
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(func_str.c_str());
                        ImGui::TableSetColumnIndex(4);
                        ImGui::TextUnformatted(line_num_str.c_str());
                        ImGui::TableSetColumnIndex(5);
                        ImGui::TextUnformatted(time_str.c_str());

                        // セパレーターを追加
                        ImGui::TableNextRow();
                        for (int i = 0; i < 6; ++i) {
                            ImGui::TableSetColumnIndex(i);
                            ImGui::Separator();
                        }
                    }
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
#endif
}

void GuiLogger::Finalize() {
    // GUIの終了処理
    logger_.reset();
    logger_ = nullptr;
}

#pragma endregion

} // namespace OriGine
