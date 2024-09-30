#include "directX12/dxDevice/DxDevice.h"

#include <cassert>

#include "logger/Logger.h"

void DxDevice::Init() {
	HRESULT result;

	///================================================
	///	IDXGIFactoryの初期化
	///================================================
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

	assert(SUCCEEDED(result));
	//=================================================

	///================================================
	///	IDXGIAdapterの初期化
	///================================================
	for(UINT i = 0;
		dxgiFactory_->EnumAdapterByGpuPreference(
			i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&useAdapter_))
		!= DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報取得
		DXGI_ADAPTER_DESC3 adapterDesc {};
		result = useAdapter_->GetDesc3(&adapterDesc);

		assert(SUCCEEDED(result));

		//ソフトウェアアダプタは弾く
		if(!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Logger::OutputLog(std::format(L"Use Adapter : {}\n",adapterDesc.Description));
			break;
		}
		//ソフトウェアアダプタの場合は見つからなかったことにする
		useAdapter_ = nullptr;
	}
	// "適切な" アダプターが見つからない場合はエラーを出す
	assert(useAdapter_ != nullptr);
	///================================================

	///================================================
	///	D3D12Device の生成
	///================================================
	//機能レベルとログ出力用文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char *featrueLevelStrings[] = {
		"12.2","12.1","12.0"
	};

	//レベルの高い順に生成できるか試す
	for(size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターを生成
		result = D3D12CreateDevice(
			useAdapter_.Get(),featureLevels[i],IID_PPV_ARGS(&device_)
		);
		//生成できたか確認
		if(SUCCEEDED(result)) {
			Logger::OutputLog(std::format("FeatureLevel : {}\n",featrueLevelStrings[i]));
			break;//生成出来たらループを抜ける
		}
	}
	assert(device_ != nullptr);
	///================================================

#ifdef _DEBUG
	ID3D12InfoQueue *infoQueue = nullptr;
	if(SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラーの時に止める
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION,true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR,true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING,true);

		//=========================================
		//エラーと警告の抑制
		//=========================================
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11 でのDXGIデバッグレイヤーとDx12デバッグレイヤーの相互作用によるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY serverities[] = {D3D12_MESSAGE_SEVERITY_INFO};
		D3D12_INFO_QUEUE_FILTER filter {};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(serverities);
		filter.DenyList.pSeverityList = serverities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		infoQueue->Release();//解放
	}
#endif // _DEBUG
}

void DxDevice::Finalize() {
	device_.Reset();
	dxgiFactory_.Reset();
	useAdapter_.Reset();
}
