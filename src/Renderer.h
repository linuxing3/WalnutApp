#pragma once

#include "Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <glm/glm.hpp>
#include <memory>

class Renderer {
public:
  Renderer() = default;

  void OnResize(uint32_t width, uint32_t height);
  void Render(const Scene &scene, const Camera &camera);

  std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
  // Hit point info
  struct HitPayload {
    float HitDistance;
    glm::vec3 WorldPosition;
    glm::vec3 WorldNormal;

    int ObjectIndex;
  };

  // Ray gen perpixel color [light,material,reflect]
  // <- trace <- closestHit/miss
  glm::vec4 PerPixel(uint32_t x, uint32_t y);

  HitPayload TraceRay(const Ray &ray);
  HitPayload ClosestHit(const Ray &ray, float hitDistance, int objectIndex);
  HitPayload Miss(const Ray &ray);

private:
  std::shared_ptr<Walnut::Image> m_FinalImage;

  const Scene *m_ActiveScene = nullptr;
  const Camera *m_ActiveCamera = nullptr;

  uint32_t *m_ImageData = nullptr;
};
