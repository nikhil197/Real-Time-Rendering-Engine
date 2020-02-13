#include "pch.h"

#include "Skybox.h"

#include "ErrorHandler.h"	// TODO: This should not be included outside of Core Submodule
#include "Model/Cube.h"

#include "Entities/Camera.h"
#include "Engine/Controllers/CameraController.h"

#include "Textures/CubeMap.h"
#include "Shaders/Shader.h"

#include "VertexArray.h"
#include "Buffers/IndexBuffer.h"
#include "Buffers/VertexBuffer.h"
#include "Buffers/VertexBufferLayout.h"

namespace GraphX
{
	Skybox::Skybox(const std::string& ShaderFilePath, const std::string& FilePath, const std::vector<std::string>& FileNames, const Ref<CameraController>& CameraController, const GM::Vector4& color, float factor, unsigned int slot, float Speed)
		: m_VAO(CreateScope<VertexArray>()), m_VBO(nullptr), m_IBO(nullptr), m_Shader(CreateRef<Shader>(ShaderFilePath)), m_CubeMap(new CubeMap(FilePath, FileNames)), m_CameraController(CameraController), m_BindingSlot(slot), m_Rotation(0.0f), m_BlendColor(color), RotationSpeed(Speed), BlendFactor(factor)
	{
		std::vector<unsigned int> indices = Cube::GetIndices();
		// Top face
		indices[6]  = 7;
		indices[7]  = 3;
		indices[8]  = 6;
		indices[9]  = 6;
		indices[10] = 3;
		indices[11] = 2;
		// Bottom face
		indices[18] = 0;
		indices[19] = 4;
		indices[20] = 1;
		indices[21] = 1;
		indices[22] = 4;
		indices[23] = 5;

		std::vector<GM::Vector3> vertices = Cube::GetVertexPositions();

		m_VBO = CreateScope<VertexBuffer>(&vertices[0], vertices.size() * sizeof(GM::Vector3));
		m_IBO = CreateRef<IndexBuffer>(&indices[0], indices.size());

		VertexBufferLayout layout = {
			{ BufferDataType::Float3 }
		};

		m_VAO->AddVertexBuffer(*m_VBO, layout);
		m_VAO->AddIndexBuffer(*m_IBO);

		m_View = m_CameraController->GetCamera()->GetViewMatrix();
		m_View[0][3] = 0.0f;
		m_View[1][3] = 0.0f;
		m_View[2][3] = 0.0f;

		m_Shader->Bind();
		m_Shader->SetUniform4f("u_BlendColor", m_BlendColor);
	}

	void Skybox::Update(float DeltaTime)
	{
		m_Rotation += RotationSpeed * DeltaTime;
		GM::Utility::ClampAngle(m_Rotation);
		m_View = m_View * GM::Rotation(RotationSpeed * DeltaTime, GM::Vector3::YAxis);
	}

	void Skybox::Enable(class Shader& shader, const std::string& Name) const
	{
	}

	void Skybox::Enable() const
	{
		GLCall(glDepthMask(GL_FALSE));
		GLCall(glDisable(GL_CULL_FACE));

		m_VAO->Bind();
		
		m_Shader->Bind();
		m_CubeMap->Bind(m_BindingSlot);

		// Set the uniforms
		m_Shader->SetUniformMat4f("u_View", m_View);

		if(m_CameraController->GetProjectionMode() == ProjectionMode::Perspective)
			m_Shader->SetUniformMat4f("u_Projection", m_CameraController->GetCamera()->GetProjectionMatrix());

		m_Shader->SetUniform1i("u_Skybox", m_BindingSlot);
		m_Shader->SetUniform1f("u_BlendFactor", BlendFactor);
	}

	void Skybox::Disable() const
	{
		GLCall(glDepthMask(GL_TRUE));
		GLCall(glEnable(GL_CULL_FACE));

		m_VAO->UnBind();
		
		m_Shader->UnBind();
		m_CubeMap->UnBind();
	}

	Skybox::~Skybox()
	{
		
	}
}