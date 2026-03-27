@echo off
cls
echo compilando...

set SRCS=src/main.cpp src/UI/Window.cpp src/UI/WindowPicker.cpp src/Backend/Graphics.cpp src/Backend/ScreenCapture.cpp src/Backend/FrameGenerator.cpp src/Backend/Renderer.cpp

cl.exe /EHsc /W3 /std:c++17 /DUNICODE /D_UNICODE /D_DEBUG /Zi /MDd %SRCS% user32.lib d3d11.lib dxgi.lib d3dcompiler.lib gdi32.lib /Fe:PicaScaling.exe /I. /Isrc

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERRO] Falha na compilacao.
    pause
    exit /b 1
)

echo.
echo [LOG] Compilacao concluida.
if not exist Shaders mkdir Shaders
xcopy src\Shaders\*.hlsl Shaders\ /Y /Q

echo.
pause