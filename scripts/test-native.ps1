param(
    [Parameter(Mandatory=$true)][string]$Godot,
    [string]$CTest = 'ctest',
    [string]$BuildDir = 'build/native',
    [string]$Config = 'Release',
    [switch]$IncludeRL,
    [switch]$IncludeRealm
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    & $CTest --test-dir $BuildDir -C $Config --output-on-failure -V
    if ($LASTEXITCODE -ne 0) { throw 'Core regressions failed' }
    $smokes=@('native_smoke','ui_smoke','campaign_smoke')
    if ($IncludeRL) { $smokes += 'rl_smoke' }
    if ($IncludeRealm) { $smokes += 'realm_smoke' }
    $logDir=Join-Path ([IO.Path]::GetFullPath($BuildDir)) 'test-logs'
    New-Item -ItemType Directory -Force $logDir | Out-Null
    foreach ($smoke in $smokes) {
        $outLog=Join-Path $logDir "$smoke.stdout.log"
        $errLog=Join-Path $logDir "$smoke.stderr.log"
        $run=Start-Process -FilePath $Godot -ArgumentList @('--headless','--path',('"'+(Join-Path $projectRoot 'godot_project')+'"'),'--script',"res://tests/$smoke.gd") -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $outLog -RedirectStandardError $errLog
        $output=(Get-Content -LiteralPath $outLog,$errLog -Raw | Out-String)
        Write-Host $output
        if ($run.ExitCode -ne 0 -or $output -match '(?m)^(SCRIPT ERROR:|ERROR:)' -or $output -notmatch 'SMOKE: \d+/\d+ checks passed') { throw "$smoke failed" }
    }
}
finally { Pop-Location }
