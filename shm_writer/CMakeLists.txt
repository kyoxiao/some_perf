cmake_minimum_required (VERSION 2.6)
project(shm_writer CXX)
set(CMAKE_CXX_COMPILER g++)
file(GLOB SOURCES
  "src/*.cpp"
  "../shared/*.cpp")
file(GLOB INCLUDES
  "src"
  "inc"
  "../shared")
add_executable(shm_writer.bin ${SOURCES})
target_compile_options(shm_writer.bin PRIVATE -DDEBUG)
target_include_directories(shm_writer.bin PRIVATE ${INCLUDES})
target_link_libraries(shm_writer.bin PUBLIC ${THOSTTRADERAPI})
target_link_libraries (shm_writer.bin PUBLIC
  stdc++
  z
  rt
  pthread
)
