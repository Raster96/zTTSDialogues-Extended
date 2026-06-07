// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	zSAPI::zSAPI() {
		EnumerateVoices();
	}

	zSAPI::~zSAPI() {
		Release();
	}

	void zSAPI::Release() {
		if (voice)
		{
			voice->Release();
			voice = NULL;
		}
	}

	void zSAPI::Init() {
		HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&voice);

		if (!SUCCEEDED(hr))
		{
			CMessageA::Error("Could not initialize voice", "zTTSDialogues Error");
			exit(0);
		}
	}

	void zSAPI::EnumerateVoices() {
		availableVoices.Clear();

		// Enumerate OneCore voices (Windows 10+)
		CComPtr<IEnumSpObjectTokens> cpEnum;
		ULONG ulCount = 0;

		// Try OneCore voices first
		HRESULT hr = SpEnumTokens(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices", NULL, NULL, &cpEnum);
		if (SUCCEEDED(hr) && cpEnum) {
			cpEnum->GetCount(&ulCount);
			
			for (ULONG i = 0; i < ulCount; i++) {
				CComPtr<ISpObjectToken> cpToken;
				if (SUCCEEDED(cpEnum->Next(1, &cpToken, NULL))) {
					VoiceInfo info;
					
					// Get voice ID
					LPWSTR pszId = NULL;
					if (SUCCEEDED(cpToken->GetId(&pszId))) {
						info.id = pszId;
						CoTaskMemFree(pszId);
					}
					
					// Get voice name
					LPWSTR pszName = NULL;
					if (SUCCEEDED(cpToken->GetStringValue(NULL, &pszName))) {
						info.name = pszName;
						CoTaskMemFree(pszName);
					}
					
					// Get language
					LPWSTR pszLang = NULL;
					if (SUCCEEDED(cpToken->GetStringValue(L"Language", &pszLang))) {
						info.language = pszLang;
						CoTaskMemFree(pszLang);
					}
					
					// Get gender
					LPWSTR pszGender = NULL;
					if (SUCCEEDED(cpToken->GetStringValue(L"Gender", &pszGender))) {
						info.gender = pszGender;
						CoTaskMemFree(pszGender);
					}
					
					availableVoices.Insert(info);
				}
			}
		}

		// Also try SAPI5 Desktop voices as fallback
		cpEnum.Release();
		hr = SpEnumTokens(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices", NULL, NULL, &cpEnum);
		if (SUCCEEDED(hr) && cpEnum) {
			cpEnum->GetCount(&ulCount);
			
			for (ULONG i = 0; i < ulCount; i++) {
				CComPtr<ISpObjectToken> cpToken;
				if (SUCCEEDED(cpEnum->Next(1, &cpToken, NULL))) {
					VoiceInfo info;
					
					// Get voice ID
					LPWSTR pszId = NULL;
					if (SUCCEEDED(cpToken->GetId(&pszId))) {
						info.id = pszId;
						
						// Check if this voice is already in the list (avoid duplicates)
						bool duplicate = false;
						for (int j = 0; j < availableVoices.GetNum(); j++) {
							if (availableVoices[j].id == info.id) {
								duplicate = true;
								break;
							}
						}
						
						if (!duplicate) {
							CoTaskMemFree(pszId);
							
							// Get voice name
							LPWSTR pszName = NULL;
							if (SUCCEEDED(cpToken->GetStringValue(NULL, &pszName))) {
								info.name = pszName;
								CoTaskMemFree(pszName);
							}
							
							// Get language
							LPWSTR pszLang = NULL;
							if (SUCCEEDED(cpToken->GetStringValue(L"Language", &pszLang))) {
								info.language = pszLang;
								CoTaskMemFree(pszLang);
							}
							
							// Get gender
							LPWSTR pszGender = NULL;
							if (SUCCEEDED(cpToken->GetStringValue(L"Gender", &pszGender))) {
								info.gender = pszGender;
								CoTaskMemFree(pszGender);
							}
							
							availableVoices.Insert(info);
						}
						else {
							CoTaskMemFree(pszId);
						}
					}
				}
			}
		}
	}

	Array<VoiceInfo> zSAPI::GetAvailableVoices() {
		return availableVoices;
	}

	bool zSAPI::IsDoneReading() {
		if (!voice)
			return true;

		SPVOICESTATUS status;
		voice->GetStatus(&status, NULL);
		if (status.dwRunningState == SPRS_DONE)
			return true;

		return false;
	}

	void zSAPI::ReloadSettings() {
		if (!voice)
			Init();

		voice->SetVolume(VoiceVolume);
		voice->SetRate(VoiceRate);

		CComPtr<IEnumSpObjectTokens> cpEnum;
		CComPtr<ISpObjectToken> cpToken;

		// If SelectedVoiceIndex is 0, auto-select by language and gender
		// Otherwise, use the specific voice by index
		if (SelectedVoiceIndex == 0) {
			// Auto mode - try to find a voice matching language
			// First try OneCore voices
			SpEnumTokens(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices", L"Language = " + LanguageID, NULL, &cpEnum);
			cpEnum->Next(1, &cpToken, NULL);
			
			// If no OneCore voice found, try SAPI5 Desktop voices
			if (!cpToken) {
				cpEnum.Release();
				SpEnumTokens(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices", L"Language = " + LanguageID, NULL, &cpEnum);
				cpEnum->Next(1, &cpToken, NULL);
			}
			
			if (cpToken) {
				voice->SetVoice(cpToken);
			}
		}
		else {
			// Specific voice selected (index 1+)
			int voiceArrayIndex = SelectedVoiceIndex - 1; // Convert from 1-based to 0-based
			if (voiceArrayIndex >= 0 && voiceArrayIndex < availableVoices.GetNum()) {
				VoiceInfo& selectedVoice = availableVoices[voiceArrayIndex];
				
				// Search for the voice by ID in both OneCore and SAPI5 registries
				// Try OneCore first
				SpEnumTokens(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices", NULL, NULL, &cpEnum);
				if (cpEnum) {
					ULONG ulCount = 0;
					cpEnum->GetCount(&ulCount);
					
					for (ULONG i = 0; i < ulCount; i++) {
						CComPtr<ISpObjectToken> tempToken;
						if (SUCCEEDED(cpEnum->Next(1, &tempToken, NULL))) {
							LPWSTR pszId = NULL;
							if (SUCCEEDED(tempToken->GetId(&pszId))) {
								if (selectedVoice.id == pszId) {
									cpToken = tempToken;
									CoTaskMemFree(pszId);
									break;
								}
								CoTaskMemFree(pszId);
							}
						}
					}
				}
				
				// If not found in OneCore, try SAPI5 Desktop
				if (!cpToken) {
					cpEnum.Release();
					SpEnumTokens(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices", NULL, NULL, &cpEnum);
					if (cpEnum) {
						ULONG ulCount = 0;
						cpEnum->GetCount(&ulCount);
						
						for (ULONG i = 0; i < ulCount; i++) {
							CComPtr<ISpObjectToken> tempToken;
							if (SUCCEEDED(cpEnum->Next(1, &tempToken, NULL))) {
								LPWSTR pszId = NULL;
								if (SUCCEEDED(tempToken->GetId(&pszId))) {
									if (selectedVoice.id == pszId) {
										cpToken = tempToken;
										CoTaskMemFree(pszId);
										break;
									}
									CoTaskMemFree(pszId);
								}
							}
						}
					}
				}
				
				if (cpToken) {
					voice->SetVoice(cpToken);
				}
			}
		}

		cpEnum.Release();
		cpToken.Release();
	}

	oCNpc* zSAPI::GetCurrentSpeaker() {
		return currentspeaker;
	}

	int zSAPI::GetCurrentHandle() {
		return currenthandle;
	}

	void zSAPI::Read(const string& str, oCNpc* speaker, int handle) {
		ReloadSettings();

		currenthandle = handle;
		currentspeaker = speaker;

		DWORD flags = SPF_ASYNC | SPF_IS_NOT_XML | SPF_PURGEBEFORESPEAK;
		voice->Speak(str.AToW(CodePage), flags, NULL);
	}

	void zSAPI::StopReading() {
		Release();

		currentspeaker = nullptr;
		currenthandle = 0;
	}
}