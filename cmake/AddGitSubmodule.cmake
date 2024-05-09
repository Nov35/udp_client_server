function(add_git_submodule relative_dir)
    find_package(Git REQUIRED)

    set(FULL_DIR ${CMAKE_SOURCE_DIR}/${relative_dir})

    if (NOT EXISTS ${FULL_DIR}/CMakeLists.txt)
        execute_process(COMMAND ${GIT_EXECUTABLE}
            submodule update --init --recursive -- ${relative_dir}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()

endfunction(add_git_submodule)
