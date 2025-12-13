# Generate Protobuf Descriptor Files
# 生成 Protobuf Descriptor 文件供 Skynet 使用

# 使用方法：
#   .\tools\generate_proto_desc.ps1

# 设置项目根路径
$ProjectRoot = Split-Path $PSScriptRoot -Parent

# 检查 protoc 是否安装
$protoc = Get-Command protoc -ErrorAction SilentlyContinue

# 如果系统中没有 protoc，尝试使用 vcpkg 安装的版本
if (-not $protoc) {
    $vcpkgProtoc = Join-Path $ProjectRoot "vcpkg_installed\x64-windows\tools\protobuf\protoc.exe"
    if (Test-Path $vcpkgProtoc) {
        $protoc = @{ Source = $vcpkgProtoc }
        Write-Host "Using vcpkg protoc: $vcpkgProtoc" -ForegroundColor Green
    } else {
        Write-Host "Error: protoc not found!" -ForegroundColor Red
        Write-Host "Please install Protocol Buffers compiler:" -ForegroundColor Yellow
        Write-Host "  - Download from: https://github.com/protocolbuffers/protobuf/releases" -ForegroundColor Yellow
        Write-Host "  - Or use vcpkg: vcpkg install protobuf" -ForegroundColor Yellow
        Write-Host "  - Checked: $vcpkgProtoc" -ForegroundColor Gray
        exit 1
    }
} else {
    Write-Host "Found protoc: $($protoc.Source)" -ForegroundColor Green
}

$protocPath = $protoc.Source

# 设置路径
$ProtoDir = Join-Path $ProjectRoot "protobuf\skynet"
$DescDir = Join-Path $ProtoDir "src"

# 创建输出目录
if (-not (Test-Path $DescDir)) {
    New-Item -ItemType Directory -Path $DescDir | Out-Null
    Write-Host "Created directory: $DescDir" -ForegroundColor Green
}

# 获取所有 proto 文件
$ProtoFiles = Get-ChildItem -Path $ProtoDir -Filter "*.proto" -File

if ($ProtoFiles.Count -eq 0) {
    Write-Host "No .proto files found in $ProtoDir" -ForegroundColor Yellow
    exit 0
}

Write-Host "`nGenerating descriptor files..." -ForegroundColor Cyan
Write-Host "Proto directory: $ProtoDir" -ForegroundColor Gray
Write-Host "Output directory: $DescDir" -ForegroundColor Gray
Write-Host ""

# 生成每个 proto 文件的 descriptor
$SuccessCount = 0
$FailCount = 0

foreach ($ProtoFile in $ProtoFiles) {
    $ProtoName = $ProtoFile.BaseName
    $ProtoPath = $ProtoFile.FullName
    $DescPath = Join-Path $DescDir "$ProtoName.pb"
    
    Write-Host "Processing: $($ProtoFile.Name)..." -NoNewline
    
    # 执行 protoc 命令
    # 使用 --include_imports 包含所有依赖的 proto 文件
    $Arguments = @(
        "--descriptor_set_out=$DescPath",
        "--include_imports",
        "--proto_path=$ProtoDir",
        $ProtoPath
    )
    
    try {
        $process = Start-Process -FilePath $protocPath `
                                 -ArgumentList $Arguments `
                                 -NoNewWindow `
                                 -Wait `
                                 -PassThru `
                                 -RedirectStandardError (Join-Path $env:TEMP "protoc_error.txt")
        
        if ($process.ExitCode -eq 0) {
            Write-Host " OK" -ForegroundColor Green
            $SuccessCount++
        } else {
            $ErrorMsg = Get-Content (Join-Path $env:TEMP "protoc_error.txt") -Raw
            Write-Host " FAILED" -ForegroundColor Red
            Write-Host "  Error: $ErrorMsg" -ForegroundColor Red
            $FailCount++
        }
    } catch {
        Write-Host " FAILED" -ForegroundColor Red
        Write-Host "  Error: $_" -ForegroundColor Red
        $FailCount++
    }
}

Write-Host ""
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Success: $SuccessCount" -ForegroundColor Green
Write-Host "  Failed:  $FailCount" -ForegroundColor $(if ($FailCount -gt 0) { "Red" } else { "Gray" })
Write-Host "  Output:  $DescDir" -ForegroundColor Gray

if ($SuccessCount -gt 0) {
    Write-Host "`nGenerated descriptor files:" -ForegroundColor Cyan
    Get-ChildItem -Path $DescDir -Filter "*.pb" | ForEach-Object {
        $Size = [math]::Round($_.Length / 1KB, 2)
        Write-Host "  $($_.Name) ($Size KB)" -ForegroundColor Gray
    }
}

Write-Host "`nDone!" -ForegroundColor Green
