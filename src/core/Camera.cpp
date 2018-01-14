#include "core/Camera.hpp"
#include "core/Window.hpp"

#define _USE_MATH_DEFINES
#include <Eigen/Geometry>
#include <iostream>
#include <math.h>

ansimproj::core::Camera::Camera(Window &window)
  : window_(window) {
  width_ = window.width();
  height_ = window.height();
  radius_ = 6.5f;
  theta_ = static_cast<float>(M_PI) / 2.0f;
  phi_ = 0.0f;
  up_ = {0.0f, 1.0f, 0.0f};
  center_ = {0.0f, 0.0f, 0.0f};
  position_ = {0.0f, 0.0f, radius_};
  oldMouseX_ = window_.mouseX();
  oldMouseY_ = window_.mouseY();
  recalcView();
  recalcProjection();
}

ansimproj::core::Camera::~Camera() {}

void ansimproj::core::Camera::update(float deltaTime) {
  const auto &mouseX = window_.mouseX();
  const auto &mouseY = window_.mouseY();
  if (window_.mouseDown()) {
    const float deltaX = (mouseX - oldMouseX_) * SENSITIVITY;
    const float deltaY = (mouseY - oldMouseY_) * SENSITIVITY;

    theta_ -= deltaY;
    if (theta_ < 0.01f)
      theta_ = 0.01f;
    else if (theta_ > M_PI - 0.01f)
      theta_ = static_cast<float>(M_PI - 0.01f);

    phi_ -= deltaX;
    if (phi_ < 0.0f)
      phi_ += 2 * M_PI;
    else if (phi_ > 2 * M_PI)
      phi_ -= 2 * M_PI;

    const float x = center_[0] + radius_ * sin(theta_) * sin(phi_);
    const float y = center_[1] + radius_ * cos(theta_);
    const float z = center_[2] + radius_ * sin(theta_) * cos(phi_);
    position_ = {x, y, z};

    recalcView();
  }
  oldMouseX_ = mouseX;
  oldMouseY_ = mouseY;

  if (width_ != window_.width() || height_ != window_.height()) {
    width_ = window_.width();
    height_ = window_.height();
    recalcProjection();
  }
}

void ansimproj::core::Camera::recalcView() {
  Eigen::Vector3f f = (center_ - position_).normalized();
  Eigen::Vector3f u = up_.normalized();
  Eigen::Vector3f s = f.cross(u).normalized();
  u = s.cross(f);
  Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
  mat(0, 0) = s.x();
  mat(0, 1) = s.y();
  mat(0, 2) = s.z();
  mat(0, 3) = -s.dot(position_);
  mat(1, 0) = u.x();
  mat(1, 1) = u.y();
  mat(1, 2) = u.z();
  mat(1, 3) = -u.dot(position_);
  mat(2, 0) = -f.x();
  mat(2, 1) = -f.y();
  mat(2, 2) = -f.z();
  mat(2, 3) = f.dot(position_);
  mat.row(3) << 0, 0, 0, 1;
  view_ = mat;
}

void ansimproj::core::Camera::recalcProjection() {
  constexpr float FOV = static_cast<const float>(60.0f * M_PI / 180.0f);
  constexpr float FAR_PLANE = 50.0f;
  constexpr float NEAR_PLANE = 0.1f;
  const float aspect = static_cast<float>(width_) / height_;
  const float theta = static_cast<float>(FOV * 0.5);
  const float range = FAR_PLANE - NEAR_PLANE;
  const float invtan = static_cast<float>(1.0f / tan(theta));
  projection_.setIdentity();
  projection_(0, 0) = invtan / aspect;
  projection_(1, 1) = invtan;
  projection_(2, 2) = -(NEAR_PLANE + FAR_PLANE) / range;
  projection_(3, 2) = -1;
  projection_(2, 3) = -2 * NEAR_PLANE * FAR_PLANE / range;
  projection_(3, 3) = 0;
}

Eigen::Matrix4f ansimproj::core::Camera::view() const {
  return view_;
}

Eigen::Matrix4f ansimproj::core::Camera::projection() const {
  return projection_;
}
