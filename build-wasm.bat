@echo off
setlocal

REM BEFORE RUNNING THIS SCRIPT MAKE SURE EMSCRIPTEN ENVIRONMENT IS SET UP!

REM ======= Prepare paths =======

REM Store the path where the .bat file is being executed
set "SCRIPT_DIR=%~dp0"

REM Define a variable containing a directory relative path
set "BUILD_PATH=builds\wasm"


REM ======= Check if clean build =======

REM Read the first argument and save a flag if it is "--clean"
set "CLEAN_FLAG=false"
if "%1"=="--clean" (
	set "CLEAN_FLAG=true"
)

REM If "--clean" argument was passed and the directory exists, delete it
if "%CLEAN_FLAG%"=="true" (
	if exist "%BUILD_PATH%" (
		rd /s /q "%BUILD_PATH%"
	)
)


REM ======= Move to the build directory =======

REM If the directory doesn't exist, create it
if not exist "%BUILD_PATH%" (
	mkdir "%BUILD_PATH%"
)

REM Move to the build path
cd %BUILD_PATH%


REM ======= Build web assembly with emscripten (assumes configuration is done) =======

REM Prepare for make
call emcmake cmake -G Ninja "%SCRIPT_DIR%"
if %errorlevel% neq 0 (
	echo emcmake cmake command failed with error level %errorlevel%
	exit /b %errorlevel%
)

REM Make
call emmake ninja
if %errorlevel% neq 0 (
	echo emmake ninja command failed with error level %errorlevel%
	exit /b %errorlevel%
)


REM ======= Copy built files to a public directory, ready to be published =======

REM Define the target directory
set "PUBLIC_BUILD_PATH=%SCRIPT_DIR%\public-html"

REM Ensure the public directory exists
if not exist "%PUBLIC_BUILD_PATH%" (
	mkdir "%PUBLIC_BUILD_PATH%"
)

REM Remove build.wasm, build.data and build.js files from the target directory
del /q "%PUBLIC_BUILD_PATH%\build.wasm"
del /q "%PUBLIC_BUILD_PATH%\build.data"
del /q "%PUBLIC_BUILD_PATH%\build.js"

REM Copy build.js, build.data and build.wasm files from the current directory to the target directory
copy /y "build.js" "%PUBLIC_BUILD_PATH%"
copy /y "build.data" "%PUBLIC_BUILD_PATH%"
copy /y "build.wasm" "%PUBLIC_BUILD_PATH%"

REM Go back to initial path
cd %SCRIPT_DIR%

endlocal
