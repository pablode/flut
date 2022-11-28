#pragma once

#include <glm/glm.hpp>

#include "Window.hpp"

namespace flut
{
  class Camera
  {
  private:
    constexpr static float FOV = static_cast<float>(60.0f * M_PI / 180.0f);
    constexpr static float SENSITIVITY = 0.005f;
    constexpr static float NEAR_PLANE = 0.01f;
    constexpr static float FAR_PLANE = 1000.0f;
    constexpr static float INITIAL_RADIUS = 18.0f;

  public:
    Camera(const Window& window);

    ~Camera();

    void update(float dt);

    glm::mat4 view() const;

    glm::mat4 projection() const;

    glm::mat4 invProjection() const;

  private:
    void recalcView();

    void recalcProjection();

  private:
    const Window& m_window;
    uint32_t m_width;
    uint32_t m_height;
    glm::mat4 m_view;
    glm::mat4 m_projection;
    glm::mat4 m_invProjection;
    glm::vec3 m_position;
    glm::vec3 m_center;
    glm::vec3 m_up;
    int32_t m_oldMouseX;
    int32_t m_oldMouseY;
    float m_radius;
    float m_theta;
    float m_phi;
  };
}
