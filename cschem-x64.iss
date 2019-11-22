; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{84AF32E8-9487-421E-9A8B-73712049EBBD}
AppName=CSchem
AppVersion=0.1
AppPublisher=Daniel Wagenaar
AppPublisherURL=http://www.danielwagenaar.net
AppSupportURL=http://www.danielwagenaar.net
AppUpdatesURL=http://www.danielwagenaar.net
DefaultDirName={pf}\CSchem
DisableProgramGroupPage=yes
LicenseFile=C:\Users\Daniel Wagenaar\Documents\Progs\CSchem\GPL-3.0.txt
OutputDir=..\..\releases
OutputBaseFilename=CSchem-0.1-x64-setup
SetupIconFile=C:\Users\Daniel Wagenaar\Documents\Progs\CSchem\src\App\CSchem.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\Daniel Wagenaar\Documents\Progs\CSchem\release-CSchem-x86\*"; DestDir: "{app}"; Flags: ignoreversion  recursesubdirs createallsubdirs

[Icons]
Name: "{commonprograms}\CSchem"; Filename: "{app}\CSchem.exe"
Name: "{commondesktop}\CSchem"; Filename: "{app}\CSchem.exe"; Tasks: desktopicon
Name: "{commonprograms}\CPCB"; Filename: "{app}\CPCB.exe"
Name: "{commondesktop}\CPCB"; Filename: "{app}\CPCB.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\CSchem.exe"; Description: "{cm:LaunchProgram,CSchem}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\CPCB.exe"; Description: "{cm:LaunchProgram,CPCB}"; Flags: nowait postinstall skipifsilent
