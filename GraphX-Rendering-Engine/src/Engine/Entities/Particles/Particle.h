#pragma once

#include "Entities/Entity.h"
#include "Model/Quad.h"

namespace GraphX
{
	class Texture2D;
	class Shader;

	struct ParticleProps
	{
		GM::Vector3 Position;
		GM::Vector3 Velocity;
		GM::Vector4 ColorBegin = GM::Vector4::UnitVector;
		GM::Vector4 ColorEnd = GM::Vector4::UnitVector;
		Ref<Texture2D> Texture = nullptr;
		float RotationMatrix = 0.0f;
		float SizeBegin = 1.0f, SizeEnd = 1.0f;
		float LifeSpan = 1.0f;
		float GravityEffect = 1.0f;
	};

	class Particle
		: public Entity
	{
	public:
		/* Default Consturctor for the particle pool */
		Particle();

		Particle(const ParticleProps& props);

		/* To Initialise the particles that are defualt constructed */
		void Init(const ParticleProps& props);

		/* Entity Interface */
		void Update(float DeltaTime, const GM::Vector3& CameraViewSpacePos, bool UpdateMatrix);

		virtual void Enable(Shader& shader, const std::string& EntityNameInShader = "") const override;
		
		virtual void Disable() const override;

		/* Returns the current properties of the particle */
		inline const ParticleProps& GetProps() const { return m_Props; }

		inline const GM::Vector4& GetTexOffsets() const { return m_TexOffset; }

		inline float GetBlendFactor() const { return m_BlendFactor; }

		inline float GetLifeProgress() const { return m_ElapsedTime / m_Props.LifeSpan; }

		/* Whether the particles is being used or not */
		inline bool IsActive() const { return m_Active; }

	private:
		void Update(float DeltaTime) override;
		
		/* Updated the texture atlas offset for the particle based on the particle lifespan */
		void UpdateTexOffset();

		/* Calculates the offset for the given index */
		void CalculateOffset(int index, float& xTexOffset, float& yOffset);

	private:
		/* Current Properties of the particle */
		ParticleProps m_Props;

		/* Offsets of the current and next texture to be used in the atlas */
		GM::Vector4 m_TexOffset;

		/* Time elapsed since the creation of the particle */
		float m_ElapsedTime = 0.0f;

		/* Model matrix for the particle */
		GM::Matrix4 m_Model;

		/* Factor used to blend between the two textures */
		float m_BlendFactor = 0.0f;

		/* Whether the current particle is being used or not (Used specially for particles pool) */
		bool m_Active = false;
	};
}