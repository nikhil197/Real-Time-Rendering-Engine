#include "pch.h"
#include "Terrain.h"

#include "Utilities/EngineUtil.h"
#include "Model/Mesh/Mesh3D.h"

#include "Engine/Core/Vertex.h"
#include "Shaders/Shader.h"
#include "Materials/Material.h"
#include "Textures/Texture2D.h"

#include "VertexArray.h"
#include "Buffers/VertexBuffer.h"
#include "Buffers/IndexBuffer.h"


namespace GraphX
{
	using namespace GM;

	double SmoothNoise(int x, int y)
	{
		static unsigned long long seed = 7436767332u;
		double corners = EngineUtil::Rand(x - 1, y - 1, seed) + EngineUtil::Rand(x + 1, y - 1, seed)
			+ EngineUtil::Rand(x - 1, y + 1, seed) + EngineUtil::Rand(x + 1, y + 1, seed) / 16.0;

		double sides = EngineUtil::Rand(x - 1, y, seed) + EngineUtil::Rand(x + 1, y, seed)
			+ EngineUtil::Rand(x, y + 1, seed) + EngineUtil::Rand(x, y - 1, seed) / 8.0;

		double center = EngineUtil::Rand(x, y, seed) / 4.0;
		return corners + sides + center;
	}

	double InterpolatedNoise(double x, double y)
	{
		GX_PROFILE_FUNCTION()

		int intX = (int)x;
		int intZ = (int)y;
		double fracX = x - intX;
		double fracZ = y - intZ;

		double v1 = SmoothNoise(intX, intZ);
		double v2 = SmoothNoise(intX + 1, intZ);
		double v3 = SmoothNoise(intX, intZ + 1);
		double v4 = SmoothNoise(intX + 1, intZ + 1);

		float blend = (float)(1.0 - std::cos(fracX * PI)) * 0.5f;
		double i1 = Utility::Lerp(v1, v2, blend);
		double i2 = Utility::Lerp(v3, v4, blend);

		blend = (float)(1.0 - std::cos(fracZ * PI)) * 0.5f;
		return Utility::Lerp(i1, i2, blend);
	}

	double Terrain::s_Amplitude = 5.0;

	Terrain::Terrain(int TilesX, int TilesY, float TileSize, const std::vector<std::string>& TexNames, const std::string& BlendMap, const Vector3& Position, const Vector2& Scale)
		: m_Mesh(nullptr), m_Material(nullptr), m_TilesX(TilesX), m_TilesY(TilesY), m_TileSize(TileSize), m_Vertices(nullptr), m_Indices(nullptr)
	{
		GX_PROFILE_FUNCTION()

		BuildTerrain();
		Ref<Shader> shader = CreateRef<Shader>("res/Shaders/TerrainShader.glsl");
		shader->Bind();
		shader->SetUniform2i("u_TerrainDimensions", m_TilesX, m_TilesY);
		shader->SetUniform1f("u_AmbientStrength", 0.01f);

		m_Material = CreateRef<Material>(shader);
		m_Material->SetSpecularStrength(1.0f);
		m_Material->SetShininess(256.0f);
		
		m_BlendMap = CreateRef<const Texture2D>(BlendMap);
		
		if (TexNames.size() > 0)
		{
			for (unsigned int i = 0; i < TexNames.size(); i++)
			{
				Ref<const Texture2D> tex = CreateRef<const Texture2D>(TexNames[i], true); // All terrain textures will be tiled textures
				m_Material->AddTexture(tex);
			}
		}
		
		if (m_Vertices && m_Indices)
		{
			m_Mesh = CreateRef<Mesh3D>(Position, Rotator::ZeroRotator, Vector3(Scale.x, 1.0f, Scale.y), *m_Vertices, *m_Indices, m_Material);
			
			delete m_Vertices;
			delete m_Indices;
		}
		else
		{
			GX_ENGINE_ERROR("Error while building the terrain");
		}
	}

	void Terrain::BuildTerrain()
	{
		Timer timer("Build Terrain");
		GX_ENGINE_INFO("Building Terrain");
		GX_PROFILE_FUNCTION()
		
		m_Vertices = new std::vector<Vertex3D>();
		m_Indices = new std::vector<unsigned int>();
		Vertex3D vertex;
		for (int x_New = 0; x_New < m_TilesX; x_New++)
		{
			for (int y_New = 0; y_New < m_TilesY; y_New++)
			{
				// Calculate the vertices of the terrain
				//double zCoord = GetZCoords(x, z);
				vertex.Position = Vector3(-x_New * m_TileSize, y_New * m_TileSize, /*(float)yCoord*/-10.0f);
				vertex.TexCoord = Vector2((float)y_New, (float)x_New);
				m_Vertices->emplace_back(vertex);

				// Calculate the indices for the vertices of the terrain
				if (y_New != m_TilesX - 1 && x_New != m_TilesY - 1)
				{
					// Lower triangle
					m_Indices->push_back(x_New * m_TilesX + y_New);
					m_Indices->push_back(x_New * m_TilesX + y_New + 1);
					m_Indices->push_back(((x_New + 1) * m_TilesX) + y_New + 1);

					// Upper triangle
					m_Indices->push_back(((x_New + 1) * m_TilesX) + y_New + 1);
					m_Indices->push_back(((x_New + 1) * m_TilesX) + y_New);
					m_Indices->push_back(x_New * m_TilesX + y_New);
				}
			}
		}

		for (int x = 0; x < m_TilesX; x++)
		{
			for (int y = 0; y < m_TilesY; y++)
			{
				CalculateNormal(x, y);
			}
		}

		// Reset the seed back to default
		EngineUtil::ResetSeed();
	}
	

	double Terrain::GetZCoords(int x, int y)
	{
		double total = InterpolatedNoise(x / 8.0 , y / 8.0 ) * s_Amplitude;
		total += InterpolatedNoise(x / 4.0 , y / 4.0 ) * s_Amplitude / 3.0 ;
		total += InterpolatedNoise(x / 2.0 , y / 2.0 ) * s_Amplitude / 9.0 ;
		return total;
	}

	void Terrain::CalculateNormal(int x, int y)
	{
		GX_PROFILE_FUNCTION()

		float heightL = m_Vertices->at(Utility::Max((x - 1) + y * m_TilesX, 0)).Position.y;
		float heightR = m_Vertices->at(Utility::Min((x + 1) + y * m_TilesX, (int)m_Vertices->size() - 1)).Position.y;
		float heightD = m_Vertices->at(Utility::Max(x + (y - 1) * m_TilesX, 0)).Position.y;
		float heightU = m_Vertices->at(Utility::Min(x + (y + 1) * m_TilesX, (int)m_Vertices->size() - 1)).Position.y;
		m_Vertices->at(x + y * m_TilesX).Normal = Vector3(heightL - heightR, 2.0, heightD - heightU).Normal();
	}
	
	void Terrain::Update(float DeltaTime)
	{
		if (m_Mesh)
		{
			m_Mesh->Update(DeltaTime);
		}
	}

	void Terrain::Enable(class Shader& shader, const std::string& Name) const
	{
	}

	void Terrain::Enable() const
	{
		GX_PROFILE_FUNCTION()

		m_Material->Bind();
		m_BlendMap->Bind(4);
		m_Material->GetShader()->SetUniform1i("u_BlendMap", 4);

		if (m_Mesh)
		{
			m_Mesh->Enable();
		}
	}

	void Terrain::Disable() const
	{
		GX_PROFILE_FUNCTION()

		m_BlendMap->UnBind();
		if (m_Mesh)
		{
			m_Mesh->Disable();
		}
	}

	bool Terrain::InitResources()
	{
		if (m_Mesh)
			return m_Mesh->InitResources();
		return false;
	}

	bool Terrain::ReleaseResources()
	{
		if (m_Mesh)
			return m_Mesh->ReleaseResources();

		return true;
	}

	Terrain::~Terrain()
	{
	}
}