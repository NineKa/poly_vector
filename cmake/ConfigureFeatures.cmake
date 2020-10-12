#
cmake_minimum_required(VERSION 3.1.3)

try_compile(POLY_VECTOR_HAS_CXX_DISJUNCTION ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/has_disjunction.cpp CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
message("POLY_VECTOR_HAS_CXX_DISJUNCTION is ${POLY_VECTOR_HAS_CXX_DISJUNCTION}")

try_compile(POLY_VECTOR_HAS_CXX_ALLOCATOR_ALWAYS_EQUAL ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/has_allocator_always_equal.cpp CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
message("POLY_VECTOR_HAS_CXX_ALLOCATOR_ALWAYS_EQUAL is ${POLY_VECTOR_HAS_CXX_ALLOCATOR_ALWAYS_EQUAL}")

try_compile(POLY_VECTOR_HAS_CXX_STL_TYPE_ALIASES ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/has_stl_type_aliases.cpp CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
message("POLY_VECTOR_HAS_CXX_STL_TYPE_ALIASES is ${POLY_VECTOR_HAS_CXX_STL_TYPE_ALIASES}")


configure_file(${PROJECT_SOURCE_DIR}/include/poly/detail/vector_config.h.in ${CMAKE_BINARY_DIR}/gen/poly/detail/vector_config.h)
#
