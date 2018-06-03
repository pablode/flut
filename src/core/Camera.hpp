#pragma once

#include <Eigen/Core>

#include "core/Window.hpp"

namespace ansimproj {
  namespace core {
    class Camera {

    private:
      constexpr static float FOV = static_cast<float>(60.0f * M_PI / 180.0f);
      constexpr static float SENSITIVITY = 0.005f;
      constexpr static float NEAR_PLANE = 0.01f;
      constexpr static float FAR_PLANE = 25.0f;

    public:
      Camera(Window &window);

      ~Camera();

      void update(float dt);

      Eigen::Matrix4f view() const;

      Eigen::Matrix4f projection() const;

      Eigen::Matrix4f invProjection() const;

    private:
      void recalcView();

      void recalcProjection();

    private:
      Window &window_;
      std::uint32_t width_;
      std::uint32_t height_;
      Eigen::Matrix4f view_;
      Eigen::Matrix4f projection_;
      Eigen::Matrix4f invProjection_;
      Eigen::Vector3f position_;
      Eigen::Vector3f center_;
      Eigen::Vector3f up_;
      std::int32_t oldMouseX_;
      std::int32_t oldMouseY_;
      float radius_;
      float theta_;
      float phi_;
    };
  }
}
