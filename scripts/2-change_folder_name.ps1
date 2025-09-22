# 2-2-change_folder_name.ps1
# Run this from inside ./data
# Usage:
#   Preview:   powershell -ExecutionPolicy Bypass -File .\2-change_folder_name.ps1
#   Execute:   powershell -ExecutionPolicy Bypass -File .\2-change_folder_name.ps1 -Execute

[CmdletBinding()]
param(
    [switch]$Execute
)

Set-StrictMode -Version Latest

# ---------------------------
# LIST_OF_NAMES (id <tab> NAME)
# Replace / extend this block if needed
# ---------------------------
$LIST_OF_NAMES = @"
20242089	BLANCO DOMINGUEZ, GABRIEL ALEXANDER
20242097	LABAN BENAVIDES, DAVE ANDREWS
20242119	HIDALGO FERMIN, BRANDON ANDRES
20242432	PARRA SALAZAR, JOSE FELIPE SEBASTIAN
20242657	HUANI MALPARTIDA, JUSTY EDWARD
20242660	CASTILLO LAYME, LAURA MARCELA
20242804	YACHACHIN PALACIOS, ANTHONY GERARD
20242903	OSORIO LIMACHI, DIEGO DE ALESANDRO
20243316	VELASQUEZ DE LA CRUZ, RICHARD RONALDO
20243352	HUAMANI LOPEZ, FRANCIS
20243449	CCACCYA HUAMANI, ALBERT
20243519	SUPHO CRUZ, FERNANDO JOSE
20243596	ECHE SEMINARIO, DAVID ESTEBAN
20243642	RIQUELME ORTEGA, URIEL ALEXANDER
20244387	LEON CARDENAS, RICHARD
20244418	LEY DOMINGUEZ, ANTONIO ALEJANDRO
20245606	BALAREZO PRECIADO, SEBASTIAN LEONEL
"@


# ---------------------------
# Helpers
# ---------------------------
function Remove-Diacritics {
    param([string]$s)
    if ($null -eq $s) { return $s }
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

function Normalize-ForKey {
    param([string]$s)
    if ($null -eq $s) { return "" }
    $t = Remove-Diacritics($s)
    $t = $t.Trim()
    $t = $t -replace '\s+', ' '           # collapse whitespace
    return $t.ToUpperInvariant()
}

function To-CamelNoSpace {
    param([string]$s)
    if ($null -eq $s) { return "" }
    $t = Remove-Diacritics($s)
    $t = $t -replace '[\t\r\n]+',' '
    $t = $t.Trim()
    $t = $t -replace '\s+', ' '
    if ($t -eq '') { return '' }
    $words = $t -split ' '
    $out = ""
    foreach ($w in $words) {
        # remove non alpha-numeric chars (commas, dots, quotes, etc.)
        $clean = ($w -replace '[^A-Za-z0-9]', '')
        if ($clean -eq '') { continue }
        $lower = $clean.ToLowerInvariant()
        if ($lower.Length -eq 1) {
            $out += $lower.ToUpperInvariant()
        } else {
            $out += $lower.Substring(0,1).ToUpperInvariant() + $lower.Substring(1)
        }
    }
    return $out
}

# ---------------------------
# Build mapping: normalized_name -> @{ id=..., name=... }
# ---------------------------
$map = @{}
foreach ($line in $LIST_OF_NAMES -split "`n") {
    $L = $line.Trim()
    if ([string]::IsNullOrWhiteSpace($L)) { continue }
    if ($L -match '^\s*(\d+)\s+(.+)$') {
        $id = $matches[1]
        $name = $matches[2].Trim()
        $key = Normalize-ForKey($name)
        if (-not $map.ContainsKey($key)) {
            $map[$key] = @{ id = $id; name = $name }
        } else {
            Write-Warning "Duplicate entry for key '$key' (line: $L)"
        }
    } else {
        Write-Warning "Skipping unrecognized mapping line: '$L'"
    }
}

# ---------------------------
# Process directories
# ---------------------------
$dirs = Get-ChildItem -Directory

if ($dirs.Count -eq 0) {
    Write-Host "No directories found in current folder."
    exit 0
}

$renamed = 0
$deleted = 0
$skipped = 0
$failed = 0

foreach ($d in $dirs) {
    $origName = $d.Name

    # Extract the name portion before the first underscore
    $namePart = ($origName -split '_', 2)[0].Trim()
    if ($namePart -eq '') {
        Write-Warning "Cannot extract name from folder '$origName' - skipping."
        $skipped++
        continue
    }

    $normKey = Normalize-ForKey($namePart)

    if ($map.ContainsKey($normKey)) {
        # matched -> rename
        $entry = $map[$normKey]
        $id = $entry.id
        $listName = $entry.name

        # parse last and first names from the listName (the canonical version)
        $last = $null; $first = $null
        if ($listName -match '^(.*?),(.*)$') {
            $last = $matches[1].Trim()
            $first = $matches[2].Trim()
        } elseif ($namePart -match '^(.*?),(.*)$') {
            # fallback to the folder's part if listName lacks comma
            $last = $matches[1].Trim()
            $first = $matches[2].Trim()
        } else {
            # final fallback: split tokens roughly
            $tokens = $namePart -split '\s+'
            if ($tokens.Count -ge 2) {
                $last = ($tokens[0..($tokens.Count-2)] -join ' ')
                $first = $tokens[-1]
            } else {
                Write-Warning "Could not split name into last/first for '$namePart' - skipping."
                $skipped++
                continue
            }
        }

        $lastCamel  = To-CamelNoSpace($last)
        $firstCamel = To-CamelNoSpace($first)
        if ($lastCamel -eq '' -or $firstCamel -eq '') {
            Write-Warning "Empty name parts after processing for '$origName' - skipping."
            $skipped++
            continue
        }

        $newName = "${lastCamel}_${firstCamel}_${id}"
        $targetFull = Join-Path -Path $d.Parent.FullName -ChildPath $newName

        if ($origName -eq $newName) {
            Write-Host "Skipping (already correct): $origName"
            $skipped++
            continue
        }

        if (Test-Path -LiteralPath $targetFull) {
            Write-Warning "Target already exists, skipping rename: '$origName' -> '$newName'"
            $skipped++
            continue
        }

        if ($Execute) {
            try {
                Rename-Item -LiteralPath $d.FullName -NewName $newName -ErrorAction Stop
                Write-Host "Renamed: '$origName' -> '$newName'"
                $renamed++
            } catch {
                Write-Error "Failed to rename '$origName': $_"
                $failed++
            }
        } else {
            Write-Host "(Preview) Rename: '$origName' -> '$newName'"
            $renamed++
        }

    } else {
        # not in list -> delete
        if ($Execute) {
            try {
                Remove-Item -LiteralPath $d.FullName -Recurse -Force -ErrorAction Stop
                Write-Host "Deleted (not in list): '$origName'"
                $deleted++
            } catch {
                Write-Error "Failed to delete '$origName': $_"
                $failed++
            }
        } else {
            Write-Host "(Preview) Would delete (not in list): '$origName'"
            $deleted++
        }
    }
}

Write-Host ""
Write-Host "Summary:"
Write-Host "  Execute mode: $([bool]$Execute)"
Write-Host "  Renames (preview or actual): $renamed"
Write-Host "  Deletes  (preview or actual): $deleted"
Write-Host "  Skipped: $skipped"
Write-Host "  Failures: $failed"
