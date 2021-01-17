# https://clang.llvm.org/get_started.html

# establecer path donde se instalo visual studio
$vs_path = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community'
# establecer path donde esta instalado Windows Kits
$wk_path = 'C:\Program Files (x86)\Windows Kits\10'

# Agregar a $PATH
$env:Path += "$vs_path\VC\Tools\MSVC\14.28.29617\bin\Hostx64\x64"
$env:Path += "$vs_path\MSBuild\Current\Bin"
$env:Path += "$wk_path\bin\10.0.18362.0\x64"

# Agregar a $LIB
$env:LIB += "$vs_path\VC\Tools\MSVC\14.28.29617\lib\x64"
$env:LIB += "$vs_path\VC\Tools\MSVC\14.28.29617\lib\x64\uwp"
$env:LIB += "$wk_path\Lib\10.0.18362.0\um\x64"
$env:LIB += "$wk_path\Lib\10.0.17763.0\ucrt\x64"


# Agregar a CPATH
$env:CPATH += "$wk_path\VC\Tools\MSVC\14.28.29617\include"
$env:CPATH += "$wk_path\Include\10.0.17763.0\ucrt"
$env:CPATH += "$wk_path\Include\10.0.18362.0\ucrt"
$env:CPATH += "$wk_path\Include\10.0.17763.0\um"

$directorio_actual = Get-Location
if (!(Test-Path -Path $directorio_actual\llvm-project)) {
    git clone --config core.autocrlf=false https://github.com/llvm/llvm-project.git --branch release/11.x llvm-project
}

if (!(Test-Path -Path $directorio_actual\llvm-project\build)) {
    New-Item -ItemType directory -Path $directorio_actual\llvm-project\build
}

# registrar compilador CL.exe
"$vs_path\VC\Auxiliary\Build\vcvarsx86_amd64.bat"

Set-Location llvm-project\build
cmake -G "Ninja" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_MAKE_PROGRAM=ninja `
    --debug-trycompile -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host `
    -DLLVM_BUILD_EXAMPLES=ON -DLLVM_ENABLE_OCAMLDOC=OFF -DLLVM_BUILD_DOCS=OFF `
    -DLLVM_BUILD_TESTS=ON -DLLVM_ENABLE_PROJECTS='clang' ..\llvm\

# correr ninja con 4 hilos
ninja -j 4

Set-Location $directorio_actual