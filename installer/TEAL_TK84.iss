; ============================================================================
;  TEAL TK84 - Windows installer (Inno Setup 6 script)
;
;  Produces a distributable installer .exe that installs the TEAL TK84 VST3
;  plugin (and, optionally, the Standalone app) - no build tools required on
;  the end user's machine.
;
;  HOW TO BUILD THIS INSTALLER (on Windows):
;    1. Build the plugin first so the artefacts exist:
;         install.bat              (or: cmake --build build --config Release)
;    2. Install Inno Setup 6:      https://jrsoftware.org/isdl.php
;    3. Compile this script:
;         iscc installer\TEAL_TK84.iss
;       (or open it in the Inno Setup IDE and press F9)
;
;  Output:  installer\Output\TEAL_TK84-1.0.0-Windows.exe
; ============================================================================

#define AppName        "TEAL TK84"
#define AppVersion      "1.0.0"
#define AppPublisher    "TEAL"
#define AppExeName      "TEAL TK84.exe"
#define Vst3BundleName  "TEAL TK84.vst3"

; Location of the built artefacts, relative to this .iss file.
; Override on the command line with:  iscc /DArtefactDir="C:\path\Release" ...
#ifndef ArtefactDir
  #define ArtefactDir "..\build\TEAL_TK84_artefacts\Release"
#endif

[Setup]
AppId={{40d20d82-7674-4292-becc-b48b4510bc84}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\{#AppPublisher}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir=Output
OutputBaseFilename=TEAL_TK84-{#AppVersion}-Windows
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}
; VST3 installs to a machine-wide location, so administrator rights are needed.
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Types]
Name: "full";   Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3";       Description: "VST3 plugin (recommended)"; Types: full custom; Flags: fixed
Name: "standalone"; Description: "Standalone application";     Types: full

[Files]
; --- VST3 bundle -> Common Files\VST3\TEAL TK84.vst3 ---
Source: "{#ArtefactDir}\VST3\{#Vst3BundleName}\*"; \
    DestDir: "{commoncf}\VST3\{#Vst3BundleName}"; \
    Flags: ignoreversion recursesubdirs createallsubdirs; \
    Components: vst3

; --- Standalone app -> Program Files\TEAL\TEAL TK84 ---
; skipifsourcedoesntexist lets the installer compile even if the Standalone
; format was not built.
Source: "{#ArtefactDir}\Standalone\{#AppExeName}"; \
    DestDir: "{app}"; \
    Flags: ignoreversion skipifsourcedoesntexist; \
    Components: standalone

[Icons]
Name: "{group}\{#AppName}";          Filename: "{app}\{#AppExeName}"; Components: standalone
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Launch {#AppName}"; \
    Flags: nowait postinstall skipifsilent; Components: standalone

[UninstallDelete]
; Remove the VST3 bundle folder on uninstall.
Type: filesandordirs; Name: "{commoncf}\VST3\{#Vst3BundleName}"
