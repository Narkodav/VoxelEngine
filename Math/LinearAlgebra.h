#pragma once
#include "Common.h"

namespace LinearAlgebra
{
	class CoordinateSystem
	{
	private:
		glm::vec3 x = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 y = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 z = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

		void orthonormalize() {
			// Gram-Schmidt process to ensure orthonormality
			x = glm::normalize(x);

			y = y - glm::dot(y, x) * x;
			y = glm::normalize(y);

			z = glm::cross(x, y);
			z = glm::normalize(z);

			if (glm::dot(glm::cross(x, y), z) < 0) {
				z = -z;
			}
		}

	public:
		static constexpr CoordinateSystem baseSystem()
		{
			return CoordinateSystem();
		}

		constexpr CoordinateSystem() = default;
		CoordinateSystem(glm::vec3 x, glm::vec3 y, glm::vec3 z)
			: x(x), y(y), z(z) {
			orthonormalize();
		};
		CoordinateSystem(glm::vec3 x, glm::vec3 y, glm::vec3 z, glm::vec3 offset)
			: x(x), y(y), z(z), offset(offset){
			orthonormalize();
		};

		constexpr CoordinateSystem(const CoordinateSystem&) = default;
		constexpr CoordinateSystem& operator=(const CoordinateSystem&) = default;
		constexpr CoordinateSystem(CoordinateSystem&&) = default;
		constexpr CoordinateSystem& operator=(CoordinateSystem&&) = default;

		const glm::vec3& getX() const { return x; }
		const glm::vec3& getY() const { return y; }
		const glm::vec3& getZ() const { return z; }
		const glm::vec3& getOffset() const { return offset; }
		void setOffset(const glm::vec3& offset) { this->offset = offset; };

		glm::vec3 worldToLocal(const glm::vec3& worldVec) const {
			glm::vec3 offseted = worldVec - offset;
			return glm::vec3(
				glm::dot(offseted, x),
				glm::dot(offseted, y),
				glm::dot(offseted, z)
			);
		}

		constexpr glm::vec3 localToWorld(const glm::vec3& localVec) const {
			return localVec.x * x + localVec.y * y + localVec.z * z + offset;
		}

		glm::vec2 projectOnXYPlane(const glm::vec3& worldVec) const {
			glm::vec3 offseted = worldVec - offset;
			return glm::vec2(
				glm::dot(offseted, x),
				glm::dot(offseted, y)
			);
		}

		bool isValid(float tolerance = 1e-6f) const {
			return std::abs(glm::length(x) - 1.0f) < tolerance &&
				std::abs(glm::length(y) - 1.0f) < tolerance &&
				std::abs(glm::length(z) - 1.0f) < tolerance &&
				std::abs(glm::dot(x, y)) < tolerance &&
				std::abs(glm::dot(x, z)) < tolerance &&
				std::abs(glm::dot(y, z)) < tolerance &&
				glm::dot(glm::cross(x, y), z) > 0.99f; // Right-handed
		}

		void alignWithZAxis(glm::vec3 axis)
		{
			glm::normalize(axis);

			auto rotAxis = glm::cross(z, axis);
			float dot = glm::dot(z, axis);
			if (glm::length(rotAxis) > std::numeric_limits<float>::epsilon())
			{
				rotAxis = glm::normalize(rotAxis);
				auto angle = glm::acos(dot);
				glm::mat3 rot = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, rotAxis));
				x = rot * x;
				y = rot * y;
				z = rot * z;
			}
			else if (dot < 0) //vectors are opposite
			{
				x = -x;
				z = -z;
			}

			assert(glm::length(z - axis) < 1e-6f); // x should align with normal
			assert(std::abs(glm::dot(x, y)) < 1e-6f); // should be orthogonal
			assert(std::abs(glm::dot(x, z)) < 1e-6f); // should be orthogonal
			assert(std::abs(glm::dot(y, z)) < 1e-6f); // should be orthogonal
			assert(std::abs(glm::length(x) - 1.0f) < 1e-6f); // should be unit length
			assert(std::abs(glm::length(y) - 1.0f) < 1e-6f); // should be unit length
			assert(std::abs(glm::length(z) - 1.0f) < 1e-6f); // should be unit length
		}
	};
};

