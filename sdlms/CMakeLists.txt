

include_directories("${CMAKE_SOURCE_DIR}/3rdparty/entt/src")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/SDL/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp/mio/include")

file(GLOB_RECURSE
        systems_cpp
        CONFIGURE_DEPENDS
        Systems/*.cpp
)

file(GLOB_RECURSE
        systems_cppm
        CONFIGURE_DEPENDS
        Systems/*.cppm
)

file(GLOB_RECURSE
        entities_cpp
        CONFIGURE_DEPENDS
        Entities/*.cpp
)

file(GLOB_RECURSE
        entities_cppm
        CONFIGURE_DEPENDS
        Entities/*.cppm
)

file(GLOB_RECURSE
        resources_cpp
        CONFIGURE_DEPENDS
        Resources/*.cpp
)

file(GLOB_RECURSE
        resources_cppm
        CONFIGURE_DEPENDS
        Resources/*.cppm
)

file(GLOB_RECURSE
        components_cpp
        CONFIGURE_DEPENDS
        Components/*.cpp
)

file(GLOB_RECURSE
        components_cppm
        CONFIGURE_DEPENDS
        Components/*.cppm
)

file(GLOB_RECURSE
        core_cpp
        CONFIGURE_DEPENDS
        Core/*.cpp
)

file(GLOB_RECURSE
        core_cppm
        CONFIGURE_DEPENDS
        Core/*.cppm
)

file(GLOB_RECURSE
        commons_cpp
        CONFIGURE_DEPENDS
        Commons/*.cpp
)

file(GLOB_RECURSE
        commons_cppm
        CONFIGURE_DEPENDS
        Commons/*.cppm
)

file(GLOB_RECURSE
        ui_cpp
        CONFIGURE_DEPENDS
        UI/*.cpp
)

file(GLOB_RECURSE
        ui_cppm
        CONFIGURE_DEPENDS
        UI/*.cppm
)

add_library(mod
        ${systems_cpp}
        ${entities_cpp}
        ${resources_cpp}
        ${components_cpp}
        ${core_cpp}
        ${commons_cpp}
        ${ui_cpp}
)

target_sources(mod
        PUBLIC
        FILE_SET core_cxx_modules TYPE CXX_MODULES FILES
        ${systems_cppm}
        ${entities_cppm}
        ${resources_cppm}
        ${components_cppm}
        ${core_cppm}
        ${commons_cppm}
        ${ui_cppm}
       
)

add_executable(sdlMS main.cpp)

target_link_libraries(sdlMS PRIVATE SDL3::SDL3)
target_link_libraries(sdlMS PRIVATE wzlib)
target_link_libraries(sdlMS PRIVATE freetype)
target_link_libraries(sdlMS PRIVATE EnTT)
target_link_libraries(sdlMS PRIVATE mod)


set_target_properties(sdlMS PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

if(WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_definitions(-DNOMINMAX)
    endif()
endif()