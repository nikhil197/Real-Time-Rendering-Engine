#include "pch.h"

#include "Mesh3D.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Vertex.h"
#include "Buffers/VertexBuffer.h"
#include "Buffers/VertexBufferLayout.h"
#include "Buffers/IndexBuffer.h"

namespace engine
{
	Mesh3D::Mesh3D(const gm::Vector3& Pos, const gm::Vector3& Rotation, const gm::Vector3& Scale, Shader& shader, const std::vector<const Texture*> Textures, const std::vector<Vector3D>& Vertices, const std::vector<unsigned int>& Indices)
		: Position(Pos), Rotation(Rotation), Scale(Scale), bShowDetails(0), m_Shader(shader), m_Textures(Textures), m_Vertices(Vertices), m_Indices(Indices)
	{
		m_VAO = new VertexArray();
		m_VBO = new VertexBuffer(&m_Vertices[0], m_Vertices.size() * sizeof(Vertex3D));
		VertexBufferLayout layout = Vertex3D::GetVertexLayout();
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO = new IndexBuffer(&m_Indices[0], m_Indices.size());
	}

	gm::Matrix4 Mesh3D::GetTransformationMatrix() const
	{
		gm::Translation translation(Position);
		gm::Rotation rotation(Rotation);
		gm::Scaling scale(Scale);

		return translation * rotation * scale;
	}

	Mesh3D::~Mesh3D()
	{
		delete m_VBO;
		delete m_IBO;
		delete m_VAO;
	}
}