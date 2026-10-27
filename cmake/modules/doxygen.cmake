option(ENABLE_DOXYGEN "Enable doxygen documentation" ON)

if(ENABLE_DOXYGEN)
    find_package(Doxygen QUIET)
    
    if(DOXYGEN_FOUND)
        message(STATUS "Doxygen found. Generating documentation")

        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
        doxygen_add_docs(
            doc_doxygen
            "${PROJECT_SOURCE_DIR}/src"
            "${PROJECT_SOURCE_DIR}/README.md"
        )
    else()
        message(STATUS "Failed to find doxygen")
    endif()
endif()