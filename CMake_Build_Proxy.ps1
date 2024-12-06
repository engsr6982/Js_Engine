$Env:http_proxy="http://127.0.0.1:7890";$Env:https_proxy="http://127.0.0.1:7890"

# 显示编译目标选项
Write-Host "Please select the build target:"
Write-Host "1: Debug"
Write-Host "2: Release"
Write-Host "3: RelWithDebInfo"
Write-Host "4: MinSizeRel"

# 获取用户输入
$choice = Read-Host "Please input the option (1-4): "

# 根据选择设置编译类型
switch ($choice) {
    1 { $buildType = "Debug" }
    2 { $buildType = "Release" }
    3 { $buildType = "RelWithDebInfo" }
    4 { $buildType = "MinSizeRel" }
    default { 
        Write-Host "Invalid option, using default value RelWithDebInfo"
        $buildType = "RelWithDebInfo" 
    }
}

Write-Host "Build target: $buildType"

cmake -B build -S . -DCMAKE_BUILD_TYPE=$buildType

cmake --build build --config $buildType

pause