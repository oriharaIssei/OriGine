#include "directX12/buffer/Object3dMesh.h"

#include <System.h>

void TextureObject3dMesh::Create(UINT vertexSize,UINT indexSize) {
	if(vertexSize != 0) {
		UINT vertDataSize = sizeof(TextureVertexData);

		vertBuff.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),vertDataSize * vertexSize);
		vbView.BufferLocation = vertBuff.getResource()->GetGPUVirtualAddress();
		vbView.SizeInBytes = vertDataSize * vertexSize;
		vbView.StrideInBytes = vertDataSize;
		vertBuff.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&vertData));
	}

	if(indexSize != 0) {
		indexBuff.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),sizeof(uint32_t) * indexSize);
		ibView.BufferLocation = indexBuff.getResource()->GetGPUVirtualAddress();
		ibView.SizeInBytes = sizeof(uint32_t) * indexSize;
		ibView.Format = DXGI_FORMAT_R32_UINT;
		indexBuff.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&indexData));
	}
}

void PrimitiveObject3dMesh::Create(UINT vertexSize,UINT indexSize) {
	UINT vertDataSize = sizeof(PrimitiveVertexData);

	if(vertexSize != 0) {
		vertBuff.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),sizeof(PrimitiveVertexData) * vertexSize);

		vertBuff.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&vertData));

		vbView.BufferLocation = vertBuff.getResource()->GetGPUVirtualAddress();
		vbView.SizeInBytes = vertDataSize * vertexSize;
		vbView.StrideInBytes = vertDataSize;
	}

	if(indexSize != 0) {
		indexBuff.CreateBufferResource(System::getInstance()->getDxDevice()->getDevice(),sizeof(uint32_t) * indexSize);
		ibView.BufferLocation = indexBuff.getResource()->GetGPUVirtualAddress();
		ibView.SizeInBytes = sizeof(uint32_t) * indexSize;
		ibView.Format = DXGI_FORMAT_R32_UINT;
		indexBuff.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&indexData));
	}
}