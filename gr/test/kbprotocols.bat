@echo off

rem # Toolchain

if "%1" == "owc19" (
	set WCL=owc19
	set WCC=cl386

) else if "%1" == "owc20" (
	set WCL=owc20
	set WCC=cl386
        
) else if "%1" == "vc2019" (
	set WCL=vs160
	set WCC=cl

) else if "%1" == "mingw32" (
	set GCC=mingw32

) else if "%1" == "mingw64" (
	set GCC=mingw64

) else if "%1" == "gcc" (
	set GCC=gcc

) else (
	echo ... unknown target toolchain
	exit
)

rem # Execute

set SOURCE=kbprotocols_test.c ../kbname.c ../kbprotocols.c ../kbsequence.c ../kbwin32.c ../ttyutil.c

if not "%WCL%" == "" (
	echo TOOLCHAIN: %WCL%
	%WCC% -nologo -I.. -I..\..\libw32 -I..\..\include -DHAVE_CONFIG_H -DKBPROTOCOLS_TEST -MDd %SOURCE% ..\..\lib.%WCL%\debug\libtrie.lib ..\..\lib.%WCL%\debug\libmisc.lib ..\..\lib.%WCL%\debug\libdlmalloc.lib
)

if not "%GCC%" == "" (
	echo TOOLCHAIN: %GCC%
	gcc -I.. -I../../libw32 -I../../include -DHAVE_CONFIG_H -DKBPROTOCOLS_TEST -Wall -g -o kbprotocols_test %SOURCE% ../../lib.%GCC%/debug/libtrie.a ../../lib.%GCC%/debug/libmisc.a
)

