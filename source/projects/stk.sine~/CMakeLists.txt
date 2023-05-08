include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)

#############################################################
# MAX EXTERNAL
#############################################################

set(STK ${CMAKE_BINARY_DIR}/deps/stk-install)
set(STK_INCLUDE ${STK}/include/stk)
set(STK_LIB ${STK}/lib/libstk.a)

MESSAGE("STK_INCLUDE: ${STK_INCLUDE}")
MESSAGE("STK_LIB: ${STK_LIB}")

include_directories( 
  "${MAX_SDK_INCLUDES}"
  "${MAX_SDK_MSP_INCLUDES}"
  "${MAX_SDK_JIT_INCLUDES}"
)

file(GLOB PROJECT_SRC
   "*.h"
   "*.c"
   "*.cpp"
)

add_library( 
    ${PROJECT_NAME} 
    MODULE
    ${PROJECT_SRC}
)


target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${STK_INCLUDE}
)


target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${STK_LIB}
)



include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
