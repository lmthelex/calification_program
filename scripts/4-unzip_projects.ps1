# 4-unzip_projects.ps1
# Run inside ./data
# Call: powershell -ExecutionPolicy Bypass -File .\4-unzip_projects.ps1

Add-Type -AssemblyName System.IO.Compression.FileSystem

Get-ChildItem -Directory | ForEach-Object {
    $folder = $_.FullName
    $zip = Get-ChildItem -Path $folder -Filter *.zip -File | Select-Object -First 1

    if ($zip) {
        Write-Host "Extracting $($zip.Name) into $folder..."

        try {
            # Use the 2-argument overload (no encoding needed)
            [System.IO.Compression.ZipFile]::ExtractToDirectory($zip.FullName, $folder)
        } catch {
            Write-Host "Already extracted or error with $($zip.Name): $($_.Exception.Message)"
        }
    } else {
        Write-Host "No ZIP found in $folder"
    }
}
