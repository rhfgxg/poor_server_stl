Package: boost-cmake:x64-windows@1.85.0#2

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.39.33523.0
-    vcpkg-tool version: 2024-09-30-ab8988503c7cffabfd440b243a383c0a352a023d
    vcpkg-scripts version: 89f00b3b8 2024-11-11 (24 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached boostorg-boost-boost-1.85.0.tar.gz.
-- Cleaning sources at D:/opt/tools/cpp/vcpkg/buildtrees/boost-cmake/src/ost-1.85.0-27110fc97f.clean. Use --editable to skip cleaning for the packages you specify.
-- Extracting source D:/opt/tools/cpp/vcpkg/downloads/boostorg-boost-boost-1.85.0.tar.gz
-- Using source at D:/opt/tools/cpp/vcpkg/buildtrees/boost-cmake/src/ost-1.85.0-27110fc97f.clean
-- Downloading https://mirror.ghproxy.com/https://github.com/boostorg/cmake/commit/ae2e6a647187246d6009f80b56ba4c2c8f3a008c.patch?full_index=1 -> boostorg-cmake-boost-1.85.0-0009-msvc-1940-still-vc143.patch...
[DEBUG] To include the environment variables in debug output, pass --debug-env
[DEBUG] Trying to load bundleconfig from D:\opt\tools\cpp\vcpkg\vcpkg-bundle.json
[DEBUG] Failed to open: D:\opt\tools\cpp\vcpkg\vcpkg-bundle.json
[DEBUG] Bundle config: readonly=false, usegitregistry=false, embeddedsha=nullopt, deployment=Git, vsversion=nullopt
[DEBUG] Metrics enabled.
[DEBUG] Feature flag 'binarycaching' unset
[DEBUG] Feature flag 'compilertracking' unset
[DEBUG] Feature flag 'registries' unset
[DEBUG] Feature flag 'versions' unset
[DEBUG] Feature flag 'dependencygraph' unset
error: Missing boostorg-cmake-boost-1.85.0-0009-msvc-1940-still-vc143.patch and downloads are blocked by x-block-origin.
error: https://mirror.ghproxy.com/https://github.com/boostorg/cmake/commit/ae2e6a647187246d6009f80b56ba4c2c8f3a008c.patch?full_index=1: failed: status code 403
[DEBUG] D:\a\_work\1\s\src\vcpkg\base\downloads.cpp(1030): 
[DEBUG] Time in subprocesses: 0us
[DEBUG] Time in parsing JSON: 4us
[DEBUG] Time in JSON reader: 0us
[DEBUG] Time in filesystem: 961us
[DEBUG] Time in loading ports: 0us
[DEBUG] Exiting after 2.1 s (2104501us)

CMake Error at scripts/cmake/vcpkg_download_distfile.cmake:32 (message):
      
      Failed to download file with error: 1
      If you are using a proxy, please check your proxy setting. Possible causes are:
      
      1. You are actually using an HTTP proxy, but setting HTTPS_PROXY variable
         to `https://address:port`. This is not correct, because `https://` prefix
         claims the proxy is an HTTPS proxy, while your proxy (v2ray, shadowsocksr
         , etc..) is an HTTP proxy. Try setting `http://address:port` to both
         HTTP_PROXY and HTTPS_PROXY instead.
      
      2. If you are using Windows, vcpkg will automatically use your Windows IE Proxy Settings
         set by your proxy software. See https://github.com/microsoft/vcpkg-tool/pull/77
         The value set by your proxy might be wrong, or have same `https://` prefix issue.
      
      3. Your proxy's remote server is out of service.
      
      If you've tried directly download the link, and believe this is not a temporary
      download server failure, please submit an issue at https://github.com/Microsoft/vcpkg/issues
      to report this upstream download server failure.
      

Call Stack (most recent call first):
  scripts/cmake/vcpkg_download_distfile.cmake:287 (z_vcpkg_download_distfile_show_proxy_and_fail)
  C:/Users/lhw/AppData/Local/vcpkg/registries/git-trees/2b1869c19b01257dc08348e1b29bcd7d5bad2fcc/portfile.cmake:13 (vcpkg_download_distfile)
  scripts/ports.cmake:192 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "poor-server-stl",
  "version": "1.0.0",
  "dependencies": [
    {
      "name": "nlohmann-json",
      "version>=": "3.11.3#1"
    },
    {
      "name": "mysql-connector-cpp"
    },
    {
      "name": "boost"
    }
  ]
}

```
</details>
