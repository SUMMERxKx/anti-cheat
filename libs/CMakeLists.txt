add_subdirectory(aimmath)
add_subdirectory(aimbot)

if(TARGET redeclipse)
    target_sources(redeclipse PRIVATE aimbot/lab_controls.cpp)
    target_link_libraries(redeclipse PRIVATE aimbot)
    target_compile_definitions(redeclipse PRIVATE LAB_AIMBOT=1)
endif()
