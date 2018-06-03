include(ExternalProject)

set(glbinding_INSTALL_DIR "${3RDPARTY_INSTALL_DIR}")
set(glbinding_BUILD_DIR "${3RDPARTY_BINARY_DIR}/glbinding")

ExternalProject_Add(glbinding
  PREFIX ${glbinding_BUILD_DIR}
  GIT_REPOSITORY https://github.com/cginternals/glbinding.git
  GIT_TAG "f32808fdc4f684e51b2efc9f91ab71dc082d0ad0"
  GIT_SHALLOW ON
  GIT_PROGRESS ON
  INSTALL_DIR ${glbinding_INSTALL_DIR}
  BUILD_IN_SOURCE ON
  PATCH_COMMAND
    git checkout -- CMakeLists.txt
    COMMAND git apply --reject --ignore-space-change --ignore-whitespace ${3RDPARTY_SOURCE_DIR}/patches/glbinding.patch
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${glbinding_INSTALL_DIR}
    -DOPTION_BUILD_TESTS=OFF
    -DOPTION_BUILD_GPU_TESTS=OFF
    -DOPTION_BUILD_TOOLS=OFF)
