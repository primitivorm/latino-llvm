# https://clang.llvm.org/get_started.html

$directorio_actual = Get-Location

# establecer path donde se instalo visual studio
$vs_path = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community'

# Cambiar la siguiente linea para compilar para x64
# cmd.exe /c "call `"$vs_path\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"
cmd.exe /c "call `"$vs_path\VC\Auxiliary\Build\vcvars32.bat`" && set > %temp%\vcvars.txt"

Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
  if ($_ -match "^(.*?)=(.*)$") {
    Set-Content "env:\$($matches[1])" $matches[2]
  }
}

if (!(Test-Path -Path $directorio_actual\llvm-project)) {
    git clone --config core.autocrlf=false https://github.com/llvm/llvm-project.git --branch release/11.x llvm-project
}

if (!(Test-Path -Path $directorio_actual\llvm-project\build)) {
    New-Item -ItemType directory -Path $directorio_actual\llvm-project\build
}

Set-Location llvm-project\build

cmake -G "Ninja" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_MAKE_PROGRAM=ninja `
    --debug-trycompile -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host `
    -DLLVM_BUILD_EXAMPLES=ON -DLLVM_ENABLE_OCAMLDOC=OFF -DLLVM_BUILD_DOCS=OFF `
    -DLLVM_BUILD_TESTS=ON -DLLVM_ENABLE_PROJECTS='clang' ..\llvm\

# --debug-trycompile -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host `
# --debug-trycompile -DCMAKE_BUILD_TYPE=Debug -DLLVM_TARGETS_TO_BUILD=host `
# --debug-trycompile 
# -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi"
# -DLLVM_ENABLE_BACKTRACES=ON

# correr ninja con 8 hilos
# ninja -j 8
# ninja clang
ninja

Set-Location $directorio_actual