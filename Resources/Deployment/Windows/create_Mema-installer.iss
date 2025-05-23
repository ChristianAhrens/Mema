#define MyAppName "Mema"
#define MyAppExeName "Mema.exe"
#define MyAppExePath "..\..\..\Builds\VisualStudio2022\x64\Release\App\Mema.exe"
#define MyAppVersion GetVersionNumbersString("..\..\..\Builds\VisualStudio2022\x64\Release\App\Mema.exe")
#define MyAppPublisher "Christian Ahrens"
#define MyAppURL "https://www.github.com/ChristianAhrens/Mema"

[Setup]
AppId={{0973C12E-915A-4D77-92FA-187CB97C73A9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
VersionInfoVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\..\..\LICENSE
InfoAfterFile=..\..\..\CHANGELOG.md
PrivilegesRequired=lowest
OutputBaseFilename="{#MyAppName}Setup_v{#MyAppVersion}"
SetupIconFile=..\..\..\Builds\VisualStudio2022\icon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#MyAppExePath}"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[UninstallDelete]
Type: files; Name: "{userappdata}\{#MyAppName}\*";
Type: dirifempty; Name: "{userappdata}\{#MyAppName}\";

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

