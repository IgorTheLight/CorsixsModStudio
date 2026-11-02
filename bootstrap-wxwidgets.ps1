#Requires -Version 5.1
<#
.SYNOPSIS
    Builds the wxWidgets 2.8.12 Unicode static Win32 libraries used by Corsix's Mod Studio.

.DESCRIPTION
    Prepares a wxWidgets 2.8.12 source tree under out\deps\wxWidgets-2.8.12,
    applies the wxPropertyGrid 1.2.6 patches kept in
    third_party\wxpropgrid-modifications, and compiles wxWidgets, wxSTC and
    wxPropertyGrid with nmake from the Visual Studio C++ toolchain.

    When the source tree is missing, the official wxWidgets 2.8.12 release
    archive is downloaded and unpacked first, so a fresh clone needs nothing
    but Visual Studio and a network connection (or -Source).

.PARAMETER Configuration
    Which wxWidgets flavours to build: Release, Debug, or both. Release is
    enough for the Release and RelWithDebInfo application builds; Debug is
    only needed for debugging into wxWidgets.

.PARAMETER WxRoot
    Directory that holds (or receives) the wxWidgets 2.8.12 source tree.

.PARAMETER Source
    Path to an already downloaded wxWidgets 2.8.12 source .zip. The archive is
    used instead of downloading one.

.PARAMETER DownloadUrl
    Archive to download when no source tree and no -Source is available.

.PARAMETER PrepareOnly
    Download, unpack and patch the sources, then stop before compiling.

.PARAMETER Force
    Rebuild even when the libraries look complete.

.EXAMPLE
    .\bootstrap-wxwidgets.ps1 -Configuration Release
#>
param(
    [ValidateSet('Release', 'Debug')]
    [string[]] $Configuration = @('Release', 'Debug'),
    [string] $WxRoot = '',
    [string] $Source = '',
    [string] $DownloadUrl = 'https://github.com/wxWidgets/wxWidgets/releases/download/v2.8.12/wxWidgets-2.8.12.zip',
    [string] $ExpectedSha256 = '57D3B406EFF18FCFE2C4E715F0C3E03EE4900BEE26F6871A6947DDE2B85C319D',
    [switch] $PrepareOnly,
    [switch] $Force
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $WxRoot) {
    $WxRoot = Join-Path $repoRoot 'out\deps\wxWidgets-2.8.12'
}
$WxRoot = [System.IO.Path]::GetFullPath($WxRoot)

function Get-WxSourceArchive {
    param(
        [string] $Url,
        [string] $Sha256,
        [string] $DestinationDirectory
    )

    New-Item -ItemType Directory -Path $DestinationDirectory -Force | Out-Null
    $archive = Join-Path $DestinationDirectory ([System.IO.Path]::GetFileName(([uri] $Url).AbsolutePath))
    if (Test-Path -LiteralPath $archive) {
        if (-not $Sha256 -or (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -eq $Sha256) {
            Write-Host "Using cached archive $archive"
            return $archive
        }
        Write-Host 'Cached archive does not match the expected checksum; downloading it again.'
        Remove-Item -LiteralPath $archive -Force
    }

    Write-Host "Downloading $Url"
    try {
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    }
    catch {
        # PowerShell 7 ignores this setting; the assignment is only for 5.1.
    }
    Invoke-WebRequest -Uri $Url -OutFile $archive -UseBasicParsing

    if ($Sha256) {
        $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
        if ($actual -ne $Sha256) {
            Remove-Item -LiteralPath $archive -Force
            throw "Checksum mismatch for $Url. Expected $Sha256 but downloaded $actual."
        }
    }
    return $archive
}

function Expand-WxSourceArchive {
    param(
        [string] $ArchivePath,
        [string] $Destination
    )

    $staging = Join-Path ([System.IO.Path]::GetTempPath()) ("cdms-wx-unpack-{0}" -f [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $staging -Force | Out-Null
    try {
        Write-Host "Unpacking $ArchivePath"
        Expand-Archive -LiteralPath $ArchivePath -DestinationPath $staging -Force
        $unpacked = Get-ChildItem -LiteralPath $staging -Directory |
            Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'include\wx\wx.h') } |
            Select-Object -First 1
        if (-not $unpacked) {
            throw "$ArchivePath does not contain a wxWidgets source tree (include\wx\wx.h)."
        }
        New-Item -ItemType Directory -Path $Destination -Force | Out-Null
        Get-ChildItem -LiteralPath $unpacked.FullName -Force |
            Copy-Item -Destination $Destination -Recurse -Force
    }
    finally {
        Remove-Item -LiteralPath $staging -Recurse -Force -ErrorAction SilentlyContinue
    }
}

if (-not (Test-Path -LiteralPath (Join-Path $WxRoot 'include\wx\wx.h'))) {
    if ($Source) {
        if (-not (Test-Path -LiteralPath $Source)) {
            throw "-Source points at $Source, which does not exist."
        }
        $archive = (Resolve-Path -LiteralPath $Source).Path
    }
    else {
        $archive = Get-WxSourceArchive -Url $DownloadUrl -Sha256 $ExpectedSha256 `
            -DestinationDirectory (Join-Path $repoRoot 'out\downloads')
    }
    Expand-WxSourceArchive -ArchivePath $archive -Destination $WxRoot
}

# wxPropertyGrid 1.2.6 was a stand-alone library and is not part of any
# wxWidgets 2.8.x archive, so the sources this project builds are bundled.
$bundledPropgrid = Join-Path $repoRoot 'third_party\wxpropgrid'
if (Test-Path -LiteralPath $bundledPropgrid) {
    foreach ($pair in @(
            @{ From = 'include\wx\propgrid'; To = (Join-Path $WxRoot 'contrib\include\wx\propgrid') },
            @{ From = 'src\propgrid'; To = (Join-Path $WxRoot 'contrib\src\propgrid') },
            @{ From = 'build\propgrid'; To = (Join-Path $WxRoot 'contrib\build\propgrid') }
        )) {
        $from = Join-Path $bundledPropgrid $pair.From
        if (-not (Test-Path -LiteralPath $from)) {
            continue
        }
        New-Item -ItemType Directory -Path $pair.To -Force | Out-Null
        Get-ChildItem -LiteralPath $from -File | Copy-Item -Destination $pair.To -Force
    }
    Write-Host "Installed the bundled wxPropertyGrid 1.2.6 sources into $WxRoot"
}

$patchDir = Join-Path $repoRoot 'third_party\wxpropgrid-modifications'
$propgridSrc = Join-Path $WxRoot 'contrib\src\propgrid'
$propgridInc = Join-Path $WxRoot 'contrib\include\wx\propgrid'
foreach ($pair in @(
        @{ Name = 'propgrid.cpp'; Dest = $propgridSrc },
        @{ Name = 'manager.cpp'; Dest = $propgridSrc },
        @{ Name = 'propgrid.h'; Dest = $propgridInc }
    )) {
    $source = Join-Path $patchDir $pair.Name
    if (-not (Test-Path -LiteralPath $source)) {
        throw "Missing patch file $source"
    }
    if (-not (Test-Path -LiteralPath $pair.Dest)) {
        throw "wxPropertyGrid 1.2.6 is not installed under $($pair.Dest)"
    }
    Copy-Item -LiteralPath $source -Destination (Join-Path $pair.Dest $pair.Name) -Force
}

$msw = Join-Path $WxRoot 'build\msw'
$stc = Join-Path $WxRoot 'contrib\build\stc'
$propgrid = Join-Path $WxRoot 'contrib\build\propgrid'
foreach ($dir in @($msw, $stc, $propgrid)) {
    if (-not (Test-Path -LiteralPath (Join-Path $dir 'makefile.vc'))) {
        throw "Missing makefile.vc in $dir"
    }
}

if ($PrepareOnly) {
    Write-Host "wxWidgets 2.8.12 sources are ready under $WxRoot"
    return
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'vswhere.exe was not found. Install Visual Studio with the C++ workload.'
}

$vsDevCmd = & $vswhere -latest -products '*' -find 'Common7\Tools\VsDevCmd.bat' |
    Where-Object { Test-Path -LiteralPath $_ } |
    Select-Object -First 1
if (-not $vsDevCmd) {
    throw 'VsDevCmd.bat was not found. Install the Visual Studio C++ tools.'
}

function Test-WxBuilt([string] $Build) {
    $libDir = Join-Path $WxRoot 'lib\vc_lib'
    if ($Build -eq 'Debug') {
        $needed = @(
            'wxmsw28ud_core.lib', 'wxbase28ud.lib', 'wxmsw28ud_adv.lib',
            'wxmsw28ud_stc.lib', 'wxmsw28ud_propgrid.lib',
            'mswud\wx\setup.h'
        )
    }
    else {
        $needed = @(
            'wxmsw28u_core.lib', 'wxbase28u.lib', 'wxmsw28u_adv.lib',
            'wxmsw28u_stc.lib', 'wxmsw28u_propgrid.lib',
            'mswu\wx\setup.h'
        )
    }
    foreach ($name in $needed) {
        if (-not (Test-Path -LiteralPath (Join-Path $libDir $name))) {
            return $false
        }
    }
    return $true
}

function Invoke-WxNmake([string] $Build, [string] $Directory) {
    $quotedDir = $Directory.Replace('"', '')
    $quotedVs = $vsDevCmd.Replace('"', '')
    $script = Join-Path ([System.IO.Path]::GetTempPath()) ("cdms-wx-nmake-{0}.cmd" -f [guid]::NewGuid().ToString('N'))
    @(
        '@echo off'
        "call `"$quotedVs`" -arch=x86 -host_arch=x64"
        'if errorlevel 1 exit /b 1'
        "cd /d `"$quotedDir`""
        "nmake /nologo -f makefile.vc BUILD=$Build SHARED=0 UNICODE=1 CPPFLAGS=/DNEED_PBT_H"
        'exit /b %ERRORLEVEL%'
    ) | Set-Content -LiteralPath $script -Encoding ASCII
    Write-Host "nmake ($Build) in $quotedDir"
    try {
        cmd.exe /c "`"$script`""
        if ($LASTEXITCODE -ne 0) {
            throw "nmake $Build failed in $quotedDir (exit $LASTEXITCODE)"
        }
    }
    finally {
        Remove-Item -LiteralPath $script -ErrorAction SilentlyContinue
    }
}

foreach ($build in $Configuration) {
    $nmakeBuild = $build.ToLowerInvariant()
    if (-not $Force -and (Test-WxBuilt $build)) {
        Write-Host "Skipping $build; libraries already exist. Pass -Force to rebuild."
        continue
    }
    Invoke-WxNmake $nmakeBuild $msw
    Invoke-WxNmake $nmakeBuild $stc
    Invoke-WxNmake $nmakeBuild $propgrid
}

Write-Host "wxWidgets Unicode Win32 libraries are ready under $(Join-Path $WxRoot 'lib\vc_lib')"
