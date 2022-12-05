#pragma once

#include "Model.h"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace xing
{

struct TransformComponent
{
	glm::vec3 translation{};
	glm::vec3 scale{1.f, 1.f, 1.f};
	glm::vec3 rotation{};

	// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4();

	glm::mat3 normalMatrix();
};

struct PointLightComponent
{
	float lightIntensity = 1.0f;
};

class XingGameObject
{
  public:
	using id_t = unsigned int;
	using Map  = std::unordered_map<id_t, XingGameObject>;

	static XingGameObject createGameObject()
	{
		static id_t currentId = 0;
		return XingGameObject{currentId++};
	}

	static XingGameObject makePointLight(
	    float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

	XingGameObject(const XingGameObject &)            = delete;
	XingGameObject &operator=(const XingGameObject &) = delete;
	XingGameObject(XingGameObject &&)                 = default;
	XingGameObject &operator=(XingGameObject &&)      = default;

	id_t getId()
	{
		return id;
	}

	glm::vec3          color{};
	TransformComponent transform{};

	// Optional pointer components
	std::shared_ptr<Walnut::Model>       model{};
	std::unique_ptr<PointLightComponent> pointLight = nullptr;

  private:
	XingGameObject(id_t objId) :
	    id{objId}
	{}

	id_t id;
};
}        // namespace xing
