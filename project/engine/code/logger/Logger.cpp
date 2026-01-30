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

#include "EngineConfig.h"
#include "myFileSystem/MyFileSystem.h"

/// util
#include "util/StringUtil.h"

using namespace OriGine;

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

/// <summary>
/// ロガーの初期化処理.
/// ログフォルダの作成、spdlog の設定、ログローテーションの設定を行う.
/// デバッグビルド時は ImGui 用のシンクも追加される.
/// </summary>
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
        const std::string logFileName = TimeToString() + "_" + GetCurrentConfigString() + ".log";

        const size_t kMaxFileSize = Config::Logger::kMaxLogFileSize;
        const size_t kMaxFiles    = Config::Logger::kMaxLogFiles;

        // ログフォルダの作成
        myfs::CreateFolder(logFolder);

        // logger の作成 (ファイルも作成してくれる)
        logger_ = spdlog::rotating_logger_mt(
            "defaultLog", // logger名
            logFolder + "/" + logFileName, // ログファイル名
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

/// <summary>
/// ロガーの終了処理.
/// spdlog のシャットダウンを行い、リソースを解放する.
/// </summary>
void Logger::Finalize() {
    if (logger_) {
        // ロガーの終了処理
        spdlog::drop_all();
        spdlog::shutdown();
        logger_ = nullptr;
    }
}

/// <summary>
/// TRACE レベルでログを直接出力する（内部用）.
/// </summary>
void Logger::DirectTrace(const std::string& _message, const char* _file, const char* _function, int _line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{_file, _line, _function}, spdlog::level::trace, _message);
    }
    std::string debugmessage = std::format("[TRACE] / {}[{}]::{}  {}", _file, _line, _function, _message);
    OutputDebugStringA((debugmessage + "\n").c_str());
}

/// <summary>
/// INFO レベルでログを直接出力する（内部用）.
/// </summary>
void Logger::DirectInfo(const std::string& _message, const char* _file, const char* _function, int _line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{_file, _line, _function}, spdlog::level::info, _message);
    }
    std::string debugmessage = std::format("[INFO] / {}[{}]::{}  {}", _file, _line, _function, _message);
    OutputDebugStringA((debugmessage + "\n").c_str());
}

/// <summary>
/// DEBUG レベルでログを直接出力する（内部用）.
/// </summary>
void Logger::DirectDebug(const std::string& _message, const char* _file, const char* _function, int _line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{_file, _line, _function}, spdlog::level::debug, _message);
    }
    std::string debugmessage = std::format("[DEBUG] / {}[{}]::{}  {}", _file, _line, _function, _message);
    OutputDebugStringA((debugmessage + "\n").c_str());
}

/// <summary>
/// WARN レベルでログを直接出力する（内部用）.
/// </summary>
void Logger::DirectWarn(const std::string& _message, const char* _file, const char* _function, int _line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{_file, _line, _function}, spdlog::level::warn, _message);
    }

    std::string debugmessage = std::format("[WARN] / {}[{}]::{}  {}", _file, _line, _function, _message);
    OutputDebugStringA((debugmessage + "\n").c_str());
}

/// <summary>
/// ERROR レベルでログを直接出力する（内部用）.
/// </summary>
void Logger::DirectError(const std::string& _message, const char* _file, const char* _function, int _line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{_file, _line, _function}, spdlog::level::err, _message);
    }

    std::string debugmessage = std::format("[ERROR] / {}[{}]::{}  {}", _file, _line, _function, _message);
    OutputDebugStringA((debugmessage + "\n").c_str());
}

/// <summary>
/// CRITICAL レベルでログを直接出力する（内部用）.
/// </summary>
void Logger::DirectCritical(const std::string& _message, const char* _file, const char* _function, int _line) {
    if (logger_) {
        logger_->log(spdlog::source_loc{_file, _line, _function}, spdlog::level::critical, _message);
    }

    std::string debugmessage = std::format("[CRITICAL] / {}[{}]::{}  {}", _file, _line, _function, _message);
    OutputDebugStringA((debugmessage + "\n").c_str());
}

/// <summary>
/// DirectX12 のデバッグレイヤーからメッセージを取得し、対応するレベルで出力する.
/// </summary>
void Logger::DirectXLog(const char* _file, const char* _function, int _line) {
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = DxDebug::GetInstance()->GetInfoQueue();
    if (!infoQueue) {
        DirectError("ID3D12InfoQueue is null.", _file, _function, _line);
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

        std::string messageLevel = "WARNING";
        switch (severity) {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            // 重大な破損
            messageLevel = "CORRUPTION";
            DirectCritical(std::format("[D3D12][{}] {}", messageLevel, message->pDescription), _file, _function, _line);
            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            // エラー
            messageLevel = "ERROR";
            DirectError(std::format("[D3D12][{}] {}", messageLevel, message->pDescription), _file, _function, _line);
            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            // 警告
            messageLevel = "WARNING";
            DirectWarn(std::format("[D3D12][{}] {}", messageLevel, message->pDescription), _file, _function, _line);
            break;

        case D3D12_MESSAGE_SEVERITY_INFO:
            // 情報
            messageLevel = "INFO";
            DirectInfo(std::format("[D3D12][{}] {}", messageLevel, message->pDescription), _file, _function, _line);
            break;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
            // 通常メッセージ
            messageLevel = "MESSAGE";
            DirectInfo(std::format("[D3D12][{}] {}", messageLevel, message->pDescription), _file, _function, _line);
            break;
        }
    }

    infoQueue->ClearStoredMessages();
}

#pragma endregion

#pragma region "GuiLogger"

GuiLogger::GuiLogger() {}
GuiLogger::~GuiLogger() {}

/// <summary>
/// GUIロガーの初期化. Engine の持つコアロガーの参照を取得する.
/// </summary>
void GuiLogger::Initialize() {
    logger_ = Logger::logger_;
}

/// <summary>
/// ImGui を使用してログウィンドウを描画する.
/// ロガーの各シンクから ImGuiLogSink を探し、蓄積されたログメッセージを解析してテーブル表示する.
/// </summary>
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
                        // spdlog のパターンに基づき文字列解析を行い、各フィールドを抽出する
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

                        // レベルに応じてテキストの色を設定
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

/// <summary>
/// GUIロガーの終了処理.
/// </summary>
void GuiLogger::Finalize() {
    // GUIの終了処理
    logger_.reset();
    logger_ = nullptr;
}

#pragma endregion
