# 3-create_rubrics.ps1
# Run this from inside ./data
# Usage:
#   Preview/create (no overwrite): powershell -ExecutionPolicy Bypass -File .\3-create_rubrics.ps1
#   Create and overwrite existing rubric files: powershell -ExecutionPolicy Bypass -File .\3-create_rubrics.ps1 -Force

[CmdletBinding()]
param(
    [switch]$Force
)

Set-StrictMode -Version Latest

# ---------------------------
# LIST_OF_NAMES (id <tab> NAME)
# ---------------------------
$LIST_OF_NAMES = @"
20240519 OBREGON CRUZ, MADISON JAZMINE
20240535 MEIGGS MORENO, MATEO GABRIEL
20240545 YOVERA CORTEZ, FLOR DE LUCIA
20240715 SALAZAR DAMAZO, CAMILA REBECA
20240751 MANSILLA LUDEÑA, DAVID JESUS
20240764 HUAYTA CCORAHUA, STIVEN ADRIANO
20240806 ESQUERRE ALVA, FABIAN LEONARDO
20240817 GONZALES LLONTOP, LUDWING BRAD ALBERTO
20240870 GUTIERREZ SAEZ, FRANCO
20241028 GASTELO MARCHAN, JUAN ANTONIO
20241060 BARDALES AQUINO, ANDRE MARTIN
20241100 SUMARI BELLIDO, CARMEN MILAGROS
20241210 TINEO QUISPE, RICHARD FRANCO
20241308 RUDAS PAITAN, JOSUE JAIR
20241365 EVARISTO MONTOYA, JOAO ALESSANDRO
20241468 COCA REYES, JUAN LEONARDO
20241578 MORI RODRIGUEZ, FERNANDO SEBASTIAN
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

function TitleCaseWords {
    param([string]$s)
    if ($null -eq $s) { return $s }
    $s = $s.Trim()
    if ($s -eq '') { return '' }
    $words = $s -split '\s+'
    $out = @()
    foreach ($w in $words) {
        $clean = $w -replace '[^A-Za-z0-9]', ''   # drop stray punctuation
        if ($clean -eq '') { continue }
        $lower = $clean.ToLowerInvariant()
        if ($lower.Length -eq 1) {
            $out += $lower.ToUpperInvariant()
        } else {
            $out += ($lower.Substring(0,1).ToUpperInvariant() + $lower.Substring(1))
        }
    }
    return ($out -join ' ')
}

# ---------------------------
# Build id -> canonical name mapping
# ---------------------------
$map = @{}
foreach ($line in $LIST_OF_NAMES -split "`n") {
    $L = $line.Trim()
    if ([string]::IsNullOrWhiteSpace($L)) { continue }
    if ($L -match '^\s*(\d+)\s+(.+)$') {
        $id = $matches[1]
        $name = $matches[2].Trim()
        $map[$id] = $name
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

foreach ($d in $dirs) {
    $folderName = $d.Name
    # Expect format: ID_LastCamel_FirstCamel  e.g. 20244387_LeonCardenas_Richard
    $parts = $folderName -split '_'
    if ($parts.Count -lt 1) {
        Write-Warning "Cannot parse folder name: '$folderName' - skipping."
        continue
    }
    $id = $parts[0]

    if (-not $map.ContainsKey($id)) {
        Write-Warning "No mapping for ID '$id' (folder '$folderName') - skipping."
        continue
    }

    $canonicalName = $map[$id]   # e.g. "LEON CARDENAS, RICHARD"
    # Remove diacritics and build Title Case for last and first
    $nd = Remove-Diacritics($canonicalName)

    $lastPart = $null; $firstPart = $null
    if ($nd -match '^(.*?),(.*)$') {
        $lastPart = $matches[1].Trim()
        $firstPart = $matches[2].Trim()
    } else {
        # fallback: treat entire string as last name if no comma
        $lastPart = $nd.Trim()
        $firstPart = ''
    }

    $lastTitle = TitleCaseWords($lastPart)
    $firstTitle = TitleCaseWords($firstPart)

    if ($firstTitle -ne '') {
        $studentLine = "$lastTitle, $firstTitle $id"
    } else {
        $studentLine = "$lastTitle $id"
    }

    $rubricFileName = "${folderName}_rubric.txt"
    $rubricFilePath = Join-Path -Path $d.FullName -ChildPath $rubricFileName

    if ((Test-Path -LiteralPath $rubricFilePath) -and (-not $Force)) {
        Write-Host "Skipping existing rubric file: '$rubricFilePath' (use -Force to overwrite)"
        continue
    }

    # Build rubric body exactly as requested (blank lines between items)
    $body = @"
Alumno:
-
Puntaje
01. [1.50]
02. [0.50]
03. [0.50]
04. [1.00]
05. [0.25]
06. [2.00]
07. [0.50]
08. [1.00]
09. [0.50]
10. [0.50]
11. [1.00]
12. [0.25]
13. [1.50]
14. [2.50]
15. [0.50]
16. [0.50]
17. [0.25]
18. [0.50]
19. [0.50]
20. [0.50]
21. [0.75]
22. [0.75]
23. [0.75]
24. [1.50]
-
Descuentos
a. [-3.00] 0.00
b. [-2.00] 0.00
c. [-0.50] 0.00
d. [-1.00] 0.00
e. [-0.25] 0.00
f. [-1.00] 0.00
-
Observaciones

"@

    # Write file (UTF8). -Force to overwrite if requested.
    try {
        $body | Out-File -FilePath $rubricFilePath -Encoding UTF8 -Force:$Force
        Write-Host "Created rubric: $rubricFilePath"
    } catch {
        Write-Error "Failed to create rubric for '$folderName': $_"
    }
}

Write-Host "Done."
