#pragma once

#include <Eigen/Core>

#include "core/Window.hpp"

namespace ansimproj {
  namespace core {
    class Camera {

    private:
      const float SENSITIVITY = 0.01f;

    public:
      Camera(Window &window);

      ~Camera();

      void update(double deltaTime);

      Eigen::Matrix4f view() const;

      Eigen::Matrix4f projection() const;

    private:
      void recalcView();

      void recalcProjection();

    private:
      Window &window_;
      std::uint32_t width_;
      std::uint32_t height_;
      Eigen::Matrix4f view_;
      Eigen::Matrix4f projection_;
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
