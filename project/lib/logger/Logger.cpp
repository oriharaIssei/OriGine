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

/// util
#include "util/ConvertString.h"

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

void Logger::Initialize() {
    try {
        /*
            ファイルローテートして logファイルを保存する.
            ファイルローテートとは, logの出力が 設定したサイズを超過した場合,
            新しいファイルを作成して 出力すること.
            また ファイルの枚数が設定したサイズを超過した場合,
            古いファイルを削除して 新しいファイルを作成する.
        */
        logger_ = spdlog::rotating_logger_mt(
            "defaultLog", // logger名
            kEngineResourceDirectory + "/logs/basic-log.txt", // ログファイル名
            1048576 * 5, // ログファイルサイズ
            3); // ログファイル数

        // ログレベルの設定
        spdlog::set_level(spdlog::level::debug);

        // ログのフォーマットを設定 (出力される情報のフォーマット)
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        // flush されるレベルを設定
        // debugレベル以上のログが出力された場合に flush する
#ifdef _DEBUG
        logger_->flush_on(spdlog::level::trace);
#else
        logger_->flush_on(spdlog::level::debug);
#endif

    } catch (const spdlog::spdlog_ex& ex) {
        fprintf(stderr, "Logger initialization failed: %s\n", ex.what());
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
