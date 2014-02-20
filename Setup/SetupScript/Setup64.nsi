;Name and file
!define APPROACH_VERSION "0.5.2"

Name "KO Approach"
Caption "KO Approach ${APPROACH_VERSION} (x64)"
OutFile "Approach_Setup_x64.exe"
SetCompressor /SOLID lzma

;Default installation folder
InstallDir "$PROGRAMFILES64\KO Approach"

!define APPROACH_ARCHBITS 64
!include "Common.nsh"