#Requires -Version 5.1
<#
.SYNOPSIS
    Builds Corsix's Mod Studio and lays out a runnable folder (or .zip) for release.

.DESCRIPTION
    Runs the normal Win32 build, then uses the CMake install rules to copy the
    executable, Rainman.dll, the Lua/Squish runtime libraries and the
    Mod_Studio_Files data directory into a single self-contained folder under
    out\dist. The layout matches what the application expects at run time: the
    executable next to Mod_Studio_Files, with lua512p.dll (Release) or
    debug\lua512d.dll (Debug) inside it.

.PARAMETER Configuration
    Configuration to package. Debug packages get a -debug suffix.

.PARAMETER OutputDirectory
    Where the package folder and archive are written. Defaults to out\dist.

.PARAMETER SkipBuild
    Package the existing build output without rebuilding first.

.PARAMETER NoArchive
    Only create the folder; do not compress it into a .zip.

.EXAMPLE
    .\package-win32.ps1

.EXAMPLE
    .\package-win32.ps1 -Configuration Debug -NoArchive
#>
param(
    [ValidateSet('Release', 'Debug', 'RelWithDebInfo')]
    [string] $Configuration = 'Release',
    [string] $OutputDirectory = '',
    [switch] $SkipBuild,
    [switch] $NoArchive
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $repoRoot 'out\dist'
}

$cmakeLists = Get-Content -LiteralPath (Join-Path $repoRoot 'CMakeLists.txt') -Raw
$version = '0.5.6'
if ($cmakeLists -match 'project\s*\(\s*CorsixsModStudio\s+VERSION\s+([0-9][0-9.]*)') {
    $version = $Matches[1]
}

if (-not $SkipBuild) {
    & (Join-Path $repoRoot 'build-win32.ps1') -Configuration $Configuration
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE."
    }
}

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
    throw 'CMake was not found in Visual Studio or PATH.'
}

$buildDir = Join-Path $repoRoot 'out\build\windows-wx28-win32'
if (-not (Test-Path -LiteralPath (Join-Path $buildDir 'CMakeCache.txt'))) {
    throw "No configured build found at $buildDir. Run .\build-win32.ps1 first."
}

$suffix = ''
if ($Configuration -ne 'Release') {
    $suffix = '-' + $Configuration.ToLowerInvariant()
}
$packageName = "CorsixsModStudio-$version-win32$suffix"
$staging = [System.IO.Path]::GetFullPath((Join-Path $OutputDirectory $packageName))
if (-not $staging.StartsWith([System.IO.Path]::GetFullPath($repoRoot), [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to write the package outside the repository: $staging"
}

if (Test-Path -LiteralPath $staging) {
    Remove-Item -LiteralPath $staging -Recurse -Force
}
New-Item -ItemType Directory -Path $staging -Force | Out-Null

& $cmakePath --install $buildDir --config $Configuration --prefix $staging
if ($LASTEXITCODE -ne 0) {
    throw "cmake --install failed with exit code $LASTEXITCODE."
}

$required = @(
    'CorsixsModStudio.exe',
    'Rainman.dll',
    'Mod_Studio_Files\config.txt',
    'Mod_Studio_Files\squish.dll',
    'Mod_Studio_Files\lua512.dll'
)
# Rainman.dll imports the Lua DLL, so a copy must sit beside the executable;
# the application separately loads the renamed copies from Mod_Studio_Files.
if ($Configuration -eq 'Debug') {
    $required += 'lua512d.dll'
    $required += 'Mod_Studio_Files\debug\lua512d.dll'
}
else {
    $required += 'lua512.dll'
    $required += 'Mod_Studio_Files\lua512p.dll'
}
$missing = @($required | Where-Object { -not (Test-Path -LiteralPath (Join-Path $staging $_)) })
if ($missing.Count -gt 0) {
    throw "The package is incomplete; missing: $($missing -join ', ')"
}

$fileCount = (Get-ChildItem -LiteralPath $staging -Recurse -File).Count
Write-Host "Package folder: $staging ($fileCount files)"

if (-not $NoArchive) {
    $archive = "$staging.zip"
    if (Test-Path -LiteralPath $archive) {
        Remove-Item -LiteralPath $archive -Force
    }
    Compress-Archive -Path (Join-Path $staging '*') -DestinationPath $archive -CompressionLevel Optimal
    Write-Host "Package archive: $archive"
}
