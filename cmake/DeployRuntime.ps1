<#
.SYNOPSIS
    Recursively copies the non-system DLL dependency closure of a bundle.

.DESCRIPTION
    windeployqt copies the Qt libraries and plugins, but not the MinGW
    dependencies those libraries need (libstdc++-6, libgcc_s_seh-1,
    libwinpthread-1, libicu*, libharfbuzz*, libpng*, libb2, ...).

    This script walks the PE import table of every executable and DLL already in
    the target folder using objdump, resolves each imported name against the
    supplied search directories, copies what it finds and repeats until the
    closure is complete. Anything that cannot be resolved is a Windows system
    library and is left alone.

.PARAMETER TargetDir
    The folder to make self-contained.

.PARAMETER SearchDirs
    Directories to resolve DLL names from, in priority order, separated by
    semicolons. Normally the Qt bin directory and the MinGW toolchain bin
    directory.

.PARAMETER Objdump
    Path to objdump.exe (comes with binutils/GCC).
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string] $TargetDir,
    [Parameter(Mandatory = $true)][string] $SearchDirs,
    [Parameter(Mandatory = $true)][string] $Objdump
)

$ErrorActionPreference = 'Stop'

$TargetDir = (Resolve-Path -LiteralPath $TargetDir).Path

$validSearchDirs = @()
foreach ($dir in ($SearchDirs -split ';')) {
    $dir = $dir.Trim()
    if ($dir -and (Test-Path -LiteralPath $dir)) {
        $validSearchDirs += (Resolve-Path -LiteralPath $dir).Path
    }
}
if ($validSearchDirs.Count -eq 0) {
    throw "No valid search directories supplied."
}
if (-not (Test-Path -LiteralPath $Objdump)) {
    throw "objdump not found at '$Objdump'."
}

Write-Host "Deploying DLL closure into: $TargetDir"
Write-Host "  search: $($validSearchDirs -join '; ')"

function Get-PeImports {
    param([string] $Path)

    # objdump prints one "DLL Name: X.dll" line per import descriptor.
    $output = & $Objdump -p $Path 2>$null
    $names = @()
    foreach ($line in $output) {
        if ($line -match 'DLL Name:\s*(\S+)') {
            $names += $Matches[1]
        }
    }
    return $names
}

function Resolve-Dependency {
    param([string] $Name)

    foreach ($dir in $validSearchDirs) {
        $candidate = Join-Path $dir $Name
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }
    return $null
}

# Seed the queue with everything already in the target folder.
$queue   = [System.Collections.Generic.Queue[string]]::new()
$visited = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$copied  = [System.Collections.Generic.List[string]]::new()

foreach ($file in Get-ChildItem -LiteralPath $TargetDir -Recurse -File) {
    if ($file.Extension -in '.exe', '.dll') {
        [void]$queue.Enqueue($file.FullName)
        [void]$visited.Add($file.Name)
    }
}

$unresolved = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)

while ($queue.Count -gt 0) {
    $current = $queue.Dequeue()

    foreach ($dep in Get-PeImports -Path $current) {
        if ($visited.Contains($dep)) { continue }
        [void]$visited.Add($dep)

        $resolved = Resolve-Dependency -Name $dep
        if ($null -eq $resolved) {
            # Not in any toolchain directory: a Windows system library.
            [void]$unresolved.Add($dep)
            continue
        }

        $dest = Join-Path $TargetDir $dep
        Copy-Item -LiteralPath $resolved -Destination $dest -Force
        $copied.Add($dep)
        Write-Host ("  + {0}" -f $dep)
        [void]$queue.Enqueue($dest)
    }
}

Write-Host ""
Write-Host ("Copied {0} runtime DLL(s)." -f $copied.Count)
if ($unresolved.Count -gt 0) {
    Write-Host ("Left to the operating system ({0}):" -f $unresolved.Count)
    Write-Host ("  " + (($unresolved | Sort-Object) -join ', '))
}
