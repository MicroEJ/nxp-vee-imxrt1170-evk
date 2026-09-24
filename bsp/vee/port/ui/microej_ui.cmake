include_guard()
message("microej/ui component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/event_generator.c
    ${CMAKE_CURRENT_LIST_DIR}/src/framerate_impl_FreeRTOS.c
    ${CMAKE_CURRENT_LIST_DIR}/src/framerate.c
    ${CMAKE_CURRENT_LIST_DIR}/src/LLUI_DISPLAY_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/src/LLUI_INPUT_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/src/LLUI_LED_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/src/touch_helper.c
    ${CMAKE_CURRENT_LIST_DIR}/src/touch_manager.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE    ${CMAKE_CURRENT_LIST_DIR}/inc)
