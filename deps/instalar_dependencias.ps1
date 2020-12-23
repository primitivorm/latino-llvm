# https://clang.llvm.org/get_started.html

# establecer path donde se instalo visual studio
$vs_path = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview'

$directorio_actual = Get-Location
if (!(Test-Path -Path $directorio_actual\llvm-project)) {
    git clone --config core.autocrlf=false https://github.com/llvm/llvm-project.git
}
Start-Process "$vs_path\VC\Auxiliary\Build\vcvars64.bat"
if (!(Test-Path -Path $directorio_actual\llvm-project\build)) {
    New-Item -ItemType directory -Path $directorio_actual\llvm-project\build
}
Set-Location llvm-project\build
cmake -G "Visual Studio 16 2019" -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BUILD_EXAMPLES=ON -DCLANG_BUILD_EXAMPLES=ON -DLLVM_ENABLE_OCAMLDOC=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_BUILD_TESTS=ON -Thost=x64 -DCMAKE_BUILD_TYPE=Release ..\llvm\

# ejecuta msbuild
[System.Diagnostics.Process]::Start("$vs_path\MSBuild\Current\Bin\MSBuild.exe", "$directorio_actual\llvm-project\build\LLVM.sln")

# regresar al directorio actual
Set-Location $directorio_actual