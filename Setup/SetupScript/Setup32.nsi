;Name and file
!define APPROACH_VERSION "0.5.2"

Name "KO Approach"
Caption "KO Approach ${APPROACH_VERSION} (x86)"
OutFile "Approach_Setup_x86.exe"
SetCompressor /SOLID lzma

;Default installation folder
InstallDir "$PROGRAMFILES32\KO Approach"

!define APPROACH_ARCHBITS 32
!include "Common.nsh"