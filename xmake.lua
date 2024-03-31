--global define begin
add_rules("mode.debug", "mode.release")

set_languages("c++17")
add_ldflags("-lpthread")
--global define end

-- coroutine test begin
target("test_memory")
    set_kind("binary")
    
    add_includedirs("include")
    add_includedirs("src/include")

    add_files("src/*.cc")
    add_files("src/*.S")
    add_files("test/test_memory.cc")

target("test_coroutine_pool")
    set_kind("binary")
    
    add_includedirs("include")
    add_includedirs("src/include")

    add_files("src/*.cc")
    add_files("src/*.S")
    add_files("test/test_coroutine_pool.cc")

-- coroutine test end