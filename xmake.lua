add_rules("mode.debug", "mode.release")

add_repositories("levilamina https://github.com/LiteLDev/xmake-repo.git")

add_requires(
    "fmt >=10.0.0 <11.0.0",
    "expected-lite 0.8.0",
    "entt 3.14.0",
    "microsoft-gsl 4.0.0",
    "nlohmann_json 3.11.3",
    "boost 1.85.0",
    "glm 1.0.1",
    "concurrentqueue 1.0.4"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("Js_Engine")
    add_cxflags(
        "/EHa",
        "/utf-8",
        -- "/W4",
        "/sdl"
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_AMD64_"
    )
    add_files("src/**.cc")
    add_includedirs(
        "EndStone-SDK/include",
        "src"
    )
    add_packages(
        "fmt",
        "expected-lite",
        "entt",
        "microsoft-gsl",
        "nlohmann_json",
        "boost",
        "glm",
        "concurrentqueue"
    )
    set_kind("shared")
    set_languages("cxx20")
    add_linkdirs("EndStone-SDK/lib")
    set_symbols("debug")
    set_exceptions("none") -- 不使用异常处理

    -- entt EndStone-SDK
    add_defines("ENTT_SPARSE_PAGE=2048")
    add_defines("ENTT_PACKED_PAGE=128")

    -- NodeJs
    add_links("third-party/nodejs/lib/libnode.lib")
    add_includedirs(
        "third-party/nodejs/include",
        "third-party/nodejs/include/v8",
        "third-party/nodejs/include/uv"
    )
    -- Puerts
    add_includedirs(
        "node_modules/puerts/puerts_libs/include",
        "node_modules/puerts/src"
    )
    add_files(
        "node_modules/puerts/src/CppObjectMapper.cpp",
        "node_modules/puerts/src/JSClassRegister.cpp",
        "node_modules/puerts/src/DataTransfer.cpp"
    )
    add_links("node_modules/puerts/puerts_libs/src/*.c")

    if is_mode("debug") then
        add_defines("DEBUG")
    end

    after_build(function(target)
        local output_dir = path.join(os.projectdir(), "bin")

        -- 清理输出目录
        -- if os.isdir(output_dir) then os.rm(output_dir) end

        -- dll复制
        os.cp(target:targetfile(), path.join(output_dir, target:name() .. ".dll"))

        -- pdb复制
        local pdb_path = path.join(output_dir, target:name() .. ".pdb")
        if os.isfile(target:symbolfile()) then 
            os.cp(target:symbolfile(), pdb_path) 
        end

        -- 输出信息
        cprint("${bright green}[plugin Packer]: ${reset}plugin already generated to " .. output_dir)
    end)