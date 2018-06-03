include(ExternalProject)

set(Eigen_INSTALL_DIR "${3RDPARTY_INSTALL_DIR}")
set(Eigen_BUILD_DIR "${3RDPARTY_BINARY_DIR}/Eigen")

ExternalProject_Add(Eigen
  PREFIX ${Eigen_BUILD_DIR}
  URL http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2
  URL_MD5 a7aab9f758249b86c93221ad417fbe18
  INSTALL_DIR ${Eigen_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${Eigen_INSTALL_DIR})
