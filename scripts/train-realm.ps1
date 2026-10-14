param(
    [string]$Python='python',
    [Parameter(Mandatory=$true)][string]$BindingDir,
    [string]$OutputDir='ai/models/realm-v6-rerun',
    [string]$ResumeDir='ai/models/realm-v3'
)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
Push-Location $repo
try {
    $binding=(Resolve-Path -LiteralPath $BindingDir).Path
    $resume=(Resolve-Path -LiteralPath $ResumeDir).Path
    $output=[IO.Path]::GetFullPath($OutputDir)
    if ((Test-Path -LiteralPath $output) -and (Get-ChildItem -LiteralPath $output -Force | Select-Object -First 1)) {throw 'Choose an empty OutputDir; recorded experiments are preserved.'}
    $env:PYTHONPATH=$binding
    & $Python -c 'import polyec_env as c; assert c.REALM_SCHEMA=="realm-v6", "Build the realm-v6 training binding first"'
    if($LASTEXITCODE -ne 0){throw 'Training binding/schema check failed'}
    New-Item -ItemType Directory -Force $output | Out-Null
    Write-Host 'CPU-only PPO: 3 seeds/family, 180/240/240s fit budgets per seed; evaluation and update completion add time.'
    foreach($role in 0,1,2){
        $family=@('governance','economic','civilian')[$role]
        $seconds=@(180,240,240)[$role];$updates=@(240,900,600)[$role]
        $arguments=@((Join-Path $repo 'ai/training/realm_train_v6.py'),'--role',"$role",'--output',(Join-Path $output $family),'--resume-dir',(Join-Path $resume $family),'--seconds',"$seconds",'--updates',"$updates",'--envs','32','--domain','6')
        $quoted=$arguments | ForEach-Object {'"'+$_+'"'}
        $stdout=Join-Path $output "$family.log";$stderr=Join-Path $output "$family.stderr.log"
        $process=Start-Process -FilePath $Python -ArgumentList $quoted -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
        $shown=0
        do {
            $done=$process.WaitForExit(1000)
            $lines=@(Get-Content -LiteralPath $stdout)
            if($lines.Count -gt $shown){$lines[$shown..($lines.Count-1)] | ForEach-Object {Write-Host $_};$shown=$lines.Count}
        }while(-not $done)
        $process.WaitForExit()
        if($process.ExitCode -ne 0){Get-Content -LiteralPath $stderr | Write-Host;throw "PPO $family failed; partial evidence preserved at $output"}
        if((Get-Item -LiteralPath $stderr).Length){Get-Content -LiteralPath $stderr | Write-Host}
    }
}
finally {Pop-Location}
