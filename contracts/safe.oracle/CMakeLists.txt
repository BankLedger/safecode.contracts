add_contract(safe.oracle safe.oracle ${CMAKE_CURRENT_SOURCE_DIR}/src/safe.oracle.cpp)

target_include_directories(safe.oracle
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(safe.oracle
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/safe.oracle.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/safe.oracle.contracts.md @ONLY )

target_compile_options( safe.oracle PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
