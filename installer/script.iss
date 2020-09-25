#define MyAppName "Connection Tester"
#define MyAppVersion "1.0"
#define MyAppPublisher "Sergey Agafonov"
#define MyAppURL "https://github.com/agafon0ff/ConnectionTester"
#define MyAppExeName "ConnectionTester.exe"

[Setup]
AppId={{CC9D60C0-8176-4F32-BEAB-3538A490676E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\ConnectionTester
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=F:\projects\qt\ConnectionTester\LICENSE
OutputDir=F:\projects\qt\ConnectionTester\release\inno
OutputBaseFilename=ConnectionTesterSetup
SetupIconFile=F:\projects\qt\ConnectionTester\res\connection.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Languages\English.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "F:\projects\qt\ConnectionTester\release\mingw32\ConnectionTester.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "F:\projects\qt\ConnectionTester\release\mingw32\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall skipifsilent
