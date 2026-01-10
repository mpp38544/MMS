cmake_minimum_required(VERSION 3.14) # FetchContent works best with 3.14+
project(mms VERSION 0.1.0 LANGUAGES C CXX)

# 1. Fetch Eigen
include(FetchContent)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG master
)
FetchContent_MakeAvailable(eigen)

add_executable(mms
  src/main.cpp
  src/constants.cpp
  src/dataReplayEngine.cpp
  src/kalman.cpp
  src/kalmanfiltertest.cpp
  src/matchingEngine.cpp
  src/orderBook.cpp
  src/strategyEngine.cpp
)

target_include_directories(mms PRIVATE include)

target_link_libraries(mms PRIVATE Eigen3::Eigen)

target_compile_features(mms PRIVATE cxx_std_17)
target_compile_definitions(mms PRIVATE EIGEN_NO_DEBUG) # Disables assertions for Release builds
