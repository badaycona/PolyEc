# Keep the project model and Windows instructions; skip global external plugins.
Push-Location $PSScriptRoot
try {
    & opencode --pure --agent build @args
    $openCodeExit = $LASTEXITCODE
}
finally {
    Pop-Location
}
exit $openCodeExit
