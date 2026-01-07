# https://clang.llvm.org/get_started.html

# establecer path donde se instalo visual studio
$vs_path = 'C:\Program Files\Microsoft Visual Studio\2022\Professional'

$directorio_actual = Get-Location
if (!(Test-Path -Path $directorio_actual\llvm-project)) {
    # ejecutar este comando en cmd
    # git config --global http.postBuffer 524288000
    # clonar version especifica 11.x
    git clone --depth=1 --config core.autocrlf=false https://github.com/llvm/llvm-project.git --branch release/11.x llvm-project
    Set-Location llvm-project
    git fetch --all
    git pull
}

# Cambiar la siguiente linea para compilar para x64
# Start-Process "$vs_path\VC\Auxiliary\Build\vcvars64.bat"
Start-Process "$vs_path\VC\Auxiliary\Build\vcvars32.bat"

if (!(Test-Path -Path $directorio_actual\llvm-project\build)) {
    New-Item -ItemType directory -Path $directorio_actual\llvm-project\build
}

Set-Location llvm-project\build

# Agregar -Thost=x64 para forzar a compilar para x64
cmake -G "Visual Studio 17 2022" -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BUILD_EXAMPLES=ON `
    -DCLANG_BUILD_EXAMPLES=ON -DLLVM_ENABLE_OCAMLDOC=OFF -DLLVM_BUILD_DOCS=OFF `
    -DCMAKE_BUILD_TYPE=Debug -DLLVM_BUILD_TESTS=ON -DLLVM_INCLUDE_TESTS=ON `
    -DLLVM_ENABLE_PROJECTS='clang;lld;lldb' ..\llvm\

# ejecuta msbuild en modo Release
# Start-Process -FilePath "$vs_path\MSBuild\Current\Bin\MSBuild.exe" -ArgumentList "LLVM.sln /t:Build /p:Configuration=Release" -NoNewWindow

# ejecuta msbuild en modo Debug
[System.Diagnostics.Process]::Start("$vs_path\MSBuild\Current\Bin\MSBuild.exe", "$directorio_actual\llvm-project\build\LLVM.sln")

# regresar al directorio actual
Set-Location $directorio_actual