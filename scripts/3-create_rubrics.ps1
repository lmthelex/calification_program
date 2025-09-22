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
Alumno: $studentLine
-
Puntaje
01. [1.50]
02. [1.50]
03. [0.50]
04. [0.50]
05. [0.50]
06. [0.50]
07. [0.50]
08. [1.00]
09. [1.00]
10. [0.50]
11. [2.00]
12. [1.00]
13. [0.50]
14. [0.50]
15. [0.50]
16. [0.50]
17. [1.00]
18. [2.00]
19. [0.50]
20. [0.50]
21. [0.50]
22. [0.50]
23. [0.25]
24. [0.75]
25. [1.00]
-
Descuentos
a. [-3.00] 0.00
b. [-2.00] 0.00
c. [-0.50] 0.00
d. [-1.00] 0.00
e. [-0.25] 0.00
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
