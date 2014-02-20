;--------------------------------
;Include Modern UI

  !include "MUI.nsh"
  !include "WinMessages.nsh"
  !include "WordFunc.nsh"
  
  
;--------------------------------
;Plugins
  !addplugindir "." 


;--------------------------------
;General

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\KO Software\Approach" ""
  
  ReserveFile "PageFinish.ini"
  ReserveFile "PageRunning.ini"
  
  RequestExecutionLevel admin
  
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "..\header.bmp"
  !define MUI_UI                 "ApproachNsisDlg.exe"
  !define MUI_ABORTWARNING
  !define MUI_COMPONENTSPAGE_TEXT_DESCRIPTION_INFO "Place your mouse pointer over a component to see its description"

  ; variables declaration
  var TempResult
  var TempDeleteRoots
  ; end variable declaration


;--------------------------------
;Functions

Function .onInit
  ApproachNsis::IsVersionOK
  Pop $TempResult
  StrCmp $TempResult "1" +3

  ;Version is not supported
  MessageBox MB_OK|MB_ICONSTOP "Sorry, but KO Approach cannot be installed on this operating system!$\nSetup will not continue."
  Abort
  
  SetRegView ${APPROACH_ARCHBITS}
FunctionEnd


Function un.onInit
  SetRegView ${APPROACH_ARCHBITS}
FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
!macro ProgramTerminate UN
Function ${UN}QuitProgram

  ApproachNsis::QuitProgram
  Pop $1
  StrCmp $1 "1" ApproachIsNotRunning ApproachIsRunning


ApproachIsNotRunning:
  Push ""
  Return


ApproachIsRunning:
  Push "KO Approach is currently running.\r\nPlease quit KO Approach and click Retry."
  Return

FunctionEnd
!macroend

!insertmacro ProgramTerminate ""
!insertmacro ProgramTerminate "un."

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
!macro PageRunning_Enter UN
Function ${UN}PageRunning

  Call ${UN}QuitProgram
  Pop $1
  StrCmp $1 "" NoErrorsOccured ErrorsOccured
  
ErrorsOccured:
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "PageRunning.ini"
  !insertmacro MUI_HEADER_TEXT "Quit KO Approach" "Waiting for the program to close."

  !insertmacro MUI_INSTALLOPTIONS_WRITE "PageRunning.ini" "Field 1" "Text" $1

  !insertmacro MUI_INSTALLOPTIONS_INITDIALOG "PageRunning.ini"
  Pop $1
  
  ApproachNsis::DlgRunningPrepare $1

  !insertmacro MUI_INSTALLOPTIONS_SHOW
  Return
  
  
NoErrorsOccured:
  Abort
  
FunctionEnd
!macroend

!insertmacro PageRunning_Enter ""
!insertmacro PageRunning_Enter "un."

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!macro PageRunning_Leave UN
Function ${UN}PageRunningLeave

  Call ${UN}QuitProgram
  Pop $1
  StrCmp $1 "" NoErrorsOccured ErrorsOccured
  
NoErrorsOccured:
  Return

ErrorsOccured:
  Abort
FunctionEnd
!macroend

!insertmacro PageRunning_Leave ""
!insertmacro PageRunning_Leave "un."

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

LangString TEXT_IO_TITLE ${LANG_ENGLISH} "Success"
LangString TEXT_IO_SUBTITLE ${LANG_ENGLISH} "Congratulations! The program was successfully installed."

Function CustomFinishPage
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "PageFinish.ini"
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"

  
  ;Check if Approach.key exists in the installation directory
  
  FindFirst $0 $1 $INSTDIR\Approach.key
   
  StrCmp $1 "" KeyNotExists KeyExists 
  
KeyExists:
  FindClose $0

  ;uncheck the checkbox
  !insertmacro MUI_INSTALLOPTIONS_WRITE "PageFinish.ini" "Field 4" "State" "0"
  Goto Finish


KeyNotExists:
  ;check the checkbox
  !insertmacro MUI_INSTALLOPTIONS_WRITE "PageFinish.ini" "Field 4" "State" "1"

Finish:
  ;!insertmacro MUI_INSTALLOPTIONS_DISPLAY "PageFinish.ini"
  
  !insertmacro MUI_INSTALLOPTIONS_INITDIALOG "PageFinish.ini"
  
  StrCmp $1 "" +2 
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "PageFinish.ini" "Field 4" "HWND"
  ShowWindow $0 ${SW_HIDE}
  
  !insertmacro MUI_INSTALLOPTIONS_SHOW
  

FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Function CustomFinishPageLeave

  ;Read a value from an InstallOptions INI file - Launch Approach
  !insertmacro MUI_INSTALLOPTIONS_READ $1 "PageFinish.ini" "Field 2" "State"
  
  ;Read a value from an InstallOptions INI file - Quick Start guide
  !insertmacro MUI_INSTALLOPTIONS_READ $2 "PageFinish.ini" "Field 3" "State"
  
  ;Read a value from an InstallOptions INI file - Order Approach
  !insertmacro MUI_INSTALLOPTIONS_READ $3 "PageFinish.ini" "Field 4" "State"

  StrCmp $1 "1" "" +2
  ExecShell "open" "$INSTDIR\Approach.exe"

  StrCmp $2 "1" "" +2
  ExecShell "open" "hh.exe" "mk:@MSITStore:$INSTDIR\Approach.chm::/content/quick_start.html>tiny"
  
  StrCmp $3 "1" "" +2
  ExecShell "open" "http://www.koapproach.com/order.aspx"

FunctionEnd


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!insertmacro un.WordFind

Function un.PageItems

  ReadRegStr $0 HKLM "Software\KO Software\Approach" "KoApproachItemsRoots"
  StrCmp $0 "" End ProcessRoots

ProcessRoots:

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "PageUnItems.ini"
  !insertmacro MUI_HEADER_TEXT "Remove KO Approach Items" "You may wish to remove user-specific folders"
  !insertmacro MUI_INSTALLOPTIONS_INITDIALOG "PageUnItems.ini"

  !insertmacro MUI_INSTALLOPTIONS_READ $2 "PageUnItems.ini" "Field 2" "HWND"

  StrCpy $1 "1"
  
Loop:
  ${un.WordFind} $0 "*" "+$1" $R0

  StrCmp $R0 $0 AfterLoop
  
  SendMessage $2 ${LB_ADDSTRING} 1 "STR:$R0"
  IntOp $1 $1 + 1

  Goto Loop

AfterLoop:
  !insertmacro MUI_INSTALLOPTIONS_SHOW
  
End:
  
FunctionEnd


Function un.PageItemsLeave
  !insertmacro MUI_INSTALLOPTIONS_READ $TempDeleteRoots "PageUnItems.ini" "Field 3" "State"
FunctionEnd


Function un.EnsureRootsRemoved
  StrCmp $TempDeleteRoots "1" +1 AfterLoop
  


  ReadRegStr $0 HKLM "Software\KO Software\Approach" "KoApproachItemsRoots"
  StrCpy $1 "1"

Loop:
  ${un.WordFind} $0 "*" "+$1" $R0

  StrCmp $R0 $0 AfterLoop

  RMDir /r $R0
  IntOp $1 $1 + 1

  Goto Loop
  
AfterLoop:

FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "../../License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  Page custom PageRunning PageRunningLeave
  !insertmacro MUI_PAGE_INSTFILES

  AutoCloseWindow true           ;this closes the 'installing' page and advances to the 'finish' page
  Page custom CustomFinishPage CustomFinishPageLeave


  
  !insertmacro MUI_UNPAGE_CONFIRM
  UninstPage custom un.PageItems un.PageItemsLeave
  UninstPage custom un.PageRunning un.PageRunningLeave
  !insertmacro MUI_UNPAGE_INSTFILES
  MiscButtonText "" "" "" "Finish"
  
  ;!insertmacro MUI_UNPAGE_FINISH
  ;!insertmacro MUI_FINISHPAGE_BUTTON "Finish"
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Program Files" SecProgFiles

  SectionIn RO

  SetShellVarContext all

  SetOutPath "$INSTDIR"

  File ..\Common${APPROACH_ARCHBITS}\Approach.exe
  File ..\Common${APPROACH_ARCHBITS}\ApproachIpc.dll
  File ..\Common32\Approach.chm
  File ..\..\License.txt

  CreateDirectory $INSTDIR\Locale


  ;Store installation folder
  WriteRegStr HKLM "Software\KO Software\Approach" "" $INSTDIR

  ;InitiallyOpen
  WriteRegDWORD HKCU "Software\KO Software\Approach" "AlreadyLaunched" 0

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Write uninstall string to the registry
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "DisplayName"      "KO Approach"
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "DisplayIcon"      "$INSTDIR\Approach.exe"
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "DisplayVersion"   "${APPROACH_VERSION}"
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "HelpLink"         "http://www.ko-sw.com/Products/Approach/"
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "Publisher"        "KO Software"
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "UninstallString"  "$INSTDIR\Uninstall.exe"
  WriteRegStr    HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "URLInfoAbout"     "http://www.ko-sw.com/Support/Approach/"
  WriteRegDWORD  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "NoModify"         "1"
  WriteRegDWORD  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach" "NoRepair"         "1"

  ApproachNsis::UpdateRegPermissions       ;"Software\KO Software\Approach", "S-1-5-32-545", "KEY_FULL_ACCESS", propagate to children

SectionEnd

;--------------------------------

Section "Program Group" SecShortcut
  SetShellVarContext all

  SetOutPath "$SMPROGRAMS\KO Approach"

  CreateShortCut "$SMPROGRAMS\KO Approach\KO Approach.lnk"           "$INSTDIR\Approach.exe" "-options"
  CreateShortCut "$SMPROGRAMS\KO Approach\Uninstall KO Approach.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\KO Approach\KO Approach Help.lnk"      "$INSTDIR\Approach.chm"
SectionEnd

;--------------------------------

Section "Auto-run" SecStartup
  SetShellVarContext current
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMSTARTUP\KO Approach.lnk" "$INSTDIR\Approach.exe"
SectionEnd

;--------------------------------

SectionGroup "Plugins"

Section /o "Scope" SecPluginScope
  SetOutPath "$INSTDIR\Plugins"
  File ..\Common${APPROACH_ARCHBITS}\Scope.dll
  File ..\Common32\readme_scope.txt
SectionEnd


Section /o "InstantWave" SecPluginIWave
  SetOutPath "$INSTDIR\Plugins"
  File ..\Common${APPROACH_ARCHBITS}\InstantWave.dll
SectionEnd


Section /o "InstantTxt" SecPluginITxt
  SetOutPath "$INSTDIR\Plugins"
  File ..\Common${APPROACH_ARCHBITS}\InstantTxt.dll
SectionEnd


Section /o "ClipboardAssistant" SecPluginClipboard
  SetOutPath "$INSTDIR\Plugins"
  File ..\Common${APPROACH_ARCHBITS}\ClipboardAssistant.dll
SectionEnd

SectionGroupEnd


;--------------------------------

;Language strings
LangString DESC_SecProgFiles       ${LANG_ENGLISH} "Installs files necessary to run KO Approach"
LangString DESC_SecShortcut        ${LANG_ENGLISH} "Creates a group in the Programs folder of the Start menu"
LangString DESC_SecStartup         ${LANG_ENGLISH} "Automatically launches KO Approach when Windows starts"
LangString DESC_SecPluginScope     ${LANG_ENGLISH} "Enables previewing thumbnails of most common graphics files"
LangString DESC_SecPluginIWave     ${LANG_ENGLISH} "Enables playing WAV files"
LangString DESC_SecPluginITxt      ${LANG_ENGLISH} "Enables previewing TXT files"
LangString DESC_SecPluginClipboard ${LANG_ENGLISH} "Inserts additional file- and folder-related commands to standard context menus"

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecProgFiles}       $(DESC_SecProgFiles)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcut}        $(DESC_SecShortcut)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartup}         $(DESC_SecStartup)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPluginScope}     $(DESC_SecPluginScope)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPluginIWave}     $(DESC_SecPluginIWave)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPluginITxt}      $(DESC_SecPluginITxt)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPluginClipboard} $(DESC_SecPluginClipboard)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetShellVarContext all

  ;0. remove roots
  Call un.EnsureRootsRemoved

  ;1. remove plugins
  Delete "$INSTDIR\Plugins\Scope.dll"
  Delete "$INSTDIR\Plugins\readme_scope.txt"
  
  Delete "$INSTDIR\Plugins\InstantWave.dll"
  
  Delete "$INSTDIR\Plugins\InstantTxt.dll"
  
  Delete "$INSTDIR\Plugins\ClipboardAssistant.dll"
  
  RMDir "$INSTDIR\Plugins"
  
  ;2. installation folder

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\Approach.exe"
  Delete "$INSTDIR\ApproachIpc.dll"
  Delete "$INSTDIR\License.txt"
  Delete "$INSTDIR\Approach.chm"
  
  RMDir "$INSTDIR"
  
  ;3. program group
  Delete "$SMPROGRAMS\KO Approach\KO Approach.lnk"
  Delete "$SMPROGRAMS\KO Approach\Uninstall KO Approach.lnk"
  Delete "$SMPROGRAMS\KO Approach\KO Approach Help.lnk"
  
  RMDir "$SMPROGRAMS\KO Approach"
  
  
  
  ;4. startup shortcut
  SetShellVarContext current
  Delete "$SMSTARTUP\KO Approach.lnk"
  
  ;5. Registry keys
  DeleteRegKey          HKCU "Software\KO Software\Approach"
  DeleteRegKey /ifempty HKCU "Software\KO Software"
  
  DeleteRegKey          HKLM "Software\KO Software\Approach"
  DeleteRegKey /ifempty HKLM "Software\KO Software"
  
  DeleteRegKey          HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KO Approach"
  
SectionEnd