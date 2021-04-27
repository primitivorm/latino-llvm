# establecer path donde se instalo visual studio
$vs_path = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community'

$directorio_actual = Get-Location

# Cambiar la siguiente linea para compilar para x64
# Start-Process "$vs_path\VC\Auxiliary\Build\vcvars64.bat"
Start-Process "$vs_path\VC\Auxiliary\Build\vcvars32.bat"
if (!(Test-Path -Path $directorio_actual\build)) {
    New-Item -ItemType directory -Path $directorio_actual\build
}

Set-Location build

# Agregar -Thost=x64 para forzar a compilar para x64
cmake -G "Visual Studio 16 2019" -DLLVM_TARGETS_TO_BUILD=host `
    -DCMAKE_BUILD_TYPE=Release ..\

# ejecuta msbuild en modo Release
# Start-Process -FilePath "MSBuild.exe" -WorkingDirectory "$vs_path\MSBuild\Current\Bin" -ArgumentList "/t:Build /p:Configuration=Release" -Wait -NoNewWindow
Start-Process -FilePath "$vs_path\MSBuild\Current\Bin\MSBuild.exe" -ArgumentList "latino.sln /t:Build /p:Configuration=Release" -NoNewWindow

# ejecuta msbuild en modo Debug
# [System.Diagnostics.Process]::Start("$vs_path\MSBuild\Current\Bin\MSBuild.exe", "$directorio_actual\build\LLVM.sln")

# regresar al directorio actual
Set-Location $directorio_actual