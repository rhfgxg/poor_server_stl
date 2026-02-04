# Install Git pre-commit hook
# Usage: .\tools\windows\install_git_hooks.ps1

$HookSource = "tools/windows/pre-commit-hook.sh"
$HookDest = ".git/hooks/pre-commit"

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  Install Git Pre-Commit Hook" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check if in Git repository
if (-not (Test-Path ".git")) {
    Write-Host "ERROR: Not a Git repository" -ForegroundColor Red
    exit 1
}

# Check if source file exists
if (-not (Test-Path $HookSource)) {
    Write-Host "ERROR: Hook source file not found: $HookSource" -ForegroundColor Red
    exit 1
}

# Create hooks directory if not exists
$HooksDir = ".git/hooks"
if (-not (Test-Path $HooksDir)) {
    New-Item -ItemType Directory -Path $HooksDir | Out-Null
}

# Backup existing hook
if (Test-Path $HookDest) {
    $BackupPath = "$HookDest.backup." + (Get-Date -Format "yyyyMMdd_HHmmss")
    Write-Host "Backing up existing hook to: $BackupPath" -ForegroundColor Yellow
    Copy-Item $HookDest $BackupPath
}

# Copy hook file
Copy-Item $HookSource $HookDest -Force

# Fix line endings to LF (required for Git Bash)
$content = [System.IO.File]::ReadAllText($HookDest)
$content = $content -replace "`r`n", "`n" -replace "`r", "`n"
$utf8NoBom = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllText($HookDest, $content, $utf8NoBom)

Write-Host ""
Write-Host "SUCCESS: Git pre-commit hook installed!" -ForegroundColor Green
Write-Host ""
Write-Host "The hook will check on each commit:" -ForegroundColor White
Write-Host "  - File encoding (must be UTF-8 without BOM)" -ForegroundColor Gray
Write-Host "  - Line endings (must be LF)" -ForegroundColor Gray
Write-Host ""
Write-Host "If checks fail, commit will be blocked." -ForegroundColor Yellow
Write-Host "Fix tool: .\tools\debug\windows\fix_encoding_and_lineendings.ps1" -ForegroundColor Cyan
