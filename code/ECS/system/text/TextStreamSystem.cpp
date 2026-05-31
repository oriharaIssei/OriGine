#include "ECS/system/text/TextStreamSystem.h"

#include "Engine.h"
#include "deltaTime/DeltaTimer.h"

#include "ECS/component/text/TextStreamComponent.h"
#include "ECS/component/text/TextComponent.h"

#include <functional>
#include <string>

using namespace OriGine;

namespace {
	// UTF-8 文字列のコードポイント数を数える
	size_t CountCodepoints(const std::string& _str){
		size_t n = 0;
		for(size_t i = 0; i < _str.size();){
			unsigned char c = static_cast<unsigned char>(_str[i]);
			i += (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
			++n;
		}
		return n;
	}
} // namespace

TextStreamSystem::TextStreamSystem(): ISystem(SystemCategory::Effect){}
TextStreamSystem::~TextStreamSystem(){}

void TextStreamSystem::Initialize(){}

void TextStreamSystem::Finalize(){}

void OriGine::TextStreamSystem::UpdateEntity(EntityHandle _handle){
	float dt = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

	Entity* entity = GetEntity(_handle);

	if(!entity || !entity->IsAlive()){
		return;
	}

	TextComponent* text = GetComponent<TextComponent>(_handle);
	if(!text){
		return;
	}

	TextStreamComponent* stream = GetComponent<TextStreamComponent>(_handle);
	if(!stream){
		return;
	}


	// TextComponent.text が変化したら自動的に先頭から再生し直す
	size_t hash = std::hash<std::string>{}(text->text);
	if(hash != stream->textHash){
		stream->textHash = hash;
		stream->Reset();
	}

	const int total = static_cast<int>(CountCodepoints(text->text));

	if(stream->playing){
		if(stream->elapsedDelay < stream->startDelay){
			stream->elapsedDelay += dt;
		} else{
			stream->revealed += stream->charsPerSecond * dt;
		}
	}

	if(stream->revealed < 0.0f){
		stream->revealed = 0.0f;
	}

	// revealed が巨大値（Skip）でも安全に総数へクランプする
	int show = (stream->revealed >= static_cast<float>(total))
		? total
		: static_cast<int>(stream->revealed);

	if(show >= total){
		show        = total;
		stream->finished = true;
		if(stream->loop && stream->playing){
			// ループ：先頭へ巻き戻す
			stream->revealed     = 0.0f;
			stream->elapsedDelay = 0.0f;
			stream->finished     = false;
			show            = 0;
		}
	}

	// 表示文字数が変わったときだけ TextComponent を更新（無駄な再レイアウトを避ける）
	if(show != stream->lastApplied){
		text->visibleCharCount = show;
		text->dirty            = true;
		stream->lastApplied         = show;
	}
}
