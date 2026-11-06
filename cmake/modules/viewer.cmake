option(MOLSIM_ENABLE_VIEWER "Build viewer" OFF)

if(MOLSIM_ENABLE_VIEWER)
    include(FetchContent)

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GRAPHICS "GRAPHICS_API_OPENGL_43" CACHE STRING "" FORCE)

    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.5
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
    target_compile_options(raylib PRIVATE -w)

    add_executable(MolSimViewer
            "${PROJECT_SOURCE_DIR}/viewer/main.cpp"
            "${PROJECT_SOURCE_DIR}/viewer/ParticleRenderer.cpp"
            "${PROJECT_SOURCE_DIR}/src/trajectory/MSBFReader.cpp"
    )
    add_custom_target(MolSimViewerAssets ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/viewer/assets $<TARGET_FILE_DIR:MolSimViewer>/assets)
    add_dependencies(MolSimViewer MolSimViewerAssets)

    target_include_directories(MolSimViewer PRIVATE "${PROJECT_SOURCE_DIR}/src")

    target_link_libraries(MolSimViewer PRIVATE raylib)
endif ()