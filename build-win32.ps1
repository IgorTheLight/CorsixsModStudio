#Requires -Version 5.1
param(
    [ValidateSet('Release', 'Debug', 'RelWithDebInfo')]
    [string] $Configuration = 'Release',
    [switch] $BootstrapWx
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$cmakePath = $null

if (Test-Path -LiteralPath $vswhere) {
    $cmakePath = & $vswhere -latest -products '*' -find 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' |
        Where-Object { Test-Path -LiteralPath $_ } |
        Select-Object -First 1
}

if (-not $cmakePath) {
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmakeCommand) {
        $cmakePath = $cmakeCommand.Source
    }
}

if (-not $cmakePath) {
    throw 'CMake was not found in Visual Studio or PATH. Install the CMake component in Visual Studio Installer.'
}

if ($BootstrapWx) {
    $wxConfig = @('Release')
    if ($Configuration -eq 'Debug') {
        $wxConfig = @('Release', 'Debug')
    }
    & (Join-Path $repoRoot 'bootstrap-wxwidgets.ps1') -Configuration $wxConfig
}

$buildPreset = switch ($Configuration) {
    'Debug' { 'windows-wx28-win32-debug' }
    'RelWithDebInfo' { 'windows-wx28-win32-relwithdebinfo' }
    default { 'windows-wx28-win32-release' }
}

Push-Location $repoRoot
try {
    & $cmakePath --preset windows-wx28-win32
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    & $cmakePath --build --preset $buildPreset
    $buildExit = $LASTEXITCODE
    if ($buildExit -ne 0 -and $Configuration -eq 'Debug') {
        Write-Host ''
        Write-Host 'CMake only generates the Debug configuration when the Debug wxWidgets libraries are present.' -ForegroundColor Yellow
        Write-Host 'Run .\bootstrap-wxwidgets.ps1 -Configuration Debug, then try again.' -ForegroundColor Yellow
    }
    exit $buildExit
}
finally {
    Pop-Location
}
