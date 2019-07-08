$directorio_actual = Get-Location
#crear directorio llvm-mirror si no existe
if(!(Test-Path -Path $directorio_actual\llvm-mirror )){
    New-Item -ItemType directory -Path $directorio_actual\llvm-mirror
}
Set-Location llvm-mirror
# if(Test-Path -Path $directorio_actual\llvm-mirror\llvm){
#     Remove-Item –Path $directorio_actual\llvm-mirror\llvm -Force –Recurse
# }
git clone --recursive https://github.com/llvm-mirror/llvm
Set-Location llvm\tools
git clone --recursive https://github.com/llvm-mirror/clang
git clone --recursive https://github.com/llvm-mirror/lld

Set-Location $directorio_actual\llvm-mirror\llvm\projects
git clone --recursive https://github.com/llvm-mirror/compiler-rt
git clone --recursive https://github.com/llvm-mirror/libcxx
git clone --recursive https://github.com/llvm-mirror/libcxxabi

Set-Location $directorio_actual\llvm-mirror\llvm\tools\clang\tools
git clone --recursive https://github.com/llvm-mirror/clang-tools-extra extra

Set-Location $directorio_actual
Start-Process 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat'
#crear directorio llvm-mirror if not exists
if(!(Test-Path -Path $directorio_actual\llvm-mirror\visualstudio)){
    New-Item -ItemType directory -Path $directorio_actual\llvm-mirror\visualstudio
}
Set-Location $directorio_actual\llvm-mirror\visualstudio
cmake -G "Visual Studio 16 2019" -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BUILD_EXAMPLES=ON -DCLANG_BUILD_EXAMPLES=ON -DLLVM_ENABLE_OCAMLDOC=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_BUILD_TESTS=ON -Thost=x64 -DCMAKE_BUILD_TYPE=Release ..\llvm\

#Run msbuild
[System.Diagnostics.Process]::Start("C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe", "$directorio_actual\llvm-mirror\visualstudio\LLVM.sln")
