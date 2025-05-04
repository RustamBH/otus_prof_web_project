cmake_minimum_required(VERSION 3.5)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})


#--------------------------------------------------------------------------------------------------
# variables
set(PROJECT_NAME server)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(TEST_NAME UnitTest)

# Настройки для OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})


# Настройки для Boost
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

# Настройки для GTest
enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

#--------------------------------------------------------------------------------------------------
# project
project(${PROJECT_NAME} VERSION ${PROJECT_VESRION})

#--------------------------------------------------------------------------------------------------
# builds
# Main target
add_executable(
    ${PROJECT_NAME}
	${SRC_DIR}/main.cpp
    ${SRC_DIR}/server.cpp
    ${SRC_DIR}/server.h
)

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})


# Test target
add_executable(
    ${TEST_NAME} 
    ${TEST_DIR}/server_tests.cpp
)

# Add src to the include directories for the test target
target_include_directories(
    ${TEST_NAME} PRIVATE    
	${SRC_DIR}
	${GTEST_INCLUDE_DIRS}
	${GTEST_LIBRARY} 
	${GTEST_INCLUDE_DIR}
	${GTEST_MAIN_LIBRARY}	
)

# Link test target against gtest libraries
target_link_libraries(
	${TEST_NAME} PRIVATE
	GTest::gtest 
	GTest::gtest_main 
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# properties
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
set_target_properties(${TEST_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

if (MSVC)
    target_compile_options(
        server PRIVATE
        /W4
    )
    target_compile_options(
        test_myvector PRIVATE
        /W4
    )
else ()
    target_compile_options(
        server PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(
        test_server PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()


#--------------------------------------------------------------------------------------------------
# deb pkg
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")	
	
set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)