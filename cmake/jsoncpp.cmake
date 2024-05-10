include(FetchContent)

FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
)
FetchContent_MakeAvailable(jsoncpp)

target_link_libraries(${PROJECT_NAME} PRIVATE jsoncpp)
