namespace HLMovement {
	static std::string GetSpeechPath(const std::string& file) {
		return "data/sound/hl/" + file;
	}

	std::vector<NyaAudio::NyaSound> aSoundCache;
	void PlayGameSound(const std::string& path, float volume) {
		NyaAudio::Init(pDeviceD3d->hWnd);

		auto sound = NyaAudio::LoadFile(GetSpeechPath(path).c_str());
		if (!sound) return;
		NyaAudio::SetVolume(sound, volume * fSoundVolume);
		NyaAudio::Play(sound);
		aSoundCache.push_back(sound);
	}
}