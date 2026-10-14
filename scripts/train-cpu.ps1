param(
    [string]$Python='python',
    [string]$BindingDir='build/training',
    [string]$OutputDir='artifacts/models/economic-v3',
    [string]$ResumeDir='',
    [int]$Updates=240,
    [int]$Seconds=600,
    [int]$TestStart=20000,
    [int]$Domain=0
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$previousPythonPath=$env:PYTHONPATH
$previousOMP=$env:OMP_NUM_THREADS
$previousMKL=$env:MKL_NUM_THREADS
Push-Location $projectRoot
try {
    $output=[IO.Path]::GetFullPath($OutputDir)
    if ((Test-Path -LiteralPath $output) -and (Get-ChildItem -LiteralPath $output -Force | Select-Object -First 1)) {throw 'Choose an empty OutputDir; recorded experiments are preserved.'}
    $env:PYTHONPATH=[IO.Path]::GetFullPath($BindingDir)
    $env:OMP_NUM_THREADS='2'
    $env:MKL_NUM_THREADS='2'
    $trainArgs=@('-u','ai/training/train.py','--output',$OutputDir,'--updates',$Updates,'--envs',64,'--seconds',$Seconds,'--test-start',$TestStart,'--domain',$Domain)
    if ($ResumeDir) {$trainArgs += @('--resume-dir',$ResumeDir)}
    & $Python @trainArgs
    if ($LASTEXITCODE -ne 0) {throw 'PPO/evaluation/export failed'}
}
finally {
    $env:PYTHONPATH=$previousPythonPath
    $env:OMP_NUM_THREADS=$previousOMP
    $env:MKL_NUM_THREADS=$previousMKL
    Pop-Location
}
