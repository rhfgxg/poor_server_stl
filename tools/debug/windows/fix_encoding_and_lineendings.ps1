# Encoding and Line Ending Fix Tool
# Usage: .\tools\debug\windows\fix_encoding_and_lineendings.ps1 -DryRun

param(
    [string]$RootPath = ".",
    [switch]$DryRun = $false
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  Encoding & Line Ending Fixer" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

if ($DryRun) {
    Write-Host "[DRY RUN MODE]" -ForegroundColor Yellow
    Write-Host ""
}

$FileExtensions = @("*.cpp", "*.h", "*.hpp", "*.lua", "*.cmake", "*.proto", "*.json", "*.yaml", "*.yml", "*.md", "*.txt")
$ExcludeDirs = @("build", "out", "vcpkg_installed", ".vs", ".vscode", ".git", "node_modules")

$TotalFiles = 0
$ModifiedFiles = 0
$ErrorFiles = 0
$AllFiles = @()

foreach ($ext in $FileExtensions) {
    $Files = Get-ChildItem -Path $RootPath -Filter $ext -Recurse -File | Where-Object { 
        $path = $_.FullName
        $shouldExclude = $false
        foreach ($dir in $ExcludeDirs) {
            if ($path -like "*\$dir\*") { $shouldExclude = $true; break }
        }
        -not $shouldExclude
    }
    $AllFiles += $Files
}

Write-Host "Found $($AllFiles.Count) files to check" -ForegroundColor Green
Write-Host ""

foreach ($File in $AllFiles) {
    $TotalFiles++
    $RelativePath = $File.FullName.Replace((Get-Location).Path + "\", "")
    
    try {
        $Bytes = [System.IO.File]::ReadAllBytes($File.FullName)
        if ($Bytes.Length -eq 0) { continue }
        
        $CurrentEncoding = "Unknown"
        $HasBOM = $false
        $Content = $null
        
        # Detect encoding
        if ($Bytes.Length -ge 3 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF) {
            $CurrentEncoding = "UTF-8 BOM"
            $HasBOM = $true
            $Content = [System.Text.Encoding]::UTF8.GetString($Bytes, 3, $Bytes.Length - 3)
        } elseif ($Bytes.Length -ge 2 -and $Bytes[0] -eq 0xFF -and $Bytes[1] -eq 0xFE) {
            $CurrentEncoding = "UTF-16 LE"
            $Content = [System.Text.Encoding]::Unicode.GetString($Bytes)
        } elseif ($Bytes.Length -ge 2 -and $Bytes[0] -eq 0xFE -and $Bytes[1] -eq 0xFF) {
            $CurrentEncoding = "UTF-16 BE"
            $Content = [System.Text.Encoding]::BigEndianUnicode.GetString($Bytes)
        } else {
            $Utf8Content = [System.Text.Encoding]::UTF8.GetString($Bytes)
            if ($Utf8Content.Contains([char]0xFFFD)) {
                $CurrentEncoding = "GB2312"
                $Content = [System.Text.Encoding]::Default.GetString($Bytes)
            } else {
                $CurrentEncoding = "UTF-8"
                $Content = $Utf8Content
            }
        }
        
        if ($null -eq $Content -or $Content.Length -eq 0) { continue }
        
        # Detect line endings
        $HasCRLF = $Content.Contains("`r`n")
        $LineEndingType = if ($HasCRLF) { "CRLF" } else { "LF" }
        
        # Check if needs fix
        $NeedsChange = $false
        $ChangeReason = @()
        
        if ($HasBOM) { $NeedsChange = $true; $ChangeReason += "BOM" }
        if ($CurrentEncoding -ne "UTF-8") { $NeedsChange = $true; $ChangeReason += "ENC:$CurrentEncoding" }
        if ($LineEndingType -ne "LF") { $NeedsChange = $true; $ChangeReason += "CRLF" }
        
        if ($NeedsChange) {
            $ModifiedFiles++
            $ReasonStr = $ChangeReason -join ","
            Write-Host "[$TotalFiles] FIX ($ReasonStr): $RelativePath" -ForegroundColor Yellow
            
            if (-not $DryRun) {
                $Content = $Content -replace "`r`n", "`n" -replace "`r", "`n"
                $Utf8NoBom = New-Object System.Text.UTF8Encoding $false
                [System.IO.File]::WriteAllText($File.FullName, $Content, $Utf8NoBom)
                Write-Host "    Fixed!" -ForegroundColor Green
            }
        } else {
            Write-Host "[$TotalFiles] OK: $RelativePath" -ForegroundColor Green
        }
    } catch {
        $ErrorFiles++
        Write-Host "[$TotalFiles] ERROR: $RelativePath" -ForegroundColor Red
        Write-Host "    $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  Complete" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Total: $TotalFiles | Fixed: $ModifiedFiles | Errors: $ErrorFiles" -ForegroundColor White

if ($DryRun -and $ModifiedFiles -gt 0) {
    Write-Host ""
    Write-Host "[DRY RUN] No files were modified." -ForegroundColor Yellow
    Write-Host "Run without -DryRun to apply fixes." -ForegroundColor Yellow
}