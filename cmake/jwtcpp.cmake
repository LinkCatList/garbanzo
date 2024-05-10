include(FetchContent)
fetchcontent_declare(jwt-cpp 
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG 08bcf77a687fb06e34138e9e9fa12a4ecbe12332 # v0.7.0 release
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "disable building examples" FORCE)
fetchcontent_makeavailable(jwt-cpp)
 
target_link_libraries(${PROJECT_NAME} PRIVATE jwt-cpp::jwt-cpp)