param(
    [string]$Python='python',
    [string]$CMake='cmake',
    [string]$BuildDir='build/training',
    [string]$ToolchainFile='',
    [string]$OrtRoot=''
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    $pythonPath=& $Python -c 'import sys; print(sys.executable)'
    if ($LASTEXITCODE -ne 0) {throw 'Python unavailable'}
    $pybindDir=& $Python -m pybind11 --cmakedir
    if ($LASTEXITCODE -ne 0) {throw 'Install ai/requirements.txt in the training environment'}
    $configure=@('-S',$projectRoot,'-B',$BuildDir,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DPOLYEC_BUILD_APP=OFF','-DPOLYEC_BUILD_PYTHON=ON',"-DPython3_EXECUTABLE=$pythonPath","-Dpybind11_DIR=$pybindDir")
    if ($ToolchainFile) {$configure += "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile"}
    if ($OrtRoot) {$configure += @('-DPOLYEC_BUILD_AI=ON',"-DORT_ROOT=$OrtRoot")}
    else {$configure += '-DPOLYEC_BUILD_AI=OFF'}
    & $CMake @configure
    if ($LASTEXITCODE -ne 0) {throw 'Training configure failed'}
    & $CMake --build $BuildDir --parallel 4
    if ($LASTEXITCODE -ne 0) {throw 'Training binding build failed'}
    if ($OrtRoot) {
        Copy-Item -LiteralPath (Join-Path $OrtRoot 'lib/onnxruntime.dll'),(Join-Path $OrtRoot 'lib/onnxruntime_providers_shared.dll') -Destination $BuildDir
    }
}
finally {Pop-Location}
