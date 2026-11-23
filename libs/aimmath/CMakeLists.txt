add_library(aimmath STATIC aimmath.cpp)
target_include_directories(aimmath PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(aimmath PRIVATE cxx_std_20)
target_link_libraries(aimmath PRIVATE ac_warnings)
