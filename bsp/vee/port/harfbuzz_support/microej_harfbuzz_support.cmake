include_guard()
message("microej/harfbuzz_support component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/hb-alloc.c
)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/libharfbuzz.a)
