include (ExternalProject)

set(GTEST_VERSION "1.8.0")
set(THRIFT_VERSION "0.12.0")
set(BOOST_VERSION "1.58")
set(DOXYGEN_VERSION "1.8")

find_package(Threads REQUIRED)
find_package(Boost ${BOOST_VERSION} REQUIRED)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

set(EXTERNAL_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC ${CMAKE_CXX_FLAGS_${UPPERCASE_BUILD_TYPE}}")
set(EXTERNAL_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ${CMAKE_C_FLAGS_${UPPERCASE_BUILD_TYPE}}")

# Google Test framework
if (BUILD_TESTS)
  if(APPLE)
    set(GTEST_CMAKE_CXX_FLAGS "${EXTERNAL_CXX_FLAGS} -DGTEST_USE_OWN_TR1_TUPLE=1 -Wno-unused-value -Wno-ignored-attributes")
  else()
    set(GTEST_CMAKE_CXX_FLAGS "${EXTERNAL_CXX_FLAGS}")
  endif()
  
  set(GTEST_PREFIX "${PROJECT_BINARY_DIR}/external/gtest")
  set(GTEST_INCLUDE_DIR "${GTEST_PREFIX}/include")
  set(GTEST_STATIC_LIB
    "${GTEST_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(GTEST_MAIN_STATIC_LIB
    "${GTEST_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}")
  
  set(GTEST_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                       "-DCMAKE_INSTALL_PREFIX=${GTEST_PREFIX}"
                       "-Dgtest_force_shared_crt=ON"
                       "-DCMAKE_CXX_FLAGS=${GTEST_CMAKE_CXX_FLAGS}")
  
  ExternalProject_Add(googletest
    URL "https://github.com/google/googletest/archive/release-${GTEST_VERSION}.tar.gz"
    CMAKE_ARGS ${GTEST_CMAKE_ARGS})
  
  message(STATUS "GTest include dir: ${GTEST_INCLUDE_DIR}")
  message(STATUS "GTest static library: ${GTEST_STATIC_LIB}")
  message(STATUS "GTest main static library: ${GTEST_MAIN_STATIC_LIB}")
  include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
  
  add_library(gtest STATIC IMPORTED GLOBAL)
  set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${GTEST_STATIC_LIB})
  
  add_library(gtest_main STATIC IMPORTED GLOBAL)
  set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION
    ${GTEST_MAIN_STATIC_LIB})
endif()

ExternalProject_Add(lz4
        URL https://github.com/lz4/lz4/archive/v1.8.2.tar.gz
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE 1
        BUILD_COMMAND make -C lib lib MOREFLAGS=-fPIC
        INSTALL_COMMAND ""
)
ExternalProject_Get_Property(lz4 SOURCE_DIR BINARY_DIR)
set(lz4_INCLUDE_DIR "${SOURCE_DIR}/lib")
set(lz4_STATIC_LIB "${BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX}")
include_directories(SYSTEM ${lz4_INCLUDE_DIR})

set(JEMALLOC_CXX_FLAGS "${EXTERNAL_CXX_FLAGS}")
set(JEMALLOC_C_FLAGS "${EXTERNAL_C_FLAGS}")
set(JEMALLOC_LD_FLAGS "-Wl,--no-as-needed")
set(JEMALLOC_PREFIX "${PROJECT_BINARY_DIR}/external/jemalloc")
set(JEMALLOC_HOME "${JEMALLOC_PREFIX}")
set(JEMALLOC_INCLUDE_DIR "${JEMALLOC_PREFIX}/include")
set(JEMALLOC_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CXX_FLAGS=${JEMALLOC_CXX_FLAGS}"
        "-DCMAKE_INSTALL_PREFIX=${JEMALLOC_PREFIX}")
set(JEMALLOC_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}jemalloc")
set(JEMALLOC_LIBRARIES "${JEMALLOC_PREFIX}/lib/${JEMALLOC_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
ExternalProject_Add(jemalloc
        URL https://github.com/jemalloc/jemalloc/releases/download/5.0.1/jemalloc-5.0.1.tar.bz2
        PREFIX ${JEMALLOC_PREFIX}
        BUILD_BYPRODUCTS ${JEMALLOC_LIBRARIES}
        CONFIGURE_COMMAND ${JEMALLOC_PREFIX}/src/jemalloc/configure --prefix=${JEMALLOC_PREFIX} --enable-autogen --enable-prof-libunwind CFLAGS=${JEMALLOC_C_FLAGS} CXXFLAGS=${JEMALLOC_CXX_FLAGS}
        INSTALL_COMMAND make install_lib
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)
message(STATUS "Jemalloc library: ${JEMALLOC_LIBRARIES}")
install(FILES ${JEMALLOC_LIBRARIES} DESTINATION lib)

if (BUILD_RPC)
  set(THRIFT_CXX_FLAGS "${EXTERNAL_CXX_FLAGS}")
  set(THRIFT_C_FLAGS "${EXTERNAL_C_FLAGS}")
  set(THRIFT_PREFIX "${PROJECT_BINARY_DIR}/external/thrift")
  set(THRIFT_HOME "${THRIFT_PREFIX}")
  set(THRIFT_INCLUDE_DIR "${THRIFT_PREFIX}/include")
  set(THRIFT_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                        "-DCMAKE_CXX_FLAGS=${THRIFT_CXX_FLAGS}"
                        "-DCMAKE_C_FLAGS=${THRIFT_C_FLAGS}"
                        "-DCMAKE_INSTALL_PREFIX=${THRIFT_PREFIX}"
                        "-DCMAKE_INSTALL_RPATH=${THRIFT_PREFIX}/lib"
                        "-DBUILD_COMPILER=OFF"
                        "-DBUILD_TESTING=OFF"
                        "-DWITH_SHARED_LIB=OFF"
                        "-DWITH_QT4=OFF"
                        "-DWITH_QT5=OFF"
                        "-DWITH_C_GLIB=OFF"
                        "-DWITH_HASKELL=OFF"
                        "-DWITH_ZLIB=OFF" # For now
                        "-DWITH_OPENSSL=OFF" # For now
                        "-DWITH_LIBEVENT=OFF" # For now
                        "-DWITH_JAVA=OFF"
                        "-DWITH_PYTHON=OFF"
                        "-DWITH_CPP=ON"
                        "-DWITH_STDTHREADS=OFF"
                        "-DWITH_BOOSTTHREADS=OFF"
                        "-DWITH_STATIC_LIB=ON")


  if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(THRIFT_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}thriftd")
  else ()
    set(THRIFT_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}thrift")
  endif ()

  set(THRIFT_STATIC_LIB "${THRIFT_PREFIX}/lib/${THRIFT_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  ExternalProject_Add(thrift
      URL "http://archive.apache.org/dist/thrift/${THRIFT_VERSION}/thrift-${THRIFT_VERSION}.tar.gz"
      CMAKE_ARGS ${THRIFT_CMAKE_ARGS})

  include_directories(SYSTEM ${THRIFT_INCLUDE_DIR} ${THRIFT_INCLUDE_DIR}/thrift)
  message(STATUS "Thrift include dir: ${THRIFT_INCLUDE_DIR}")
  message(STATUS "Thrift static library: ${THRIFT_STATIC_LIB}")
  add_library(thriftstatic STATIC IMPORTED GLOBAL)
  set_target_properties(thriftstatic PROPERTIES IMPORTED_LOCATION ${THRIFT_STATIC_LIB})
  
  install(FILES ${THRIFT_STATIC_LIB} DESTINATION lib)
  install(DIRECTORY ${THRIFT_INCLUDE_DIR}/thrift DESTINATION include)
endif()

if (WITH_PY_CLIENT)
  include(FindPythonInterp)
  if (NOT PYTHONINTERP_FOUND)
    message(FATAL_ERROR "Cannot build python client without python interpretor")
  endif()
  find_python_module(setuptools REQUIRED)
  if (NOT PY_SETUPTOOLS)
    message(FATAL_ERROR "Python setuptools is required for python client")
  endif()
endif()

if (WITH_JAVA_CLIENT)
  find_package(Java REQUIRED)
  find_package(Ant REQUIRED)
  set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.7" "-target" "1.7" "-nowarn")
endif()

if (BUILD_DOC)
  find_package(MkDocs REQUIRED)
  find_package(Doxygen ${DOXYGEN_VERSION} REQUIRED)
endif()
