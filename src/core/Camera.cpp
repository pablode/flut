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
  projection_.setZero();
  view_.setIdentity();
  recalcView();
  recalcProjection();
}

ansimproj::core::Camera::~Camera() {}

void ansimproj::core::Camera::update(float dt) {
  const auto &mouseX = window_.mouseX();
  const auto &mouseY = window_.mouseY();
  bool recalcPos = false;

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

    recalcPos = true;
  }

  if (window_.keyUp()) {
    radius_ = std::max(0.001f, radius_ - 5.0f * dt);
    recalcPos = true;
  } else if (window_.keyDown()) {
    radius_ += 5.0f * dt;
    recalcPos = true;
  }

  if (recalcPos) {
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
  Eigen::Vector3f s = f.cross(up_).normalized();
  Eigen::Vector3f u = s.cross(f).normalized();
  view_(0, 0) = s.x();
  view_(0, 1) = s.y();
  view_(0, 2) = s.z();
  view_(0, 3) = -s.dot(position_);
  view_(1, 0) = u.x();
  view_(1, 1) = u.y();
  view_(1, 2) = u.z();
  view_(1, 3) = -u.dot(position_);
  view_(2, 0) = -f.x();
  view_(2, 1) = -f.y();
  view_(2, 2) = -f.z();
  view_(2, 3) = f.dot(position_);
}

void ansimproj::core::Camera::recalcProjection() {
  const float aspect = static_cast<float>(width_) / height_;
  const float theta = static_cast<float>(FOV * 0.5);
  const float range = FAR_PLANE - NEAR_PLANE;
  const float invtan = 1.0f / std::tan(theta);
  projection_(0, 0) = invtan / aspect;
  projection_(1, 1) = invtan;
  projection_(2, 2) = -(NEAR_PLANE + FAR_PLANE) / range;
  projection_(3, 2) = -1.0f;
  projection_(2, 3) = -(2.0f * NEAR_PLANE * FAR_PLANE) / range;
}

Eigen::Matrix4f ansimproj::core::Camera::view() const {
  return view_;
}

Eigen::Matrix4f ansimproj::core::Camera::projection() const {
  return projection_;
}
