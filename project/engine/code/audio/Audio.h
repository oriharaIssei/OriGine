#pragma once

#include <xaudio2.h>

#include <memory>
#include <wrl.h>

#include <stdint.h>
#include <string>

struct ChunkHeader{
	char id[4];
	int32_t size;
};
struct RiffHeader{
	ChunkHeader chunk;
	char type[4];
};
struct FormatChunk{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData{
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	uint32_t bufferSize;
};

class Audio{
public:
	static void StaticInitialize();
	static void StaticFinalize();

	Audio(){}
	~Audio(){}

	void Initialize(const std::string& fileName);

	void PlayTrigger();
	void PlayLoop();

	void Pause();

	void Finalize();

private:
	SoundData LoadWave(const std::string& fileName);
	void SoundUnLoad();
private:
	static Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	static IXAudio2MasteringVoice* masterVoice_;

	SoundData soundData_;
	IXAudio2SourceVoice* pSourceVoice_ = nullptr;

	bool isLoop_ = false;
};