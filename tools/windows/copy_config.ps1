# Copy Config Files
# 用于 debug 时，同步生成文件文件夹的配置文件
# 修改配置文件后，需要执行此脚本
# 配置文件复制到可执行文件同级的 /config 中

# 设置路径常量
$ScriptDir = $PSScriptRoot
$ProjectDir = Split-Path (Split-Path $ScriptDir -Parent) -Parent

# 配置文件目录
$ConfigDir = Join-Path $ProjectDir "config\cpp\cfg_server"

# 生成文件目录
$DebugOutDir = Join-Path $ProjectDir "out\build\x64-debug"

# 检查配置文件目录是否存在
if (-not (Test-Path $ConfigDir)) {
    Write-Host "Error: 配置文件目录不存在: $ConfigDir" -ForegroundColor Red
    pause
    exit 1
}

# 检查生成目录是否存在
if (-not (Test-Path $DebugOutDir)) {
    Write-Host "Warning: 生成目录不存在: $DebugOutDir" -ForegroundColor Yellow
    Write-Host "请先编译项目" -ForegroundColor Yellow
    pause
    exit 1
}

Write-Host "`n复制配置文件到生成目录..." -ForegroundColor Cyan
Write-Host "配置源: $ConfigDir" -ForegroundColor Gray
Write-Host "目标: $DebugOutDir" -ForegroundColor Gray
Write-Host ""

# 定义服务器配置映射
$ServerConfigs = @(
    @{Name = "battle";   ConfigFile = "cfg_battle_server.lua"},
    @{Name = "central";  ConfigFile = "cfg_central_server.lua"},
    @{Name = "db";       ConfigFile = "cfg_db_server.lua"},
    @{Name = "file";     ConfigFile = "cfg_file_server.lua"},
    @{Name = "gateway";  ConfigFile = "cfg_gateway_server.lua"},
    @{Name = "logic";    ConfigFile = "cfg_logic_server.lua"},
    @{Name = "login";    ConfigFile = "cfg_login_server.lua"},
    @{Name = "matching"; ConfigFile = "cfg_matching_server.lua"}
)

$SuccessCount = 0
$FailCount = 0

# 复制各服务器配置文件
foreach ($Server in $ServerConfigs) {
    $ServerName = $Server.Name
    $ConfigFile = $Server.ConfigFile
    
    # 目标目录
    $TargetDir = Join-Path $DebugOutDir "src\$ServerName\config"
    
    # 创建目标目录
    if (-not (Test-Path $TargetDir)) {
        New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
    }
    
    # 源文件和目标文件路径
    $SourceFile = Join-Path $ConfigDir $ConfigFile
    $TargetFile = Join-Path $TargetDir $ConfigFile
    
    # 复制文件
    Write-Host "  [$ServerName]" -NoNewline -ForegroundColor Cyan
    Write-Host " $ConfigFile..." -NoNewline
    
    if (Test-Path $SourceFile) {
        try {
            Copy-Item -Path $SourceFile -Destination $TargetFile -Force
            Write-Host " OK" -ForegroundColor Green
            $SuccessCount++
        } catch {
            Write-Host " FAILED" -ForegroundColor Red
            Write-Host "    Error: $_" -ForegroundColor Red
            $FailCount++
        }
    } else {
        Write-Host " SKIPPED (源文件不存在)" -ForegroundColor Yellow
        $FailCount++
    }
}

# 复制数据库配置文件（特殊处理）
$DbConfigFile = "cfg_db.lua"
$DbTargetDir = Join-Path $DebugOutDir "src\db\config"
$DbSourceFile = Join-Path $ConfigDir $DbConfigFile
$DbTargetFile = Join-Path $DbTargetDir $DbConfigFile

Write-Host "  [db]" -NoNewline -ForegroundColor Cyan
Write-Host " $DbConfigFile (额外)..." -NoNewline

if (Test-Path $DbSourceFile) {
    try {
        Copy-Item -Path $DbSourceFile -Destination $DbTargetFile -Force
        Write-Host " OK" -ForegroundColor Green
        $SuccessCount++
    } catch {
        Write-Host " FAILED" -ForegroundColor Red
        Write-Host "    Error: $_" -ForegroundColor Red
        $FailCount++
    }
} else {
    Write-Host " SKIPPED (源文件不存在)" -ForegroundColor Yellow
}

# 显示总结
Write-Host ""
Write-Host "总结:" -ForegroundColor Cyan
Write-Host "  成功: $SuccessCount" -ForegroundColor Green
Write-Host "  失败: $FailCount" -ForegroundColor $(if ($FailCount -gt 0) { "Red" } else { "Gray" })

if ($SuccessCount -gt 0) {
    Write-Host "`n✓ 配置文件已复制到生成目录" -ForegroundColor Green
}

Write-Host "`n脚本执行完毕" -ForegroundColor Green
pause
