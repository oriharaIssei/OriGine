#include "ECS/system/text/TextRenderSystem.h"
#include "ECS/system/text/TextLayoutSystem.h"

#include "Engine.h"
#include "directX12/DxDescriptor.h"
#include "winApp/WinApp.h"
#include "EngineConfig.h"
#include "directX12/DxDevice.h"
#include "text/FontManager.h"

#include <algorithm>
#include <cstring>

namespace OriGine {

	static constexpr uint32_t kVerticesPerQuad = 4;
	static constexpr uint32_t kIndicesPerQuad = 6;

	TextRenderSystem::TextRenderSystem(): BaseRenderSystem(){}
	TextRenderSystem::~TextRenderSystem() = default;

	void TextRenderSystem::Initialize(){
		BaseRenderSystem::Initialize();

		WinApp* window = Engine::GetInstance()->GetWinApp();
		viewPortMat_ = MakeMatrix4x4::Orthographic(
			0,0,
			static_cast<float>(window->GetWidth()),
			static_cast<float>(window->GetHeight()),
			Config::Rendering::kMinDepth,
			Config::Camera::kDefaultFarClip);

		constBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

		auto device = Engine::GetInstance()->GetDxDevice()->device_;
		uint32_t vbSize = maxQuads_ * kVerticesPerQuad * sizeof(TextVertex);
		uint32_t ibSize = maxQuads_ * kIndicesPerQuad * sizeof(uint16_t);

		vertexBuffer_.CreateBufferResource(device,vbSize);

		indexBuffer_.CreateBufferResource(device,ibSize);

		uint16_t* indexData = nullptr;
		indexBuffer_.GetResource()->Map(0,nullptr,reinterpret_cast<void**>(&indexData));
		for(uint32_t i = 0; i < maxQuads_; ++i){
			uint16_t base = static_cast<uint16_t>(i * 4);
			indexData[i * 6 + 0] = base + 0;
			indexData[i * 6 + 1] = base + 1;
			indexData[i * 6 + 2] = base + 2;
			indexData[i * 6 + 3] = base + 1;
			indexData[i * 6 + 4] = base + 3;
			indexData[i * 6 + 5] = base + 2;
		}
		indexBuffer_.GetResource()->Unmap(0,nullptr);

		vbView_.BufferLocation = vertexBuffer_.GetResource()->GetGPUVirtualAddress();
		vbView_.SizeInBytes = vbSize;
		vbView_.StrideInBytes = sizeof(TextVertex);

		ibView_.BufferLocation = indexBuffer_.GetResource()->GetGPUVirtualAddress();
		ibView_.SizeInBytes = ibSize;
		ibView_.Format = DXGI_FORMAT_R16_UINT;

		// 登録済みフォントの GPU アトラスを事前生成しておく（描画中の生成を避ける）
		FontManager* fm = FontManager::GetInstance();
		for(const auto& entry : fm->GetFontList()){
			BitmapFont* font = fm->GetFont(entry.handle);
			if(font){
				EnsureAtlas(font);
			}
		}
	}

	void TextRenderSystem::Finalize(){
		layoutCache_.clear();
		constBuffer_.Finalize();

		for(auto& [font,gpu] : atlases_){
			if(gpu.created){
				Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(gpu.srv);
				gpu.resource.Finalize();
			}
		}
		atlases_.clear();

		vertexBuffer_.Finalize();
		indexBuffer_.Finalize();

		BaseRenderSystem::Finalize();
	}

	TextRenderSystem::FontAtlasGpu& TextRenderSystem::EnsureAtlas(BitmapFont* _font){
		auto& gpu = atlases_[_font];
		if(!gpu.created){
			CreateAtlasTexture(*_font,gpu);
		} else if(_font->IsAtlasDirty()){
			ReuploadAtlasTexture(*_font,gpu);
		}
		return gpu;
	}

	void TextRenderSystem::CreateAtlasTexture(BitmapFont& _font, FontAtlasGpu& _gpu){
		int w = _font.GetAtlasWidth();
		int h = _font.GetAtlasHeight();
		const auto& pixels = _font.GetAtlasPixels();

		auto device = Engine::GetInstance()->GetDxDevice()->device_;

		std::vector<uint8_t> rgba(w * h * 4);
		for(int i = 0; i < w * h; ++i){
			rgba[i * 4 + 0] = 255;
			rgba[i * 4 + 1] = 255;
			rgba[i * 4 + 2] = 255;
			rgba[i * 4 + 3] = pixels[i];
		}

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = w;
		texDesc.Height = h;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		device->CreateCommittedResource(
			&heapProps,D3D12_HEAP_FLAG_NONE,
			&texDesc,D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(_gpu.resource.GetResourceRef().GetAddressOf()));

		UINT64 uploadSize = 0;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		device->GetCopyableFootprints(&texDesc,0,1,0,&footprint,nullptr,nullptr,&uploadSize);

		DxResource uploadBuffer;
		uploadBuffer.CreateBufferResource(device,static_cast<size_t>(uploadSize));

		uint8_t* mapped = nullptr;
		uploadBuffer.GetResource()->Map(0,nullptr,reinterpret_cast<void**>(&mapped));
		for(UINT row = 0; row < static_cast<UINT>(h); ++row){
			std::memcpy(mapped + footprint.Offset + row * footprint.Footprint.RowPitch,
						rgba.data() + row * w * 4,
						w * 4);
		}
		uploadBuffer.GetResource()->Unmap(0,nullptr);

		DxCommand uploadCmd;
		uploadCmd.Initialize("textAtlasUpload","textAtlasUpload");
		uploadCmd.Close();
		uploadCmd.CommandReset();

		auto& cmdList = uploadCmd.GetCommandList();

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = _gpu.resource.GetResource().Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = uploadBuffer.GetResource().Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprint;

		cmdList->CopyTextureRegion(&dst,0,0,0,&src,nullptr);

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = _gpu.resource.GetResource().Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1,&barrier);

		uploadCmd.Close();
		uploadCmd.ExecuteCommandAndWait();
		uploadCmd.Finalize();

		uploadBuffer.Finalize();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;

		SRVEntry srvEntry(&_gpu.resource,srvDesc);
		_gpu.srv = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&srvEntry);
		_gpu.created = true;
		_gpu.uploadedWidth = w;
		_gpu.uploadedHeight = h;

		_font.ClearAtlasDirty();
	}

	void TextRenderSystem::ReuploadAtlasTexture(BitmapFont& _font, FontAtlasGpu& _gpu){
		int w = _font.GetAtlasWidth();
		int h = _font.GetAtlasHeight();

		bool sizeChanged = (w != _gpu.uploadedWidth || h != _gpu.uploadedHeight);

		if(sizeChanged){
			Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(_gpu.srv);
			_gpu.resource.Finalize();
			_gpu.created = false;
			CreateAtlasTexture(_font,_gpu);
			return;
		}

		const auto& pixels = _font.GetAtlasPixels();
		auto device = Engine::GetInstance()->GetDxDevice()->device_;

		std::vector<uint8_t> rgba(w * h * 4);
		for(int i = 0; i < w * h; ++i){
			rgba[i * 4 + 0] = 255;
			rgba[i * 4 + 1] = 255;
			rgba[i * 4 + 2] = 255;
			rgba[i * 4 + 3] = pixels[i];
		}

		D3D12_RESOURCE_DESC texDesc = _gpu.resource.GetResource()->GetDesc();

		UINT64 uploadSize = 0;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		device->GetCopyableFootprints(&texDesc,0,1,0,&footprint,nullptr,nullptr,&uploadSize);

		DxResource uploadBuffer;
		uploadBuffer.CreateBufferResource(device,static_cast<size_t>(uploadSize));

		uint8_t* mapped = nullptr;
		uploadBuffer.GetResource()->Map(0,nullptr,reinterpret_cast<void**>(&mapped));
		for(UINT row = 0; row < static_cast<UINT>(h); ++row){
			std::memcpy(mapped + footprint.Offset + row * footprint.Footprint.RowPitch,
						rgba.data() + row * w * 4,
						w * 4);
		}
		uploadBuffer.GetResource()->Unmap(0,nullptr);

		DxCommand uploadCmd;
		uploadCmd.Initialize("textAtlasReupload","textAtlasReupload");
		uploadCmd.Close();
		uploadCmd.CommandReset();

		auto& cmdList = uploadCmd.GetCommandList();

		D3D12_RESOURCE_BARRIER barrierToCopy = {};
		barrierToCopy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierToCopy.Transition.pResource = _gpu.resource.GetResource().Get();
		barrierToCopy.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierToCopy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrierToCopy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1,&barrierToCopy);

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = _gpu.resource.GetResource().Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = uploadBuffer.GetResource().Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprint;

		cmdList->CopyTextureRegion(&dst,0,0,0,&src,nullptr);

		D3D12_RESOURCE_BARRIER barrierToSrv = {};
		barrierToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierToSrv.Transition.pResource = _gpu.resource.GetResource().Get();
		barrierToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrierToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1,&barrierToSrv);

		uploadCmd.Close();
		uploadCmd.ExecuteCommandAndWait();
		uploadCmd.Finalize();

		uploadBuffer.Finalize();

		_font.ClearAtlasDirty();
	}

	void TextRenderSystem::DispatchRenderer(EntityHandle _entity){
		auto& texts = GetComponents<TextComponent>(_entity);

		for(auto& text : texts){
			if(!text.visible) continue;

			BitmapFont* font = FontManager::GetInstance()->GetFont(text.fontHandle);
			if(!font) continue;

			auto& layout = layoutCache_[_entity.uuid];
			layoutSystem_.UpdateLayout(*font,text,layout);

			if(!layout.quads.empty()){
				renderEntries_.push_back({text.renderPriority,font,&layout});
			}
		}
	}

	void TextRenderSystem::RebuildVertexBuffer(){
		quadCount_ = 0;
		for(auto& e : renderEntries_){
			quadCount_ += static_cast<uint32_t>(e.layout->quads.size());
		}
		if(quadCount_ == 0) return;

		if(quadCount_ > maxQuads_){
			vertexBuffer_.Finalize();
			indexBuffer_.Finalize();

			maxQuads_ = quadCount_ * 2;
			auto device = Engine::GetInstance()->GetDxDevice()->device_;

			vertexBuffer_.CreateBufferResource(device,maxQuads_ * kVerticesPerQuad * sizeof(TextVertex));
			indexBuffer_.CreateBufferResource(device,maxQuads_ * kIndicesPerQuad * sizeof(uint16_t));

			uint16_t* indexData = nullptr;
			indexBuffer_.GetResource()->Map(0,nullptr,reinterpret_cast<void**>(&indexData));
			for(uint32_t i = 0; i < maxQuads_; ++i){
				uint16_t base = static_cast<uint16_t>(i * 4);
				indexData[i * 6 + 0] = base + 0;
				indexData[i * 6 + 1] = base + 1;
				indexData[i * 6 + 2] = base + 2;
				indexData[i * 6 + 3] = base + 1;
				indexData[i * 6 + 4] = base + 3;
				indexData[i * 6 + 5] = base + 2;
			}
			indexBuffer_.GetResource()->Unmap(0,nullptr);

			vbView_.BufferLocation = vertexBuffer_.GetResource()->GetGPUVirtualAddress();
			vbView_.SizeInBytes = maxQuads_ * kVerticesPerQuad * sizeof(TextVertex);
			vbView_.StrideInBytes = sizeof(TextVertex);

			ibView_.BufferLocation = indexBuffer_.GetResource()->GetGPUVirtualAddress();
			ibView_.SizeInBytes = maxQuads_ * kIndicesPerQuad * sizeof(uint16_t);
			ibView_.Format = DXGI_FORMAT_R16_UINT;
		}

		TextVertex* vtx = nullptr;
		vertexBuffer_.GetResource()->Map(0,nullptr,reinterpret_cast<void**>(&vtx));

		uint32_t qi = 0;
		for(auto& e : renderEntries_){
			for(auto& q : e.layout->quads){
				vtx[qi * 4 + 0] = {{q.posMin[X],q.posMin[Y],0.0f,1.0f},q.uvMin,q.color};
				vtx[qi * 4 + 1] = {{q.posMax[X],q.posMin[Y],0.0f,1.0f},{q.uvMax[X],q.uvMin[Y]},q.color};
				vtx[qi * 4 + 2] = {{q.posMin[X],q.posMax[Y],0.0f,1.0f},{q.uvMin[X],q.uvMax[Y]},q.color};
				vtx[qi * 4 + 3] = {{q.posMax[X],q.posMax[Y],0.0f,1.0f},q.uvMax,q.color};
				++qi;
			}
		}
		vertexBuffer_.GetResource()->Unmap(0,nullptr);
	}

	void TextRenderSystem::Rendering(){
		// 使用される全フォントのアトラスを準備（ダーティなら再アップロード）
		for(auto& e : renderEntries_){
			EnsureAtlas(e.font);
		}

		// 描画順(priority)を主、同 priority 内はフォントでグループ化して SRV 切替を減らす
		std::sort(renderEntries_.begin(),renderEntries_.end(),
				  [](const RenderEntry& a,const RenderEntry& b){
					  if(a.priority != b.priority) return a.priority < b.priority;
					  return a.font < b.font;
				  });

		WinApp* window = Engine::GetInstance()->GetWinApp();
		viewPortMat_ = MakeMatrix4x4::Orthographic(
			0,0,
			static_cast<float>(window->GetWidth()),
			static_cast<float>(window->GetHeight()),
			Config::Rendering::kMinDepth,
			Config::Camera::kDefaultFarClip);

		constBuffer_->vpMat = viewPortMat_;
		constBuffer_.ConvertToBuffer();

		RebuildVertexBuffer();
		if(quadCount_ == 0){
			renderEntries_.clear();
			return;
		}

		auto& cmdList = dxCommand_->GetCommandList();

		StartRender();

		cmdList->SetGraphicsRootSignature(pso_->rootSignature.Get());
		cmdList->SetPipelineState(pso_->pipelineState.Get());

		constBuffer_.SetForRootParameter(cmdList,0);

		cmdList->IASetVertexBuffers(0,1,&vbView_);
		cmdList->IASetIndexBuffer(&ibView_);

		// 同一フォントの連続区間をまとめて描画する
		uint32_t quadCursor = 0;
		size_t i = 0;
		while(i < renderEntries_.size()){
			BitmapFont* runFont = renderEntries_[i].font;
			uint32_t runStartQuad = quadCursor;
			uint32_t runQuads = 0;

			while(i < renderEntries_.size() && renderEntries_[i].font == runFont){
				runQuads += static_cast<uint32_t>(renderEntries_[i].layout->quads.size());
				++i;
			}
			quadCursor += runQuads;

			if(runQuads == 0) continue;

			auto atlasItr = atlases_.find(runFont);
			if(atlasItr == atlases_.end() || !atlasItr->second.created) continue;

			cmdList->SetGraphicsRootDescriptorTable(1,atlasItr->second.srv.GetGpuHandle());
			cmdList->DrawIndexedInstanced(runQuads * kIndicesPerQuad,1,runStartQuad * kIndicesPerQuad,0,0);
		}

		renderEntries_.clear();
	}

	void TextRenderSystem::StartRender(){
		auto& cmdList = dxCommand_->GetCommandList();
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
		cmdList->SetDescriptorHeaps(1,ppHeaps);
	}

	bool TextRenderSystem::ShouldSkipRender() const{
		return renderEntries_.empty();
	}

	void TextRenderSystem::CreatePSO(){
		ShaderManager* sm = ShaderManager::GetInstance();

		if(sm->IsRegisteredPipelineStateObj("Text_Normal")){
			pso_ = sm->GetPipelineStateObj("Text_Normal");
			return;
		}

		sm->LoadShader("Text.VS");
		sm->LoadShader("Text.PS",kShaderDirectory,L"ps_6_0");

		ShaderInformation info{};
		info.vsKey = "Text.VS";
		info.psKey = "Text.PS";

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.MinLOD = 0;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		info.pushBackSamplerDesc(sampler);

		D3D12_ROOT_PARAMETER rootParams[2] = {};
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].Descriptor.ShaderRegister = 0;
		info.pushBackRootParameter(rootParams[0]);

		D3D12_DESCRIPTOR_RANGE range = {};
		range.BaseShaderRegister = 0;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		size_t idx = info.pushBackRootParameter(rootParams[1]);
		info.SetDescriptorRange2Parameter(&range,1,idx);

		D3D12_INPUT_ELEMENT_DESC inputDescs[3] = {};
		inputDescs[0].SemanticName = "POSITION";
		inputDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		info.pushBackInputElementDesc(inputDescs[0]);

		inputDescs[1].SemanticName = "TEXCOORD";
		inputDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		info.pushBackInputElementDesc(inputDescs[1]);

		inputDescs[2].SemanticName = "COLOR";
		inputDescs[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		info.pushBackInputElementDesc(inputDescs[2]);

		info.changeCullMode(D3D12_CULL_MODE_NONE);
		info.blendMode_ = BlendMode::Normal;

		pso_ = sm->CreatePso("Text_Normal",info,Engine::GetInstance()->GetDxDevice()->device_);
	}

} // namespace OriGine
