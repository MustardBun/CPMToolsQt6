<#
.SYNOPSIS
    Build the CPMToolsQt6 release archive: runnable Windows build + full source.

.DESCRIPTION
    Produces a single ZIP containing both halves of the release:

        CPMToolsQt6-v0.1-preview/
            CPMToolsQt6.exe        <- ready to run, no install needed
            cpmcli.exe             <- headless engine harness
            diskdefs               <- disk geometry definitions
            Qt6*.dll  platforms/   <- bundled Qt and MinGW runtime
            README.md  LICENSE
            src/                   <- complete source tree

    The binary at the root is the portable build produced by build.sh (or a
    manual CMake build); this script does not compile anything itself. Build
    first, then run this.

.PARAMETER Version
    Version tag used in the folder and archive names. Defaults to v0.1-preview.

.PARAMETER OutputDir
    Where to write the ZIP. Defaults to the repository root.

.PARAMETER BuildDir
    The build directory holding the compiled binary. Defaults to build/.

.PARAMETER KeepStaging
    Leave the staging folder in place instead of deleting it after zipping.

.EXAMPLE
    ./build.sh Release clean
    powershell -ExecutionPolicy Bypass -File package_release.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File package_release.ps1 -Version v0.2-preview

.NOTES
    Copyright (C) 2026 Joseph Kwok (@MustardBun)
    SPDX-License-Identifier: GPL-3.0-or-later
#>
[CmdletBinding()]
param(
    [string] $Version   = 'v0.1-preview',
    [string] $OutputDir = '',
    [string] $BuildDir  = '',
    [switch] $KeepStaging
)

$ErrorActionPreference = 'Stop'

$root = $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = $root }
if ([string]::IsNullOrWhiteSpace($BuildDir))  { $BuildDir  = Join-Path $root 'build' }

$stageName = "CPMToolsQt6-$Version"
$stageRoot = Join-Path $env:TEMP 'CPMToolsQt6-release'
$stageDir  = Join-Path $stageRoot $stageName
$srcDir    = Join-Path $stageDir 'src'
$zipPath   = Join-Path $OutputDir "CPMToolsQt6-$Version.zip"

# ---------------------------------------------------------------------------
# Sanity: the binary must have been built already.
# ---------------------------------------------------------------------------
$exeName = 'CPMToolsQt6.exe'
$exePath = Join-Path $BuildDir $exeName

if (-not (Test-Path -LiteralPath $exePath)) {
    throw @"
$exeName not found in '$BuildDir'.

Build it first, for example:
    ./build.sh Release clean
"@
}

# ---------------------------------------------------------------------------
# What goes in the source tree (under src/) and what never does.
# ---------------------------------------------------------------------------
$srcTopLevelFiles = @(
    'CMakeLists.txt',
    'LICENSE',
    'README.md',
    'CHANGELOG.md',
    'RELEASE_NOTES.md',
    'CONTRIBUTING.md',
    'AGENTS.md',
    'diskdefs',
    'build.sh',
    'package.sh',
    'package_release.ps1',
    '.gitignore',
    '.gitattributes'
)

$srcDirs      = @('qt', 'i18n', 'cmake', 'scripts', 'tools', '.github')
$engineGlobs  = @('*.c', '*.h')
$uiGlobs      = @('*.qrc', '*.ui')

# Never copied into src/.
$excludeDirs = @('.git', '.vscode', 'build', 'dist',
                 'CMakeFiles', '_deps', 'node_modules')
$excludeFilePatterns = @(
    '\.obj$', '\.o$', '\.a$', '\.exe$', '\.dll$', '\.ilk$', '\.pdb$',
    '\.res$', '\.tds$', '\.dcu$', '\.qm$', '\.log$', '\.zip$', '~$'
)

# The original C++Builder/VCL sources are kept in the repository for reference
# only; they are not part of the Qt 6 build.
$vclFiles = @(
    'AboutDlg.cpp', 'AboutDlg.h', 'AboutDlg.dfm',
    'MdiFrame.cpp', 'MdiFrame.h', 'MdiFrame.dfm',
    'MkfsUnit.cpp', 'MkfsUnit.h', 'MkfsUnit.dfm',
    'CpmtoolsGUI.cpp', 'CpmtoolsGUI.mak', 'CpmtoolsGUI.res',
    'clean.bat'
)

# Qt plugin folders the application needs at run time.
$qtPluginDirs = @('platforms', 'styles', 'imageformats', 'iconengines',
                  'tls', 'networkinformation', 'generic')

# Development-only artefacts that must never ship.
$devOnlyFiles = @('uishot.exe')
$devOnlyPlugins = @('platforms\qoffscreen.dll')

function Test-Excluded {
    param([string] $RelativePath)

    foreach ($part in ($RelativePath -split '[\\/]')) {
        if ($excludeDirs -contains $part) { return $true }
    }
    foreach ($pattern in $excludeFilePatterns) {
        if ($RelativePath -match $pattern) { return $true }
    }
    return $false
}

function Copy-SourceTree {
    param([string] $Source, [string] $Destination)

    foreach ($item in Get-ChildItem -LiteralPath $Source -Recurse -File -Force) {
        $relative = $item.FullName.Substring($root.Length).TrimStart('\')
        if (Test-Excluded $relative) { continue }

        $target = Join-Path $Destination $relative
        $parent = Split-Path -Parent $target
        if (-not (Test-Path -LiteralPath $parent)) {
            New-Item -ItemType Directory -Force -Path $parent | Out-Null
        }
        Copy-Item -LiteralPath $item.FullName -Destination $target -Force
    }
}

# ---------------------------------------------------------------------------
Write-Host "CPMToolsQt6 release  $Version"
Write-Host "  source     : $root"
Write-Host "  binary from: $BuildDir"
Write-Host "  archive    : $zipPath"
Write-Host ''

if (Test-Path -LiteralPath $stageDir) {
    Remove-Item -LiteralPath $stageDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
New-Item -ItemType Directory -Force -Path $srcDir   | Out-Null

# ---------------------------------------------------------------------------
# 1. Runnable build at the archive root.
# ---------------------------------------------------------------------------
$binaryCount = 0

# Executables and data files.
foreach ($name in @($exeName, 'cpmcli.exe', 'diskdefs', 'README.md', 'LICENSE')) {
    $source = Join-Path $BuildDir $name
    if (Test-Path -LiteralPath $source) {
        Copy-Item -LiteralPath $source -Destination (Join-Path $stageDir $name) -Force
        $binaryCount++
    } elseif ($name -notin @('cpmcli.exe')) {
        # README/LICENSE may not be copied into build/ by every configuration.
        $fallback = Join-Path $root $name
        if (Test-Path -LiteralPath $fallback) {
            Copy-Item -LiteralPath $fallback -Destination (Join-Path $stageDir $name) -Force
            $binaryCount++
        } else {
            Write-Warning "  missing: $name"
        }
    } else {
        Write-Warning "  missing: $name"
    }
}

# Bundled DLLs (Qt plus the MinGW runtime closure).
foreach ($dll in Get-ChildItem -LiteralPath $BuildDir -Filter *.dll -File) {
    Copy-Item -LiteralPath $dll.FullName -Destination (Join-Path $stageDir $dll.Name) -Force
    $binaryCount++
}

# Qt plugin folders.
foreach ($dir in $qtPluginDirs) {
    $path = Join-Path $BuildDir $dir
    if (Test-Path -LiteralPath $path) {
        Copy-Item -LiteralPath $path -Destination (Join-Path $stageDir $dir) -Recurse -Force
    }
}

# Strip development-only artefacts.
foreach ($name in $devOnlyFiles) {
    $p = Join-Path $stageDir $name
    if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force }
}
foreach ($rel in $devOnlyPlugins) {
    $p = Join-Path $stageDir $rel
    if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force }
}

$dllCount = @(Get-ChildItem -LiteralPath $stageDir -Filter *.dll -File).Count
Write-Host ("  binary  : {0} files ({1} DLLs + plugins)" -f $binaryCount, $dllCount)

# ---------------------------------------------------------------------------
# 2. Source tree under src/.
# ---------------------------------------------------------------------------
$srcCount = 0

foreach ($name in $srcTopLevelFiles) {
    $source = Join-Path $root $name
    if (Test-Path -LiteralPath $source) {
        Copy-Item -LiteralPath $source -Destination (Join-Path $srcDir $name) -Force
        $srcCount++
    } else {
        Write-Warning "  missing source file: $name"
    }
}

foreach ($glob in $engineGlobs) {
    foreach ($file in Get-ChildItem -LiteralPath $root -Filter $glob -File) {
        if ($vclFiles -contains $file.Name) { continue }
        Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $srcDir $file.Name) -Force
        $srcCount++
    }
}

foreach ($dir in $srcDirs) {
    $path = Join-Path $root $dir
    if (-not (Test-Path -LiteralPath $path)) {
        Write-Warning "  missing source directory: $dir"
        continue
    }
    Copy-SourceTree -Source $path -Destination $srcDir
    $srcCount++
}

foreach ($glob in $uiGlobs) {
    foreach ($file in Get-ChildItem -LiteralPath $root -Filter $glob -File -ErrorAction SilentlyContinue) {
        Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $srcDir $file.Name) -Force
        $srcCount++
    }
}

$srcFiles = @(Get-ChildItem -LiteralPath $srcDir -Recurse -File)
Write-Host ("  source  : {0} files" -f $srcFiles.Count)

# ---------------------------------------------------------------------------
# 3. Report and compress.
# ---------------------------------------------------------------------------
Write-Host ''
Write-Host '  archive layout:'
Get-ChildItem -LiteralPath $stageDir | Sort-Object { -not $_.PSIsContainer }, Name | ForEach-Object {
    if ($_.PSIsContainer) {
        $n = @(Get-ChildItem -LiteralPath $_.FullName -Recurse -File).Count
        "    [dir] {0}/  ({1} files)" -f $_.Name, $n
    } else {
        "          $($_.Name)"
    }
}

if (Test-Path -LiteralPath $zipPath) { Remove-Item -LiteralPath $zipPath -Force }

Write-Host ''
Write-Host '  compressing...'
Compress-Archive -LiteralPath $stageDir -DestinationPath $zipPath -CompressionLevel Optimal

if (-not $KeepStaging) {
    Remove-Item -LiteralPath $stageDir -Recurse -Force -ErrorAction SilentlyContinue
    if ((Test-Path -LiteralPath $stageRoot) -and
        -not (Get-ChildItem -LiteralPath $stageRoot -Force)) {
        Remove-Item -LiteralPath $stageRoot -Force -ErrorAction SilentlyContinue
    }
}

$zip = Get-Item -LiteralPath $zipPath
Write-Host ''
Write-Host ("Done: {0}  ({1:N1} MB)" -f $zip.FullName, ($zip.Length / 1MB))
