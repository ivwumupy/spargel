
set(THIRD_PARTY_DIR ${CMAKE_BINARY_DIR}/third)

spargel_add_option(GIT_USE_SSH "git use ssh" OFF)

if (GIT_USE_SSH)
    set(GLFW_GIT_URL git@github.com:glfw/glfw.git)
else ()
    set(GLFW_GIT_URL https://github.com/glfw/glfw.git)
endif ()


################
# 1. FetchContent
################

# download at CONFIG time
# clone repo somewhere in BINARY_DIR
# just like using a normal third-party project

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY ${GLFW_GIT_URL}
  GIT_TAG        master
)

FetchContent_MakeAvailable(glfw)


################
# 2. ExternalProject
################

# download at BUILD time
# library will be separately configured and built (isolated)
# need to explicitly specify include/link target files/directories

include(ExternalProject)

ExternalProject_Add(
  glfw2
  PREFIX ${THIRD_PARTY_DIR}/glfw
  GIT_REPOSITORY ${GLFW_GIT_URL}
  GIT_TAG master
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_DIR}/glfw
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
)

# need to do this manually
include_directories(${THIRD_PARTY_DIR}/glfw/include)
