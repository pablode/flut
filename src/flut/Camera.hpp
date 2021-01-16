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
    const Window& window_;
    std::uint32_t width_;
    std::uint32_t height_;
    glm::mat4 view_;
    glm::mat4 projection_;
    glm::mat4 invProjection_;
    glm::vec3 position_;
    glm::vec3 center_;
    glm::vec3 up_;
    std::int32_t oldMouseX_;
    std::int32_t oldMouseY_;
    float radius_;
    float theta_;
    float phi_;
  };
}
