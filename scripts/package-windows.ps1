param(
    [Parameter(Mandatory=$true)][string]$Godot,
    [string]$BuildDir = 'build/native',
    [string]$OutputDir = 'dist/PolyEc-M0a-Windows',
    [switch]$IncludeRL,
    [switch]$IncludeRealm,
    [switch]$IncludeM4,
    [string]$Config = 'Release'
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    $output = [IO.Path]::GetFullPath($OutputDir)
    $godotRuntime = (Resolve-Path -LiteralPath $Godot).Path
    if ([IO.Path]::GetFileNameWithoutExtension($godotRuntime).EndsWith('_console')) {
        $godotRuntime = $godotRuntime -replace '_console\.exe$', '.exe'
        if (-not (Test-Path -LiteralPath $godotRuntime)) { throw 'Godot GUI runtime is missing beside console wrapper' }
    }
    $launcher = Join-Path $BuildDir 'PolyEc.exe'
    if (-not (Test-Path -LiteralPath $launcher)) { $launcher = Join-Path (Join-Path $BuildDir $Config) 'PolyEc.exe' }
    $library = 'godot_project/bin/polyec.windows.template_release.x86_64.dll'
    foreach ($required in @($Godot,$launcher,$library)) {
        if (-not (Test-Path -LiteralPath $required)) { throw "Missing build artifact: $required" }
    }
    New-Item -ItemType Directory -Force $output,(Join-Path $output 'bin'),(Join-Path $output 'licenses') | Out-Null
    & $Godot --headless --path (Join-Path $projectRoot 'godot_project') --export-pack Windows (Join-Path $output 'PolyEc-runtime.pck')
    if ($LASTEXITCODE -ne 0) { throw 'Godot resource pack export failed' }
    Copy-Item -LiteralPath $godotRuntime -Destination (Join-Path $output 'PolyEc-runtime.exe')
    Copy-Item -LiteralPath $launcher -Destination (Join-Path $output 'PolyEc.exe')
    Copy-Item -LiteralPath $library -Destination (Join-Path $output 'bin')
    if ($IncludeRL -or $IncludeRealm) {
        foreach ($runtimeDependency in @('onnxruntime.dll','onnxruntime_providers_shared.dll')) {
            $dependency=Join-Path 'godot_project/bin' $runtimeDependency
            if (-not (Test-Path -LiteralPath $dependency)) { throw "Missing native AI dependency: $dependency" }
            Copy-Item -LiteralPath $dependency -Destination (Join-Path $output 'bin')
        }
        New-Item -ItemType Directory -Force (Join-Path $output 'models') | Out-Null
        foreach ($modelFile in @('economic.onnx','manifest.json','evaluation.json','native-parity.json')) {
            Copy-Item -LiteralPath (Join-Path 'godot_project/models' $modelFile) -Destination (Join-Path $output 'models')
        }
        if ($IncludeRealm) {
            Copy-Item -LiteralPath 'godot_project/models/realm-v6' -Destination (Join-Path $output 'models') -Recurse
        }
    }
    Copy-Item -Path 'licenses/*' -Destination (Join-Path $output 'licenses')
    $playerReadme=if ($IncludeM4) {'docs/guides/PLAYER-README-M4.txt'} elseif ($IncludeRealm) {'docs/guides/PLAYER-README-REALM.txt'} elseif ($IncludeRL) {'docs/guides/PLAYER-README-RL.txt'} else {'docs/guides/PLAYER-README.txt'}
    Copy-Item -LiteralPath $playerReadme -Destination (Join-Path $output 'READ-ME.txt')
    if ($IncludeM4 -and (Test-Path docs/reports/M4-evidence.md)) {Copy-Item docs/reports/M4-evidence.md (Join-Path $output "BUILD-REPORT.md")}
    elseif ($IncludeRealm -and (Test-Path docs/reports/M3-evidence.md)) {Copy-Item docs/reports/M3-evidence.md (Join-Path $output "BUILD-REPORT.md")}
    Write-Host "Portable app ready: $output/PolyEc.exe"
}
finally { Pop-Location }
