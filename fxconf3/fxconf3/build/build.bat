@echo off

cd build

:::::::::::::::::::
:: Setting paths ::
:::::::::::::::::::

set dllfile=%3
set ver=%2
set build=%ver:.=%
set infile=%1
set indir=%CD%\..\mq4
set midfile=%infile%.compile

set mthash=
for /f "delims=" %%a in ('%CD%\mthash.exe') do (
	set mthash=%%a
)

set outfile=fxc
set outdir=%APPDATA%\MetaQuotes\Terminal\%mthash%\MQL4\Experts\fxc

:::::::::::::::
:: Compiling ::
:::::::::::::::

:: copy source to temp file
copy %indir%\%infile%.mq4 %indir%\%midfile%.mq4

:: replace text in the temp file
CScript "%CD%\ReplaceText.vbs" //B //nologo %indir%\%midfile%.mq4 {{ver}} %ver%
CScript "%CD%\ReplaceText.vbs" //B //nologo %indir%\%midfile%.mq4 {{build}} %build%

:: compile temp file
%CD%\mql64.exe %indir%\%midfile%.mq4

:: delete temp file
:: del %indir%\%midfile%.mq4

:::::::::::::::::
:: Copy result ::
:::::::::::::::::

move /Y %indir%\%midfile%.ex4 %outdir%\%outfile%%build%.ex4
copy /Y %dllfile% %outdir%\%outfile%%build%.dll