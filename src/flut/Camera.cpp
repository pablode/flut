#include "Camera.hpp"
#include "Window.hpp"

#include <algorithm>
#include <iostream>
#include <math.h>

using namespace flut;

Camera::Camera(const Window& window)
  : m_window(window)
{
  m_width = window.width();
  m_height = window.height();
  m_radius = INITIAL_RADIUS;
  m_theta = static_cast<float>(M_PI) / 2.0f;
  m_phi = 0.0f;
  m_up = {0.0f, 1.0f, 0.0f};
  m_center = {0.0f, 0.0f, 0.0f};
  m_position = {0.0f, 0.0f, m_radius};
  m_oldMouseX = m_window.mouseX();
  m_oldMouseY = m_window.mouseY();
  m_projection = glm::mat4();
  m_invProjection = glm::mat4();
  m_view = glm::mat4(1);
  recalcView();
  recalcProjection();
}

Camera::~Camera() {}

void Camera::update(float dt)
{
  const auto& mouseX = m_window.mouseX();
  const auto& mouseY = m_window.mouseY();
  bool recalcPos = false;

  if (m_window.mouseDown())
  {
    const float deltaX = (mouseX - m_oldMouseX) * SENSITIVITY;
    const float deltaY = (mouseY - m_oldMouseY) * SENSITIVITY;

    m_theta -= deltaY;
    if (m_theta < 0.01f) {
      m_theta = 0.01f;
    }
    else if (m_theta > M_PI - 0.01f) {
      m_theta = static_cast<float>(M_PI - 0.01f);
    }

    m_phi -= deltaX;
    if (m_phi < 0.0f) {
      m_phi += 2 * M_PI;
    }
    else if (m_phi > 2 * M_PI) {
      m_phi -= 2 * M_PI;
    }

    recalcPos = true;
  }

  if (m_window.keyUp())
  {
    m_radius = std::max(0.001f, m_radius - 5.0f * dt);
    recalcPos = true;
  }
  else if (m_window.keyDown())
  {
    m_radius += 5.0f * dt;
    recalcPos = true;
  }

  if (recalcPos)
  {
    const float x = m_center[0] + m_radius * sin(m_theta) * sin(m_phi);
    const float y = m_center[1] + m_radius * cos(m_theta);
    const float z = m_center[2] + m_radius * sin(m_theta) * cos(m_phi);
    m_position = {x, y, z};
    recalcView();
  }

  m_oldMouseX = mouseX;
  m_oldMouseY = mouseY;

  if (m_width != m_window.width() || m_height != m_window.height())
  {
    m_width = m_window.width();
    m_height = m_window.height();
    recalcProjection();
  }
}

void Camera::recalcView()
{
  const glm::vec3 f = glm::normalize(m_center - m_position);
  const glm::vec3 s = glm::normalize(glm::cross(f, m_up));
  const glm::vec3 u = glm::normalize(glm::cross(s, f));
  m_view[0][0] = s.x;
  m_view[1][0] = s.y;
  m_view[2][0] = s.z;
  m_view[3][0] = -glm::dot(s, m_position);
  m_view[0][1] = u.x;
  m_view[1][1] = u.y;
  m_view[2][1] = u.z;
  m_view[3][1] = -glm::dot(u, m_position);
  m_view[0][2] = -f.x;
  m_view[1][2] = -f.y;
  m_view[2][2] = -f.z;
  m_view[3][2] = glm::dot(f, m_position);
}

void Camera::recalcProjection()
{
  const float aspect = static_cast<float>(m_width) / m_height;
  const float theta = static_cast<float>(FOV * 0.5);
  const float range = FAR_PLANE - NEAR_PLANE;
  const float invtan = 1.0f / std::tan(theta);
  m_projection[0][0] = invtan / aspect;
  m_projection[1][1] = invtan;
  m_projection[2][2] = -(NEAR_PLANE + FAR_PLANE) / range;
  m_projection[2][3] = -1.0f;
  m_projection[3][2] = -(2.0f * NEAR_PLANE * FAR_PLANE) / range;
  m_invProjection = glm::inverse(m_projection);
}

glm::mat4 Camera::view() const
{
  return m_view;
}

glm::mat4 Camera::projection() const
{
  return m_projection;
}

glm::mat4 Camera::invProjection() const
{
  return m_invProjection;
}
