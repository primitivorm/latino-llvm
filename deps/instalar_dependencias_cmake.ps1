# https://clang.llvm.org/get_started.html

# establecer path donde se instalo visual studio
$vs_path = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community'

$directorio_actual = Get-Location
if (!(Test-Path -Path $directorio_actual\llvm-project)) {
    # clonar version especifica 11.x
    git clone --config core.autocrlf=false https://github.com/llvm/llvm-project.git --branch release/11.x llvm-project
}

# Cambiar la siguiente linea para compilar para x64
# Start-Process "$vs_path\VC\Auxiliary\Build\vcvars64.bat"
Start-Process "$vs_path\VC\Auxiliary\Build\vcvars32.bat"
if (!(Test-Path -Path $directorio_actual\llvm-project\build)) {
    New-Item -ItemType directory -Path $directorio_actual\llvm-project\build
}

Set-Location llvm-project\build

cmake -G "Visual Studio 16 2019" -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BUILD_EXAMPLES=ON `
    -DCLANG_BUILD_EXAMPLES=ON -DLLVM_ENABLE_OCAMLDOC=OFF -DLLVM_BUILD_DOCS=OFF `
    -DCMAKE_BUILD_TYPE=Release -DLLVM_BUILD_TESTS=ON -DLLVM_ENABLE_PROJECTS='clang' ..\llvm\

# ejecuta msbuild en modo Release
Start-Process -FilePath "$vs_path\MSBuild\Current\Bin\MSBuild.exe" -ArgumentList "LLVM.sln /t:Build /p:Configuration=Release" -NoNewWindow

# ejecuta msbuild en modo Debug
# [System.Diagnostics.Process]::Start("$vs_path\MSBuild\Current\Bin\MSBuild.exe", "$directorio_actual\llvm-project\build\LLVM.sln")

# regresar al directorio actual
Set-Location $directorio_actual