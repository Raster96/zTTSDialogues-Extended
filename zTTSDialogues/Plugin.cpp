// This file added in headers queue
// File: "Sources.h"
#include "resource.h"

namespace GOTHIC_ENGINE {

	void ModifyVoiceSelectionMenu();
	
	HOOK Ivk_zCView_DialogMessageCXY PATCH(&zCView::DialogMessageCXY, &zCView::DialogMessageCXY_Union);
	void zCView::DialogMessageCXY_Union(zSTRING const& name, zSTRING const& text, float time, zCOLOR& color)
	{
		if (ShouldRead)
			time = text.Length() * 125.0f;

		THISCALL(Ivk_zCView_DialogMessageCXY)(name, text, time, color);
	}

	HOOK Ivk_oCNpc_StopAllVoices AS(&oCNpc::StopAllVoices, &oCNpc::StopAllVoices_Union);
	void oCNpc::StopAllVoices_Union()
	{
		THISCALL(Ivk_oCNpc_StopAllVoices)();

		if (this == SAPI.GetCurrentSpeaker())
			SAPI.StopReading();
	}

	HOOK Ivk_oCNpc_UpdateNextVoice PATCH(&oCNpc::UpdateNextVoice, &oCNpc::UpdateNextVoice_Union);
	int oCNpc::UpdateNextVoice_Union()
	{
		if (voiceIndex >= listOfVoiceHandles.GetNum())
			return THISCALL(Ivk_oCNpc_UpdateNextVoice)();

		const auto currentHandle = listOfVoiceHandles.GetSafe(voiceIndex);

		if (currentHandle == SAPI.GetCurrentHandle())
		{
			if (SAPI.IsDoneReading())
				listOfVoiceHandles.RemoveIndex(voiceIndex);
			else
				voiceIndex += 1;

			if (voiceIndex >= listOfVoiceHandles.GetNum())
			{
				voiceIndex = 0;
				return 1;
			}
		}

		return THISCALL(Ivk_oCNpc_UpdateNextVoice)();
	}

	HOOK Ivk_oCNpc_EV_PlaySound PATCH(&oCNpc::EV_PlaySound, &oCNpc::EV_PlaySound_Union);
	int oCNpc::EV_PlaySound_Union(oCMsgConversation* msg)
	{
		if (this != player && this != oCInformationManager::GetInformationManager().Npc)
			return THISCALL(Ivk_oCNpc_EV_PlaySound)(msg);

		if (msg->handle == 0)
		{
			ShouldRead = false;
			if (vdf_fexists(msg->name.ToChar(), VDF_VIRTUAL | VDF_PHYSICAL) == 0)
				ShouldRead = true;
			else
			{
				if (ReplaceAllDialogues)
					ShouldRead = true;
			}
		}

		int result = THISCALL(Ivk_oCNpc_EV_PlaySound)(msg);

		if (zsound->IsSoundActive(msg->handle) && ShouldRead)
			zsound->StopSound(msg->handle);

		if (ShouldRead) {
			int duration = msg->text.Length() * 125;
			msg->f_no = duration;
			SAPI.Read(string(msg->text), this, msg->handle);
			ShouldRead = false;
		}

		if (SAPI.GetCurrentHandle() == msg->handle && SAPI.IsDoneReading()) {
			ogame->GetWorld()->csPlayer->StopAllOutputUnits(this);
			SAPI.StopReading();
		}

		return result;
	}

	void UpdateSettings() {
		VoiceVolume = zoptions->ReadInt("zTTSDialogues", "VoiceVolume", VoiceVolume);
		if (VoiceVolume < 0)
			VoiceVolume = static_cast<int>(zoptions->ReadReal("SOUND", "soundVolume", 1.0f) * 200.0f);

		VoiceRate = zoptions->ReadInt("zTTSDialogues", "VoiceRate", VoiceRate);
		std::clamp(VoiceRate, 0, 20);
		VoiceRate -= 10;

		ReplaceAllDialogues = zoptions->ReadBool("zTTSDialogues", "ReplaceAllDialogues", ReplaceAllDialogues);

		// Voice selection: 0 = Auto (by language), 1+ = specific voice index
		SelectedVoiceIndex = zoptions->ReadInt("zTTSDialogues", "SelectedVoiceIndex", 0);

		Language = zoptions->ReadInt("zTTSDialogues", "Language", Union.GetSystemLanguage());

		switch (Language)
		{
		case Lang_Rus:
			//RUSSIAN
			LanguageID = L"419";
			CodePage = ANSI_CODEPAGE_CYRILLIC;
			break;

		case Lang_Ger:
			//GERMAN
			LanguageID = L"407";
			CodePage = ANSI_COPEDAGE_NORTHORWESTERN_EUROPEAN;
			break;

		case Lang_Pol:
			//POLISH
			LanguageID = L"415";
			CodePage = ANSI_COPEDAGE_CENTRALOREASTERN_EUROPEAN;
			break;

		case Lang_Rou:
			//ROMANIAN
			LanguageID = L"418";
			CodePage = ANSI_COPEDAGE_CENTRALOREASTERN_EUROPEAN;
			break;

		case Lang_Ita:
			//ITALIAN
			LanguageID = L"410";
			CodePage = ANSI_COPEDAGE_NORTHORWESTERN_EUROPEAN;
			break;

		case Lang_Cze:
			//CZECH
			LanguageID = L"405";
			CodePage = ANSI_COPEDAGE_CENTRALOREASTERN_EUROPEAN;
			break;

		case Lang_Esp:
			//SPANISH
			LanguageID = L"C0A";
			CodePage = ANSI_COPEDAGE_NORTHORWESTERN_EUROPEAN;
			break;
		case Lang_Other:
			//OTHER
			LanguageID = string(zoptions->ReadString("zTTSDialogues", "LanguageID", "409")).AToW();
			CodePage = zoptions->ReadInt("zTTSDialogues", "CodePage", 1252);
			break;
		case Lang_Eng:
		default:
			//US ENGLISH
			LanguageID = L"409";
			CodePage = ANSI_COPEDAGE_NORTHORWESTERN_EUROPEAN;
			break;
		}

		SAPI.ReloadSettings();
	}

	void Game_Entry() {
	}

	void Game_Init() {
		UpdateSettings();
		ModifyVoiceSelectionMenu();
	}

	void Game_Exit() {
	}

	void Game_PreLoop() {
	}

	void Game_Loop() {
	}

	void Game_PostLoop() {
	}

	int PreviousVoiceIndex = -1;
	int PreviousVoiceRate = -1;
	bool VoiceTestPending = false;

	void Game_MenuLoop() {

		zCMenu* activeMenu = zCMenu::GetActive();
		if (activeMenu) {
			zSTRING menuName = activeMenu->GetName();
			menuName.Upper();
			
			if (menuName == "MENU_OPT_ZTTSDIALOGUES" || menuName == "ZTTSDIALOGUES:MENU_OPT_ZTTSDIALOGUES") {
				ModifyVoiceSelectionMenu();
				
				int currentVoiceIndex = zoptions->ReadInt("zTTSDialogues", "SelectedVoiceIndex", 0);
				int currentVoiceRate = zoptions->ReadInt("zTTSDialogues", "VoiceRate", 10);
				
				if (PreviousVoiceIndex != -1 && (currentVoiceIndex != PreviousVoiceIndex || currentVoiceRate != PreviousVoiceRate)) {
					VoiceTestPending = true;
					PreviousVoiceIndex = currentVoiceIndex;
					PreviousVoiceRate = currentVoiceRate;
				} else if (PreviousVoiceIndex == -1) {
					PreviousVoiceIndex = currentVoiceIndex;
					PreviousVoiceRate = currentVoiceRate;
				}
				
				if (VoiceTestPending) {
					VoiceTestPending = false;
					
					UpdateSettings();
					
					string testText;
					
					switch (Language) {
						case Lang_Rus:
							// Russian: "Выберите голос для озвучки диалогов"
							testText = string("\xC2\xFB\xE1\xE5\xF0\xE8\xF2\xE5 \xE3\xEE\xEB\xEE\xF1 \xE4\xEB\xFF \xEE\xE7\xE2\xF3\xF7\xEA\xE8 \xE4\xE8\xE0\xEB\xEE\xE3\xEE\xE2");
							break;
						case Lang_Ger:
							// German: "Wählen Sie die Stimme für Dialoge"
							testText = "W\xE4hlen Sie die Stimme f\xFCr Dialoge";
							break;
						case Lang_Pol:
							// Polish: "Wybierz głos do odczytywania dialogów"
							testText = "Wybierz g\xB3os do odczytywania dialog\xF3w";
							break;
						case Lang_Rou:
							// Romanian: "Selectați vocea pentru naratiunea dialogurilor"
							testText = "Selecta\xFEi vocea pentru naratiunea dialogurilor";
							break;
						case Lang_Ita:
							// Italian: "Seleziona la voce per la narrazione del dialogo"
							testText = "Seleziona la voce per la narrazione del dialogo";
							break;
						case Lang_Cze:
							// Czech: "Vyberte hlas pro vyprávění dialogu"
							testText = "Vyberte hlas pro vypr\xE1v\xEC\xED dialogu";
							break;
						case Lang_Esp:
							// Spanish: "Seleccione la voz para la narración del diálogo"
							testText = "Seleccione la voz para la narraci\xF3n del di\xE1logo";
							break;
						case Lang_Eng:
						default:
							testText = "Select voice for dialogue narration";
							break;
					}
					SAPI.Read(testText, nullptr, 0);
				}
			}
		}
	}

	TSaveLoadGameInfo& SaveLoadGameInfo = UnionCore::SaveLoadGameInfo;

	void Game_SaveBegin() {
	}

	void Game_SaveEnd() {
	}

	void LoadBegin() {
	}

	void LoadEnd() {
	}

	void Game_LoadBegin_NewGame() {
		LoadBegin();
	}

	void Game_LoadEnd_NewGame() {
		LoadEnd();
	}

	void Game_LoadBegin_SaveGame() {
		LoadBegin();
	}

	void Game_LoadEnd_SaveGame() {
		LoadEnd();
	}

	void Game_LoadBegin_ChangeLevel() {
		LoadBegin();
	}

	void Game_LoadEnd_ChangeLevel() {
		LoadEnd();
	}

	void Game_LoadBegin_Trigger() {
	}

	void Game_LoadEnd_Trigger() {
	}

	void Game_Pause() {
	}

	void Game_Unpause() {
	}

	void Game_DefineExternals() {
	}
	
	bool VoiceMenuModified = false;
	
	void ModifyVoiceSelectionMenu() {
		if (VoiceMenuModified) return;
		
		zCMenuItem* voiceItem = zCMenuItem::GetByName("MENUITEM_OPT_ZTTSDIALOGUES_SELECTEDVOICEINDEX_CHOICE");
		if (!voiceItem) {
			voiceItem = zCMenuItem::GetByName("ZTTSDIALOGUES:MENUITEM_OPT_ZTTSDIALOGUES_SELECTEDVOICEINDEX_CHOICE");
			if (!voiceItem) return;
		}
		
		Array<VoiceInfo> voices = SAPI.GetAvailableVoices();
		if (voices.GetNum() == 0) return;
		
		string choiceText = "Auto";  // 0 = Auto
		
		int maxVoices = voices.GetNum() > 50 ? 50 : voices.GetNum();
		for (int i = 0; i < maxVoices; i++) {
			string voiceName = string(voices[i].name.WToA(CodePage));
			
			if (voiceName.Length() > 80) {
				voiceName = voiceName.Copy(0, 77) + "...";
			}
			
			choiceText += "|" + voiceName;
		}
		
		voiceItem->SetText(choiceText, 0, 0);
		
		VoiceMenuModified = true;
	}

	void Game_ApplyOptions() {
		UpdateSettings();
	}

	/*
	Functions call order on Game initialization:
	  - Game_Entry           * Gothic entry point
	  - Game_DefineExternals * Define external script functions
	  - Game_Init            * After DAT files init

	Functions call order on Change level:
	  - Game_LoadBegin_Trigger     * Entry in trigger
	  - Game_LoadEnd_Trigger       *
	  - Game_Loop                  * Frame call window
	  - Game_LoadBegin_ChangeLevel * Load begin
	  - Game_SaveBegin             * Save previous level information
	  - Game_SaveEnd               *
	  - Game_LoadEnd_ChangeLevel   *

	Functions call order on Save game:
	  - Game_Pause     * Open menu
	  - Game_Unpause   * Click on save
	  - Game_Loop      * Frame call window
	  - Game_SaveBegin * Save begin
	  - Game_SaveEnd   *

	Functions call order on Load game:
	  - Game_Pause              * Open menu
	  - Game_Unpause            * Click on load
	  - Game_LoadBegin_SaveGame * Load begin
	  - Game_LoadEnd_SaveGame   *
	*/

#define AppDefault True
	CApplication* lpApplication = !CHECK_THIS_ENGINE ? Null : CApplication::CreateRefApplication(
		Enabled(AppDefault) Game_Entry,
		Enabled(AppDefault) Game_Init,
		Enabled(AppDefault) Game_Exit,
		Enabled(AppDefault) Game_PreLoop,
		Enabled(AppDefault) Game_Loop,
		Enabled(AppDefault) Game_PostLoop,
		Enabled(AppDefault) Game_MenuLoop,
		Enabled(AppDefault) Game_SaveBegin,
		Enabled(AppDefault) Game_SaveEnd,
		Enabled(AppDefault) Game_LoadBegin_NewGame,
		Enabled(AppDefault) Game_LoadEnd_NewGame,
		Enabled(AppDefault) Game_LoadBegin_SaveGame,
		Enabled(AppDefault) Game_LoadEnd_SaveGame,
		Enabled(AppDefault) Game_LoadBegin_ChangeLevel,
		Enabled(AppDefault) Game_LoadEnd_ChangeLevel,
		Enabled(AppDefault) Game_LoadBegin_Trigger,
		Enabled(AppDefault) Game_LoadEnd_Trigger,
		Enabled(AppDefault) Game_Pause,
		Enabled(AppDefault) Game_Unpause,
		Enabled(AppDefault) Game_DefineExternals,
		Enabled(AppDefault) Game_ApplyOptions
	);
}