option(ENABLE_OPENMP "Parallelize with OpenMP if available" ON)

if (ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(Lib PUBLIC OpenMP::OpenMP_CXX)
    else ()
        message(WARNING "OpenMP not found, building serial")
        # remove unknown pragmas warning, because omp not known
        target_compile_options(Lib PUBLIC $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-unknown-pragmas>)
    endif ()
endif ()