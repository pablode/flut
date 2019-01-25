include(ExternalProject)

ExternalProject_Add(glbinding
  PREFIX ${3RDPARTY_SOURCE_DIR}
  GIT_REPOSITORY https://github.com/cginternals/glbinding.git
  GIT_TAG "f32808fdc4f684e51b2efc9f91ab71dc082d0ad0"
  GIT_SHALLOW ON
  GIT_PROGRESS ON
  INSTALL_DIR ${3RDPARTY_INSTALL_DIR}
  BUILD_IN_SOURCE ON
  PATCH_COMMAND
    git checkout -- CMakeLists.txt
    COMMAND git apply --reject --ignore-space-change --ignore-whitespace ${3RDPARTY_ROOT}/patches/glbinding.patch
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${3RDPARTY_INSTALL_DIR}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DOPTION_BUILD_TESTS=OFF
    -DOPTION_BUILD_GPU_TESTS=OFF
    -DOPTION_BUILD_TOOLS=OFF)
