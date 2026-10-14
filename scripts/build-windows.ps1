param(
    [string]$CMake = 'cmake',
    [string]$BuildDir = 'build/native',
    [string]$ToolchainFile = '',
    [int]$Jobs = 4,
    [string]$OrtRoot = ''
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    if (-not (Test-Path 'third_party/godot-cpp/CMakeLists.txt')) {
        throw 'Missing godot-cpp. Run: git clone --depth 1 --branch godot-4.5-stable https://github.com/godotengine/godot-cpp third_party/godot-cpp'
    }
    $configureArgs = @('-S', $projectRoot, '-B', $BuildDir, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release')
    if ($ToolchainFile) { $configureArgs += "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile" }
    if ($OrtRoot) { $configureArgs += @('-DPOLYEC_BUILD_AI=ON',"-DORT_ROOT=$OrtRoot") }
    else { $configureArgs += '-DPOLYEC_BUILD_AI=OFF' }
    & $CMake @configureArgs
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }
    & $CMake --build $BuildDir --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
    Write-Host 'Built native DLL, core test executable and Windows launcher.'
}
finally { Pop-Location }
