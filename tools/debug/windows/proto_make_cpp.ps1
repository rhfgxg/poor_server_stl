# Generate C++ Protobuf and gRPC code
# 根据 proto 文件，生成 C++ 对应源码

# 设置路径常量
$ScriptDir = $PSScriptRoot
$ProjectDir = Split-Path (Split-Path $ScriptDir -Parent) -Parent
$ProtoDir = Join-Path $ProjectDir "protobuf\cpp"
$MakeOut = Join-Path $ProjectDir "protobuf\cpp\src"
$Protoc = Join-Path $ProjectDir "vcpkg_installed\x64-windows\tools\protobuf\protoc.exe"
$GrpcPlugin = Join-Path $ProjectDir "vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe"

# 检查 protoc 是否存在
if (-not (Test-Path $Protoc)) {
    Write-Host "Error: protoc not found at $Protoc" -ForegroundColor Red
    Write-Host "Please install protobuf using vcpkg" -ForegroundColor Yellow
    pause
    exit 1
}

# 检查 gRPC plugin 是否存在
if (-not (Test-Path $GrpcPlugin)) {
    Write-Host "Error: grpc_cpp_plugin not found at $GrpcPlugin" -ForegroundColor Red
    Write-Host "Please install grpc using vcpkg" -ForegroundColor Yellow
    pause
    exit 1
}

# 创建输出目录
if (-not (Test-Path $MakeOut)) {
    New-Item -ItemType Directory -Path $MakeOut | Out-Null
    Write-Host "Created directory: $MakeOut" -ForegroundColor Green
}

Write-Host "`nGenerating C++ protobuf and gRPC code..." -ForegroundColor Cyan
Write-Host "Proto directory: $ProtoDir" -ForegroundColor Gray
Write-Host "Output directory: $MakeOut" -ForegroundColor Gray
Write-Host ""

# 辅助函数：生成 protobuf 和 gRPC 代码
function Generate-ProtoModule {
    param (
        [string]$ModuleName,
        [string]$Description,
        [bool]$IsCommented = $false
    )
    
    $ProtoFile = Join-Path $ProtoDir "$ModuleName.proto"
    
    if ($IsCommented) {
        Write-Host "Skipped: $ModuleName ($Description) - commented out" -ForegroundColor Gray
        return
    }
    
    if (-not (Test-Path $ProtoFile)) {
        Write-Host "Skipped: $ModuleName - file not found" -ForegroundColor Yellow
        return
    }
    
    Write-Host "Processing: $ModuleName - $Description" -ForegroundColor Cyan
    
    # 生成 protobuf 代码
    Write-Host "  Generating protobuf code..." -NoNewline
    $process = Start-Process -FilePath $Protoc `
                             -ArgumentList "--proto_path=$ProtoDir", "--cpp_out=$MakeOut", $ProtoFile `
                             -NoNewWindow -Wait -PassThru
    if ($process.ExitCode -eq 0) {
        Write-Host " OK" -ForegroundColor Green
    } else {
        Write-Host " FAILED" -ForegroundColor Red
        return
    }
    
    # 生成 gRPC 代码
    Write-Host "  Generating gRPC code..." -NoNewline
    $process = Start-Process -FilePath $Protoc `
                             -ArgumentList "--proto_path=$ProtoDir", "--grpc_out=$MakeOut", `
                                          "--plugin=protoc-gen-grpc=$GrpcPlugin", $ProtoFile `
                             -NoNewWindow -Wait -PassThru
    if ($process.ExitCode -eq 0) {
        Write-Host " OK" -ForegroundColor Green
    } else {
        Write-Host " FAILED" -ForegroundColor Red
    }
}

# 生成各模块代码
Generate-ProtoModule "common" "包含共享数据类型，枚举的定义"
Generate-ProtoModule "server_battle" "战斗服务器"
Generate-ProtoModule "server_chat" "聊天服务器" -IsCommented $true
Generate-ProtoModule "server_central" "中心服务器"
Generate-ProtoModule "server_db" "数据库服务器"
Generate-ProtoModule "server_file" "文件服务器"
Generate-ProtoModule "server_gateway" "网关服务器"
Generate-ProtoModule "server_logic" "逻辑服务器"
Generate-ProtoModule "server_login" "登录服务器"
Generate-ProtoModule "server_matching" "匹配服务器"
Generate-ProtoModule "server_store" "商城服务器" -IsCommented $true

Write-Host "`n脚本执行完毕" -ForegroundColor Green
Write-Host "生成的文件位于: $MakeOut" -ForegroundColor Gray

# 显示生成的文件
Write-Host "`n生成的文件:" -ForegroundColor Cyan
Get-ChildItem -Path $MakeOut -Filter "*.pb.*" | ForEach-Object {
    $Size = [math]::Round($_.Length / 1KB, 2)
    Write-Host "  $($_.Name) ($Size KB)" -ForegroundColor Gray
}

pause
