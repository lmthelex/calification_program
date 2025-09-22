# 1-delete_tildes.ps1
# Run this from inside ./data
# Usage: powershell -ExecutionPolicy Bypass -File .\1-delete_tildes.ps1 -Execute

[CmdletBinding()]
param(
    [switch]$Execute,
    [switch]$Recurse
)

Set-StrictMode -Version Latest

function Remove-Diacritics {
    param([string]$s)
    if ($null -eq $s) { return $s }

    # Normalize to FormD (decomposed), remove NonSpacingMark chars, then recompose to FormC.
    $norm = $s.Normalize([System.Text.NormalizationForm]::FormD)
    $sb = New-Object System.Text.StringBuilder
    foreach ($ch in $norm.ToCharArray()) {
        $cat = [System.Globalization.CharUnicodeInfo]::GetUnicodeCategory($ch)
        if ($cat -ne [System.Globalization.UnicodeCategory]::NonSpacingMark) {
            [void]$sb.Append($ch)
        }
    }
    return $sb.ToString().Normalize([System.Text.NormalizationForm]::FormC)
}

# Collect directories (children or recursive)
$dirs = Get-ChildItem -Directory -Recurse:$Recurse

if ($dirs.Count -eq 0) {
    Write-Host "No directories found."
    exit 0
}

# Rename deeper paths first to avoid parent/child name collisions
$dirs = $dirs | Sort-Object { $_.FullName.Length } -Descending

$renamed = 0
$skipped  = 0
$failed   = 0

foreach ($d in $dirs) {
    $oldName = $d.Name
    $newName = Remove-Diacritics $oldName

    if ($oldName -eq $newName) {
        $skipped++
        continue
    }

    $parentPath = $d.Parent.FullName
    $targetPath = Join-Path -Path $parentPath -ChildPath $newName

    if (Test-Path -LiteralPath $targetPath) {
        Write-Warning "Target already exists, skipping: `"$oldName`" -> `"$newName`""
        $skipped++
        continue
    }

    if ($Execute) {
        try {
            Rename-Item -LiteralPath $d.FullName -NewName $newName -ErrorAction Stop
            Write-Host "Renamed: `"$oldName`" -> `"$newName`""
            $renamed++
        } catch {
            Write-Error "Failed to rename `"$oldName`": $_"
            $failed++
        }
    } else {
        Write-Host "(Preview) `"$oldName`" -> `"$newName`""
        $renamed++
    }
}

Write-Host ""
Write-Host "Summary: Preview mode = $([bool] -not $Execute). Renames processed (preview count or actual): $renamed. Skipped: $skipped. Failures: $failed."
