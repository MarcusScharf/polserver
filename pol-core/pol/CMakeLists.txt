set(ex_name pol)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    
 
set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name})
use_curl(${ex_name})
use_benchmark(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  tinyxml
  z
)
if (${linux})
  target_link_libraries(${ex_name} PUBLIC
    crypto
    pthread
  )
endif()

if(HAVE_MYSQL)
  if(${windows})
    if (${ARCH_BITS} EQUAL "64")
      target_link_libraries(${ex_name} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../../lib/mysql-connector-c-6.0.2-winx64/lib/libmysql.lib
      )
      install(
        FILES ${CMAKE_CURRENT_LIST_DIR}/../../lib/mysql-connector-c-6.0.2-winx64/lib/libmysql.dll
        DESTINATION .
        COMPONENT bin
      )
    else()
      target_link_libraries(${ex_name} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../../lib/mysql-connector-c-6.0.2-win32/lib/libmysql.lib
      )
      install(
        FILES ${CMAKE_CURRENT_LIST_DIR}/../../lib/mysql-connector-c-6.0.2-win32/lib/libmysql.dll
        DESTINATION .
        COMPONENT bin
      )
    endif()
  else()
    target_link_libraries(${ex_name} PUBLIC
      mysqlclient
    )
  endif()
endif()
if(DEFINED USE_BOOST_REGEX)
  target_link_libraries(${ex_name} PUBLIC
    boost_regex
  )
endif()

dist(${ex_name} .)
