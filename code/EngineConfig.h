#pragma once
#include "math/Vector4.h"

namespace OriGine {

/// <summary>
/// エンジン全体で共有する既定値・定数の置き場。
/// 値の意味と「なぜその値なのか」をここに集約し、各所へのマジックナンバー散在を防ぐ
/// </summary>
namespace Config {
// WinApp / Window
namespace Window {
constexpr int32_t kDefaultClientWidth  = 1280; // 起動時のクライアント領域の幅[px] (720pの16:9)
constexpr int32_t kDefaultClientHeight = 720; // 起動時のクライアント領域の高さ[px]
constexpr int32_t kDefaultMinWidth     = 320; // リサイズ時に許容する最小幅[px]。これ以下だとUIが破綻する
constexpr int32_t kDefaultMinHeight    = 240; // リサイズ時に許容する最小高さ[px]
constexpr int32_t kDefaultMaxWidth     = 0; // 0は「上限なし」を表す番兵値(モニタサイズまで拡大可能)
constexpr int32_t kDefaultMaxHeight    = 0; // 同上
constexpr uint32_t kDefaultDpi         = 96; // Windowsの標準DPI。実DPIとの比でスケーリング率を求める基準値
}

// Input
namespace Input {
constexpr size_t kHistoryCount = 10; // 保持する入力履歴のフレーム数(コマンド入力の猶予に相当)
constexpr float kStickMax      = 32767.0f; // XInputのスティック値の上限(int16の最大値)。-1〜1へ正規化する際の除数
constexpr float kStickMin      = -32767.0f; // 下限。int16の最小値は-32768だが、正負を対称に扱うため-32767としている
constexpr float kTriggerMax    = 255.0f; // XInputのトリガー値の上限(uint8の最大値)。0〜1へ正規化する際の除数
}

// DirectX12 / Rendering
namespace Rendering {
constexpr uint32_t kSwapChainBufferCount = 2; // ダブルバッファリング。GPUが描画中の裏画面と表示中の表画面の2枚

// DescriptorHeap counts
// ヒープはDirectX12では実行中にサイズを変更できないため、あらかじめ十分な数を確保しておく
constexpr uint32_t kDefaultSrvHeapCount = 512; // テクスチャ・構造化バッファ等のビュー数。最も消費が多いので大きめ
constexpr uint32_t kDefaultRtvHeapCount = 32; // レンダーターゲット数(スワップチェイン+オフスクリーン各種)
constexpr uint32_t kDefaultDsvHeapCount = 8; // 深度ステンシルビュー数

// Depth / Stencil
constexpr float kDefaultDepthClear     = 1.0f; // 深度バッファのクリア値。最遠(1.0)で埋め、手前の物体ほど小さい値で上書きされる
constexpr uint8_t kDefaultStencilClear = 0;
constexpr float kMinDepth              = 0.0f; // ビューポートの深度範囲の下限(最も手前)
constexpr float kMaxDepth              = 1.0f; // ビューポートの深度範囲の上限(最も奥)

// Colors
constexpr Vec4f kDefaultClearColor = Vec4f(0.0f, 0.0f, 0.0f, 0.0f); // アルファ0の透明。上に重ねる描画結果をそのまま活かすため
}

// Billboard
namespace Billboard {
// ビルボードの回転軸を求める際、視線ベクトルと上方向がほぼ平行だと外積が0に縮退して
// 回転が破綻する。内積の絶対値がこの閾値を超えたら平行とみなし、代替の軸に切り替える
constexpr float kThreshold = 0.99f;
}

// Camera
namespace Camera {
constexpr float kDefaultFov      = 0.45f; // 垂直画角[rad](約26度)。度数法ではない点に注意
constexpr float kDefaultNearClip = 0.1f; // ニアクリップ面までの距離。小さくしすぎると深度バッファの精度が落ちる
constexpr float kDefaultFarClip  = 100.0f; // ファークリップ面までの距離。near/farの比が大きいほど深度精度が悪化する
}

// UI
namespace UI {
constexpr float kDefaultFontSize = 16.0f; // ImGuiの既定フォントサイズ[px]
} // namespace UI

// Audio
namespace Audio {
constexpr float kMinVolume     = 0.0f; // 無音
constexpr float kMaxVolume     = 2.0f; // 元の音量の2倍まで増幅を許容(1.0を超える指定を許すためXAudio2の上限より低く制限)
constexpr float kDefaultVolume = 0.5f; // 既定音量
}

// Debug
namespace Debug {
constexpr float kJointScale        = 0.01f; // スケルトン表示時のジョイントの描画サイズ。モデルを隠さない程度に小さく
constexpr float kVelocitySideAngle = 0.2f; // 速度ベクトル可視化の矢印の傘の開き角[rad]
constexpr float kVelocityRate      = 0.3f; // 矢印の傘部分が矢全体に占める長さの割合
constexpr float kVelocityScale     = 0.5f; // 速度の大きさを描画長へ変換する倍率
}

// Logger
namespace Logger {
// ログファイルはこのサイズに達した時点でローテーションする。
// 際限なく肥大化させず、かつ直近の履歴は追える量として5MB x 3世代としている
constexpr size_t kMaxLogFileSize = 1048576 * 5; // 5MB
constexpr size_t kMaxLogFiles    = 3; // 保持する世代数
}

// Raytracing
namespace Raytracing {
// TLASインスタンスの既定マスク。レイ側のマスクとAND演算して0以外なら交差判定の対象となるため、
// 全ビットを立てておくことで「どのレイからも当たる」状態になる
constexpr uint32_t kDefaultInstanceMask = 0xFF;
}

// Physics / Collision
namespace Physics {
// 浮動小数の丸め誤差を許容するための微小値。ゼロ除算や、
// 本来平行なベクトルの外積がわずかに0でない値になるケースの判定に使う
constexpr float kEpsilon = 1e-6f;
}

// Time / Frame
namespace Time {
// 1フレームのdeltaTimeの上限[秒]。ブレークポイント停止やウィンドウドラッグで
// フレームが飛んだ際に巨大なdeltaTimeが入ると、移動量が過大になって
// コライダーをすり抜ける等の破綻が起きるためクランプする(30fps相当が下限速度)
constexpr float kMaxDeltaTime    = 1.0f / 30.0f;
constexpr size_t kFpsHistorySize = 60; // FPS表示の平均を取るサンプル数(約1秒分)
}

// Profiler
// 計測基盤(階層プロファイラ / アロケーションカウンタ)関連の定数置き場。
// ここに集約する値はすべて「フレーム中に動的確保を発生させない」ための
// 固定長バッファのサイズであり、計測対象を汚染しないための設計上の要となる。
namespace Profiler {
constexpr size_t kFrameHistorySize = 300; // フレームタイム/アロケーション履歴として保持するフレーム数(グラフ表示用)
constexpr size_t kEventNameCapacity = 48; // 1イベントに記録できるスコープ名の最大文字数(null終端含む)。固定長にして動的確保を回避する
constexpr size_t kEventStreamCapacity = 8192; // スレッドごとの1フレーム分イベントバッファの最大イベント数(Begin+End合計)。超過分は記録を諦める
constexpr size_t kMaxThreadStreams = 32; // 同時に登録できるスレッド数の上限(登録リストも固定長配列にして動的確保を回避する)
}
}

} // namespace OriGine
