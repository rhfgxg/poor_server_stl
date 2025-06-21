# 链接 wsl 或远程 linux 系统进行编译调试
本文只做配置的关键步骤介绍，安装时遇到的异常请自行解决
推荐使用 vs 自动安装 wsl，手动安装可能会遇到一些问题

# wsl 安装（windows下 linux 虚拟机）
这里我选用的是 ubuntu 22.04 版本，其他系统应该差不多

## 下载必须的 vs 组件
1. 打开 windows installer 工具
2. 修改 vs 2022
3. 选择必须的组件
	- ASP.NET 和 web 开发：使用 ASP.NET Core、ASP.NET、HTML/javasript和包括 Docker 支持的容器生成 web 应用程序
	- Linux、mac and embedded development：使用 C++ 开发 Linux、macOS 和嵌入式应用程序
4. 安装

## 启动 windows 功能
1. 设置中搜索 控制面板，并打开
2. 程序 - 启用或关闭 windows 功能，勾选：
	- Windows 虚拟机平台（如果没有找到，可以不用管）
	- 适用于 Linux 的 Windows 子系统
		- 一般来说，系统会默认勾选，但 vs 自动安装时可能识别不到，此时可以手动取消勾选，然后重新勾选（需要重启两次电脑）

## 安装 wsl
### 使用 vs 自动安装（推荐）
1. vs中 顶部菜单栏 - 项目 - 为我安装 wsl（也可能在别的地方）
2. 系统会自动检测环境及必须项，然后自动安装
3. 安装后 vs 会自动链接到 wsl ，可以在 vs 顶部工具栏 - 本地计算机 - 找到 wsl 选项 

### 启动wsl
安装后会出现一个 ubuntu 的软件，打开即可进入linux 终端
默认进入为普通用户
使用 su 指令转 root 用户时，提示
```
su: Authentication failure # 验证错误
```
需要初始化 root 用户密码，管理员权限打开 终端，执行指令：
```
ubuntu config --default-user root

ubuntu	# 启动 ubuntu
启动后要求输入两次 自定义密码，即可初始化 root 密码
```

#### 个人遇到的一些问题
- 自动检测：提示未开启 适用于 Linux 的 Windows 子系统、windows虚拟机平台
	- 解决：参考前面 “启动 Windows 功能部分”，我没有找到 windows 虚拟机平台的选项，但是依然安装成功了
- 自动检测：提示未安装必须的 工具包：cmake等
	- 解决：参考前面 “下载必须的 vs 组件” 部分，安装

### 手动安装 wsl
使用管理员权限打开 powershell
```
wsl --install # 使用这个指令会列出可用的linux系统
wsl --install -d ubuntu-22.04    # 这个指令会安装lts（稳定版）ubuntu22.04

# 安装完成后会自动打开linux终端
# 安装后，第一次打开，会提示需要输入用户名和密码
```

### 检测到 localhost 代理配置，但未镜像到 WSL。NAT 模式下的 WSL 不支持 localhost 代理
wsl 默认不会使用系统的 网络代理，所以需要手动配置
启动时，就会提示上面的信息，但是可以正常使用
- 解决方法：
	1. C盘/user/[user_name]/ 文件夹下，寻找 .wslconfig.txt 文件（如果没有就新建一个）
	2. 将文件修改为以下内容
	```
	[experimental]
	autoMemoryReclaim=gradual  # gradual  | dropcache | disabled
	networkingMode=mirrored
	dnsTunneling=true
	firewall=true
	autoProxy=true
	```
	3. 管理员权限打开 powershell，执行以下指令
	```
	wsl --shutdown # 关闭 wsl
	```
	4. 重新打开 wsl
	5. 如果依然提示，可以打开 wsl settings 工具，修改网络模式为 mirrored，然后重启 wsl

## 配置 vs 连接 wsl
修改 vs 的编译配置文件 CMakeSettings.json
CMakeSettings.json 文件内容：
```
{
  "configurations": [
    {
      "name": "x64-Debug",
	  ...... # 省略其他配置，重点参考 linux 相关配置
    },
    {
      "name": "WSL-GCC-Debug",
      "generator": "Ninja",
      "configurationType": "Debug",
      "buildRoot": "${projectDir}\\out\\build\\${name}",
      "installRoot": "${projectDir}\\out\\install\\${name}",
      "cmakeExecutable": "cmake",
      "cmakeCommandArgs": "-DCMAKE_TOOLCHAIN_FILE=/home/你的用户名/vcpkg/scripts/buildsystems/vcpkg.cmake",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "inheritEnvironments": [ "linux_x64" ],
      "wslPath": "${defaultWSLPath}"
    }
  ]
}
```
## 项目在 wsl 中的地址
使用 vs 链接 wsl 时，会将所有系统盘挂载在 linux的 /mnt/下
/mnt/ 文件夹下有
c/
d/
wsl/
wslg/
c d文件夹，对应 windows 下的c d盘，并且可以直接访问对应盘内的文件

所以，假设我的项目在 windows 的 D:/Project/cpp/poor/poor_server_stl 文件夹下
那么项目在 wsl 中的地址为：/mnt/d/Project/cpp/poor/poor_server_stl
使用 wsl 编译的项目文件夹路径：/mnt/d/Project/cpp/poor/poor_server_stl  /out/build/WSL-GCC-Debug


# 实体 linux 系统 安装与环境配置


# 配置 wsl 或 linux 系统
## 安装必须的工具
### 安装其他工具
在 wsl 终端中执行：
```
su # 切换到 root 用户，或下面的指令前 添加 sudo 例如：sudo apt update

# 更新软件源
apt update
apt-get update

apt install clang	# 安装 clang
apt install build-essential ninja-build	# 安装 Ninja 编译工具

# 可选：
apt install tree	# 文件树查看工具
# tree /mnt/d/Project/cpp/poor/poor_server_stl	# 查看 poor_server_stl 项目文件树
```

### 安装 vcpkg
在 WSL 终端执行：
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh	# 执行编译和安装脚本
./git pull	# 更新 vcpkg

# 下面是使用vcpkg安装第三方库时所需的工具
sudo apt-get install pkg-config
sudo apt-get install autoconf automake autoconf-archive
sudo apt-get install autoconf automake autoconf-archive libtool m4 make pkg-config

# 下载所需第三方库和工具：参考项目根目录下的 vcpkg.json 文件
# 例如 vcpkg install 库名（例如：abseil）
# 同时安装多个库：vcpkg install --recurse 库名列表
```

### 安装数据库 mysql 和 redis 服务
```
# 处理依赖问题
# apt-get install -f 会自动修复依赖问题。
# dpkg --configure -a 会尝试配置所有未完成安装的包。
apt-get install -f
dpkg --configure -a

# 安装和启动 mysql 服务
apt-get install mysql-server
service mysql start

# 安装和启动 redis 服务
apt-get install redis-server
service redis-server start
```
我安装 myslq 时提示mysql服务已经启动，导致安装失败
而 wsl 中并没有安装和启动 mysql 服务
所以我决定 远程链接 windwos中的 mysql服务，来解决这个问题
```
# 在 wsl 中执行
mysql -h 127.0.0.1 -u <你的用户名> -p
即可使用 mysql 功能
```
后续会考虑其他方法解决