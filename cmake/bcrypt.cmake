include(FetchContent)

FetchContent_Declare(
    bcrypt
    GIT_REPOSITORY https://github.com/hilch/Bcrypt.cpp.git
)
FetchContent_MakeAvailable(bcrypt)

target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)
