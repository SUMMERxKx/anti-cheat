add_library(aimbot STATIC aimbot.cpp)
target_link_libraries(aimbot PUBLIC aimmath PRIVATE ac_warnings)
target_compile_features(aimbot PRIVATE cxx_std_20)
