#include "Logger.h"

/// api
#include <Windows.h>

/// engine
#define RESOURCE_DIRECTORY
#include "engine/EngineInclude.h"

/// externals
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#ifdef _DEBUG
#include "imgui/imgui.h"
#include "ImGuiLogSink.h"
#endif //_DEBUG

/// lib
#include "myFileSystem/MyFileSystem.h"

/// util
#include "util/ConvertString.h"

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

std::string getCurrentDateTime() {
    // 現在時刻を取得
    auto now        = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    // tm構造体を安全に取得
    struct tm time_info;
    localtime_s(&time_info, &time_t_now);

    // 時刻をフォーマット
    std::ostringstream oss;
    oss << std::put_time(&time_info, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
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
        const std::string logFileName = getCurrentDateTime() + ".log";

        const size_t kMaxFileSize = static_cast<size_t>(1048576) * 5; // 5MB
        const size_t kMaxFiles    = 3; // 3ファイルまで保存

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

        // ログのフォーマットを設定 (出力される情報のフォーマット)
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        // flush されるレベルを設定
        // debugレベル以上のログが出力された場合に flush する
#ifdef _DEBUG
        logger_->flush_on(spdlog::level::trace);
#else
        logger_->flush_on(spdlog::level::debug);
#endif

        // ImGui用のログ sink を追加
        auto imgui_sink = std::make_shared<ImGuiLogSink>();
        imgui_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger_->sinks().push_back(imgui_sink);

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

void Logger::Trace(const std::string& message) {
    if (logger_) {
        logger_->trace(message);
    }
}

void Logger::Info(const std::string& message) {
    if (logger_) {
        logger_->info(message);
    }
}

void Logger::Debug(const std::string& message) {
    if (logger_) {
        logger_->debug(message);
    }
}

void Logger::Warn(const std::string& message) {
    if (logger_) {
        logger_->warn(message);
    }
}

void Logger::Error(const std::string& message) {
    if (logger_) {
        logger_->error(message);
    }
}

void Logger::Critical(const std::string& message) {
    if (logger_) {
        logger_->critical(message);
    }
}

void Logger::Trace(const std::wstring& message) {
    if (logger_) {
        logger_->trace(ConvertString(message));
    }
}

void Logger::Info(const std::wstring& message) {
    if (logger_) {
        logger_->info(ConvertString(message));
    }
}

void Logger::Debug(const std::wstring& message) {
    if (logger_) {
        logger_->debug(ConvertString(message));
    }
}

void Logger::Warn(const std::wstring& message) {
    if (logger_) {
        logger_->warn(ConvertString(message));
    }
}

void Logger::Error(const std::wstring& message) {
    if (logger_) {
        logger_->error(ConvertString(message));
    }
}

void Logger::Critical(const std::wstring& message) {
    if (logger_) {
        logger_->critical(ConvertString(message));
    }
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
    static bool show_log_window = true;
    ImGui::Begin("Log Window", &show_log_window, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginTable("LogTable", 3)) {
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Level");
        ImGui::TableSetupColumn("Message");
        ImGui::TableHeadersRow();

        // logger_->sinks() 内にある ImGuiLogSink からログ取得
        for (auto& sink : logger_->sinks()) {
            auto imguiSink = std::dynamic_pointer_cast<ImGuiLogSink>(sink);
            if (imguiSink) {
                const auto& logs = imguiSink->getLogMessages();
                for (const auto& line : logs) {
                    // パターン: [time] [level] message
                    size_t time_start = line.find('[');
                    size_t time_end   = line.find(']');
                    std::string time_str;
                    if (time_start != std::string::npos && time_end != std::string::npos) {
                        time_str = line.substr(time_start + 1, time_end - time_start - 1);
                    }

                    size_t level_start = line.find('[', time_end);
                    size_t level_end   = line.find(']', level_start);
                    std::string level_str;
                    if (level_start != std::string::npos && level_end != std::string::npos) {
                        level_str = line.substr(level_start + 1, level_end - level_start - 1);
                    }

                    std::string msg_str;
                    if (level_end != std::string::npos && level_end + 2 < line.length()) {
                        msg_str = line.substr(level_end + 2); // "] " の後
                    }

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(time_str.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(level_str.c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(msg_str.c_str());
                }
            }
        }

        ImGui::EndTable();
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
