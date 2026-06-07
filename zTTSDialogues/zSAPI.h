// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {
	int VoiceRate = 11;
	int VoiceVolume = -1;
	bool ReplaceAllDialogues = false;
	bool ShouldRead = false;
	int Language = 1;
	wstring LanguageID;
	uint32 CodePage;
	int SelectedVoiceIndex = 0; // 0 = Auto (select by language), 1+ = specific voice index

	// Voice information structure
	struct VoiceInfo {
		wstring id;
		wstring name;
		wstring language;
		wstring gender;
	};

	class zSAPI {
	public:
		zSAPI();
		~zSAPI();

	public:
		void Read(const string& str, oCNpc* speaker, int handle);
		void StopReading();
		bool IsDoneReading();
		oCNpc* GetCurrentSpeaker();
		int GetCurrentHandle();
		void ReloadSettings();
		Array<VoiceInfo> GetAvailableVoices();

	private:
		void Init();
		void Release();
		void EnumerateVoices();
		ISpVoice* voice;
		oCNpc* currentspeaker = nullptr;
		int currenthandle = 0;
		Array<VoiceInfo> availableVoices;
	};
}