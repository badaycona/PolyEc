param(
    [string]$Python='python',
    [Parameter(Mandatory=$true)][string]$BindingDir,
    [string]$OutputDir='ai/models/realm-v3',
    [string]$ResumeRoot='',
    [string]$Schema='realm-v3',
    [int]$Updates=90,
    [int]$Seconds=90,
    [int]$TestStart=80000,
    [int]$Domain=6
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$previousPythonPath=$env:PYTHONPATH
Push-Location $projectRoot
try {
    $env:PYTHONPATH=[IO.Path]::GetFullPath($BindingDir)
    $output=[IO.Path]::GetFullPath($OutputDir)
    New-Item -ItemType Directory -Force $output | Out-Null
    foreach ($role in 0,1,2) {
        $family=@('governance','economic','civilian')[$role]
        $arguments=@('ai/training/realm_train.py','--schema',$Schema,'--role',$role,'--output',(Join-Path $output $family),'--updates',$Updates,'--seconds',$Seconds,'--test-start',$TestStart,'--domain',$Domain)
        if ($ResumeRoot) {$arguments+=@('--resume-dir',(Join-Path $ResumeRoot $family))}
        & $Python @arguments 2>&1 | Tee-Object -FilePath (Join-Path $output "$family-training.log")
        if ($LASTEXITCODE -ne 0) {throw "Training failed for $family"}
    }
}
finally {$env:PYTHONPATH=$previousPythonPath;Pop-Location}
