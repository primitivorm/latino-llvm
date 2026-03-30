# https://clang.llvm.org/get_started.html

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

if (!(Test-Path -Path $directorio_actual\llvm-project\build)) {
    New-Item -ItemType directory -Path $directorio_actual\llvm-project\build
}

Set-Location llvm-project\build
cmake -G Ninja ../llvm `
	-DLLVM_PARAREL_COMPILER_JOBS=7 `
	-DLLVM_PARAREL_LINK_JOBS=1 `
	-DLLVM_BUILD_EXAMPLES=OFF `
	-DLLVM_TARGETS_TO_BUILD="x86" `
	-DLLVM_BUILD_TYPE=Debug `
	-DLLVM_ENABLE_ASSERTIONS=ON `
	-DLLVM_CCACHE_BUILD=ON `
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
	-DLLVM_ENABLE_PROJECTS="clang;lld;lldb;mlir;clang-tools-extra;compiler-rt" `
	-DCMAKE_C_COMPILERS=clang `
	-DCMAKE_CXX_COMPILERS=clang++ `
	-DLLVM_ENABLE_LLD=ON

# correr ninja con 7 hilos
ninja -j 7

Set-Location $directorio_actual