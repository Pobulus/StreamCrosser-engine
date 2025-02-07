cmake_minimum_required(VERSION 3.16)


include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0  # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)

project(streamcrosserengine)

add_executable(streamcrosserengine main.cpp)


target_link_libraries(streamcrosserengine PUBLIC yaml-cpp::yaml-cpp) # The library or executable that require yaml-cpp library


# target_link_libraries(streamcrosserengine yaml-cpp)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Parser/parser parser
                   COMMAND echo "Compiling parser" && make clean && make
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Parser
)


add_custom_target(parser COMMAND make
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Parser
)

# Copy parser to build directory
set(copy_source_dir "${CMAKE_SOURCE_DIR}/Parser")
set(copy_dest_dir "${CMAKE_BINARY_DIR}")
set(copy_file_name "parser")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${copy_dest_dir}
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${copy_source_dir}/${copy_file_name}" "${copy_dest_dir}/${copy_file_name}"
    COMMENT "Copying ${copy_file_name} to build directory"
)
add_dependencies(streamcrosserengine parser)

install(TARGETS streamcrosserengine RUNTIME DESTINATION bin)
