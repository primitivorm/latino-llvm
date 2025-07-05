# latino-llvm

Implementacion de lenguaje latino utilizando LLVM

# Windows

1. Ir a la carpeta deps/
`cd deps`

2. Clonar el repositorio de llvm-project en 
`git clone --config core.autocrlf=false https://github.com/llvm/llvm-project.git --branch release/11.x llvm-project`

3. Ejecutar script instalar_dependencias_cmake.ps1
`.\instalar_dependencias_cmake.ps1`

4. Ir a carpeta principal
`cd ..`

5. Ejecutar script instalar_debug.ps1
`.\instalar_debug.ps1`