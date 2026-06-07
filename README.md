# zTTSDialogues - Enhanced Voice Selection Fork

This fork introduces **enhanced voice selection** capabilities, making it easier to use custom SAPI 5 voices:
<img width="839" height="310" alt="image" src="https://github.com/user-attachments/assets/de1a54fb-7667-46b5-9eda-bde0df8cfa5d" />
https://www.youtube.com/watch?v=IOuVb_hIJX0

### Key Improvements

**Direct Voice Selection by Name**
- **Before:** Could only select by gender (Male/Female), limiting options to default system voices
- **After:** Full list of all installed SAPI 5 voices displayed by name in the options menu
- Choose from any installed voice: Microsoft voices, custom voices (RHVoice, IVONA, etc.)

**Wide Voice Selection Menu**
- Extended choice box (6400 units wide) displays complete voice names
- No more truncated names - see full voice identification like "Microsoft Adam - Polish (Poland)"
- Positioned for optimal visibility within the menu frame

**Instant Voice Preview**
- Test voices immediately when changing selection (no need to exit menu)
- Also previews speed changes in real-time
- Plays localized sample text in the selected voice language

**Easy Installation of Additional Voices**
- Simply install any SAPI 5 compatible voice on Windows
- Voices automatically appear in the selection menu
- Perfect for installing high-quality voices like RHVoice, IVONA, or regional voices

### Technical Changes

- Replaced `Gender` option with `SelectedVoiceIndex` (0 = Auto, 1-50 = specific voice)
- Modified menu system to display full voice names instead of numeric indices
- Added `ModifyVoiceSelectionMenu()` function to dynamically populate voice names
- Implemented real-time voice preview in `Game_MenuLoop()`
- Uses Windows-1250 encoding for proper display of Polish and other Central European characters

---

# Original zTTSDialogues README

# zTTSDialogues
With this plugin unvoiced, or optionally all in-game dialogues, will be read with Windows Narrator.

### Default config
```
[ZTTSDIALOGUES]
VoiceVolume=-1
;
; Volume of the TTS voice: -1 = autodetect

VoiceRate=11
; 
; Speed of the text reading (min: 0, max: 20)

ReplaceAllDialogues=0
;
; Replace all original dialogues with TTS (1) or only dialogues without voice files (0)

Language=2
;
; Language of the TTS
; 0 - Other, need to specify LanguageID and CodePage
;;; To find CodePage: https://learn.microsoft.com/pl-pl/windows/win32/intl/code-page-identifiers
;;; To find LanguageID: https://documentation.help/Microsoft-Speech-Platform-SDK-11/c494fcf6-5053-42ad-9528-9948c2d93855.htm
;;; Default values for English (United States)
;;; CodePage=1252
;;; LanguageID=409
;
;
; 1 - Russian
; 2 - English (United States)
; 3 - German
; 4 - Polish
; 5 - Romanian
; 6 - Italian
; 7 - Czech
; 8 - Spanish

SelectedVoiceIndex=0
;
; Voice selection by index (NEW in this fork)
; 0 - Auto (automatically select voice based on Language setting)
; 1-50 - Specific voice from your system (see full list in game options menu)
;
; NOTE: The 'Gender' option has been replaced with 'SelectedVoiceIndex'
; for more flexible voice selection
```

### How to Install Additional SAPI 5 Voices

1. **Download SAPI 5 compatible voice** (e.g., RHVoice, IVONA, Microsoft voices)
2. **Install the voice** following the provider's instructions
3. **Launch Gothic 2** with this plugin
4. **Open Options → zTTSDialogues**
5. **Select "Voice"** option to see all installed voices by name
6. **Choose your preferred voice** - it will preview immediately!

### Recommended Voice Sources

- **RHVoice** - Free, open-source, high-quality voices for multiple languages
- **Microsoft Speech Platform** - Additional voices from Microsoft
- **IVONA** - Professional quality voices (commercial)
- **Windows built-in voices** - Already included in Windows 10/11

