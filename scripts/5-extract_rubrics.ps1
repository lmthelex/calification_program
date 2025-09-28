# 5-extract_rubrics.ps1
# Run this from inside ./data
# Usage: powershell -ExecutionPolicy Bypass -File .\5-extract_rubrics.ps1

# Root path (relative)
$rootPath = ".\"

# Loop through each student folder
Get-ChildItem -Path $rootPath -Directory | ForEach-Object {
    $folder = $_.FullName
    $folderName = $_.Name

    # Expected rubric file names
    $rubricFile = Join-Path $folder "$folderName`_rubric.txt"
    $finalRubricFile = Join-Path $folder "$folderName`_final_rubric.txt"

    # Copy rubric.txt if it exists
    if (Test-Path $rubricFile) {
        $destination = Join-Path $rootPath (Split-Path $rubricFile -Leaf)
        Copy-Item -Path $rubricFile -Destination $destination -Force
    }

    # Copy final_rubric.txt if it exists
    if (Test-Path $finalRubricFile) {
        $destination = Join-Path $rootPath (Split-Path $finalRubricFile -Leaf)
        Copy-Item -Path $finalRubricFile -Destination $destination -Force
    }
}
