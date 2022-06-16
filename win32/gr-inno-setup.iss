;
; GRIEF - Inno Setup Script.
;
; This file is part of the GRIEF Editor.
;
; The GRIEF Editor is free software: you can redistribute it
; and/or modify it under the terms of the GRIEF Editor License.
;
; Redistributions of source code must retain the above copyright
; notice, and must be distributed with the license document above.
;
; Redistributions in binary form must reproduce the above copyright
; notice, and must include the license document above in
; the documentation and/or other materials provided with the
; distribution.
;
; The GRIEF Editor is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; License for more details.
;

#if defined(BUILD_INFO)
#include "../include/edbuildinfo.h"
#else
#include "../include/edpackageinfo.h"
#endif 

#if defined(BUILD_TOOLCHAIN)
#if defined(BUILD_TYPE)
#define BinDir "bin" + BUILD_TOOLCHAIN + "\\" + BUILD_TYPE
#else
#define BinDir "bin" + BUILD_TOOLCHAIN
#endif
#else
#define BinDir "bin"
#endif

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
;
AppId={{1BDBED1A-1B0E-4D87-BD04-31E9E3DA5ADC}}
AppName=GRIEF
AppVersion={#GR_VERSION} (build: {#GR_BUILD_DATE}-{#GR_BUILD_NUMBER})
AppCopyright=Copyright (C) 1998-2022.
AppPublisherURL=http://sourceforge.net/projects/grief/
AppSupportURL=https://github.com/adamyg/grief
AppUpdatesURL=https://github.com/adamyg/grief

;TODO, skins
;  http://isskin.codejock.com/
;  https://code.google.com/p/vcl-styles-plugins/

DefaultDirName={pf}\Grief
DefaultGroupName=Grief
LicenseFile=../COPYING

OutputDir=.
OutputBaseFilename=grwin32-build{#GR_BUILD_NUMBER}-setup
Compression=lzma
SolidCompression=yes
ChangesEnvironment=true

UninstallDisplayIcon={app}\bin\gr.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: modifypath; Description: Add application directory to your environmental path; Flags: unchecked
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Registry]
Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: string; ValueName: ""; ValueData: {app}; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: string; ValueName: "Path"; ValueData: "{app}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: string; ValueName: "UninstallString"; ValueData: {uninstallexe}; Flags: uninsdeletevalue

Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: dword;  ValueName: "MajorVersion";  ValueData: "{#GR_VERSION_1}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: dword;  ValueName: "MinorVersion";  ValueData: "{#GR_VERSION_2}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: dword;  ValueName: "PatchVersion";  ValueData: "{#GR_VERSION_3}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\GRIEF-Edit"; ValueType: dword;  ValueName: "BuildVersion";  ValueData: "{#GR_VERSION_4}"; Flags: uninsdeletevalue

[Files]
Source: "..\{#BinDir}\gr.exe";        DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\gm.exe";        DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\grcpp.exe";     DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\grunch.exe";    DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\grmandoc.exe";  DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\grwc.exe";      DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\grupdater.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\*.dll";         DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\{#BinDir}\ctbl\*";        DestDir: "{app}\bin\ctbl"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\{#BinDir}\i18n\*";        DestDir: "{app}\bin\i81n"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\macros\*";                DestDir: "{app}\macros"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\help\*";                  DestDir: "{app}\help"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\COPYING";                 DestDir: "{app}"; Flags: ignoreversion
Source: "..\Changes";                 DestDir: "{app}"; Flags: ignoreversion
; NOTE: Dont use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Grief"; Filename: "{app}\bin\gr.exe"
Name: "{commondesktop}\Grief"; Filename: "{app}\bin\gr.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Grief"; Filename: "{app}\bin\gr.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\bin\gr.exe"; Description: "{cm:LaunchProgram,Grief}"; Flags: nowait postinstall skipifsilent

[Code]
function GRIEFInstalled(var version, uninstallcmd:string):boolean;
var major,minor,patch,build:Cardinal;
begin
        Result := RegQueryDWordValue(HKLM, 'Software\GRIEF-Edit', 'MajorVersion', major)
                  and RegQueryDWordValue(HKLM, 'Software\GRIEF-Edit', 'MinorVersion', minor)
                  and RegQueryDWordValue(HKLM, 'Software\GRIEF-Edit', 'PatchVersion', patch)
                  and RegQueryDWordValue(HKLM, 'Software\GRIEF-Edit', 'BuildVersion', build);

        if not RegQueryStringValue(HKLM, 'Software\GRIEF-Edit', 'UninstallString', uninstallcmd)
        then if not RegQueryStringValue(HKLM, 'Software\GRIEF-Edit', 'Uninstall', uninstallcmd)
        then RegQueryStringValue(HKLM, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\GRIEF-Edit', 'UninstallString', uninstallcmd);
        version := IntToStr(major)+'.'+IntToStr(minor)+'.'+IntToStr(patch)+'-'+IntToStr(build);
end;

function InitializeSetup(): Boolean;
        var version, uninst :string;
            msgres, execres :integer;
begin
        Result:=true;
        if GRIEFInstalled(version,uninst)
        then
                begin
                msgres:= MsgBox('GRIEF Edit-'+version+' is currently installed.'+#13#13 +'Do you want to uninstall it first?.', mbError, MB_YESNOCANCEL);
                case msgres of
                IdYes: begin
                        Exec(uninst, '', '', SW_SHOWNORMAL, true, execres);
                        Result:=InitializeSetup();
                        end;
                IdCancel:
                         Result:=false;
                IdNo: ;
                end;
        end;
end;

[Code]
const   ModPathName = 'modifypath';
        ModPathType = 'user';

function ModPathDir(): TArrayOfString;
begin
        setArrayLength(Result, 1)
    	Result[0] := ExpandConstant('{app}\bin');
end;

procedure DosToUnix();
var
        path : String;
        data : String;
        ANSIdata : AnsiString;
begin
        path := ExpandConstant(CurrentFileName);
        LoadStringFromFile(path, ANSIdata);
        data := String(ANSIData);
        StringChangeEx(data, #13#10, #10, True);
        SaveStringToFile(path, AnsiString(data), False);
end;

#include "modpath.iss"
