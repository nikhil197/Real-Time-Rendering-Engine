#include "pch.h"
#include "Application.h"

#include "Engine/Core/VertexArray.h"
#include "Engine/Core/Shaders/Shader.h"
#include "Engine/Core/Materials/Material.h"

/* Buffers */
#include "Engine/Core/Buffers/VertexBuffer.h"
#include "Engine/Core/Buffers/VertexBufferLayout.h"
#include "Engine/Core/Buffers/IndexBuffer.h"
#include "Engine/Core/Buffers/FrameBuffer.h"

#include "Engine/Core/Textures/Texture.h"

/* Renderer */
#include "Renderer/Renderer.h"

/* Controllers */
#include "Engine/Controllers/CameraController.h"

/* Entities */
#include "Engine/Entities/Lights/PointLight.h"
#include "Engine/Entities/Lights/DirectionalLight.h"
#include "Engine/Entities/Camera.h"
#include "Engine/Entities/Skybox.h"
#include "Engine/Entities/Terrain.h"
#include "Engine/Entities/Particles/Particle.h"
#include "Engine/Entities/Particles/ParticleManager.h"
#include "Engine/Entities/Particles/ParticleSystem.h"

#include "Window.h"
#include "Timer/Clock.h"
#include "Gui/GraphXGui.h"

/* Events */
#include "Engine/Events/WindowEvent.h"
#include "Engine/Events/KeyboardEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/GUIEvent.h"

/* Input */
#include "Engine/Input/Keyboard.h"
#include "Engine/Input/Mouse.h"

/* Model */
#include "Model/Mesh/Vertex.h"
#include "Engine/Model/ModelTypes.h"
#include "Engine/Model/Model3D.h"
#include "Engine/Model/Mesh/Mesh2D.h"
#include "Engine/Model/Mesh/Mesh3D.h"
#include "Engine/model/Cube.h"

/* Utils */
#include "Engine/Utilities/EngineUtil.h"
#include "Engine/Utilities/FileOpenDialog.h"

namespace GraphX
{
	using namespace GM;

	Application::Application(std::string& title, int width, int height)
		: m_Window(nullptr), m_Title(title), m_IsRunning(true), m_EngineDayTime(0.1f), m_SelectedObject2D(nullptr), m_SelectedObject3D(nullptr), m_SunLight(nullptr), m_ShadowBuffer(nullptr), m_DepthShader(nullptr), m_CameraController(nullptr), m_DaySkybox(nullptr), m_NightSkybox(nullptr), m_CurrentSkybox(nullptr), m_ParticlesManager(nullptr), m_Shader(nullptr), m_DefaultMaterial(nullptr), m_Light(nullptr), m_DefaultTexture(nullptr)
	{
		// Initialise the clock and the logging, and the input devices
		Log::Init();
		Clock::Init();
		Mouse::Init();
		Keyboard::Init();
		
		m_Window = new Window(m_Title, width, height);

		InitializeApplication();
	}

	void Application::InitializeApplication()
	{
		// Set the event callback with the window
		m_Window->SetEventCallback(BIND_EVENT_FUNC(Application::OnEvent));

		// Initialise the renderer
		Renderer::Initialize();

		m_CameraController = CreateRef<CameraController>(GM::Vector3(0.0f, 0.0f, 3.0f), GM::Vector3::ZeroVector, GM::Vector3::YAxis, (float)m_Window->GetWidth() / (float)m_Window->GetHeight(), GX_ENGINE_NEAR_PLANE, GX_ENGINE_FAR_PLANE);

		std::vector<std::string> SkyboxNames = { "right.png", "left.png" , "top.png" , "bottom.png" , "front.png" , "back.png" };
		m_DaySkybox  = CreateRef<Skybox>("res/Shaders/Skybox.shader", "res/Textures/Skybox/Day/", SkyboxNames, m_CameraController, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		m_NightSkybox = CreateRef<Skybox>("res/Shaders/Skybox.shader", "res/Textures/Skybox/Night/", SkyboxNames, m_CameraController, Vector4(0.5f, 0.5f, 0.5f, 1.0f), 0.0f, 0, 0.f);

		m_CurrentSkybox = m_NightSkybox;

		m_SunLight = CreateRef<DirectionalLight>(GM::Vector4::UnitVector, GM::Vector3(-3.0f, -1.0f, 1.0f));
		m_Lights.emplace_back(m_SunLight);

		// Basic Lighting Shader 
		m_Shader = CreateRef<Shader>("res/Shaders/BasicLightingShader.shader");
		m_Shader->Bind();
		m_Shaders.push_back(m_Shader);

		m_DefaultMaterial = CreateRef<Material>(m_Shader);

		m_Light = CreateRef<PointLight>(Vector3(0, 50.0f, 50.0f), Vector4(1, 1, 1, 1));
		m_Lights.emplace_back(m_Light);

		m_ShadowBuffer = CreateRef<FrameBuffer>(m_Window->GetWidth(), m_Window->GetHeight(), FramebufferType::GX_FRAME_DEPTH);
		m_DepthShader = CreateRef<Shader>("res/Shaders/Depth.shader");

		m_ParticlesManager = CreateRef<ParticleManager>();
		m_ParticlesManager->Initialize(m_CameraController->GetCamera(), 1000);	// TODO: Move to a suitable location

		m_DefaultTexture  = CreateRef<Texture>("res/Textures/stone.jpg");
	}

	void Application::Run()
	{
		m_Shader->Bind();
		m_Shader->SetUniform1f("u_AmbientStrength", 0.1f);
		m_Shader->SetUniform1f("u_Shininess", 256.0f);
		m_Shader->SetUniform1f("u_Reflectivity", 1.0f);
		
		m_Shader->SetUniform3f("u_LightPos", m_Light->Position);
		m_Shader->SetUniform4f("u_LightColor", m_Light->Color);

		// For the purpose of fps count
		int times = 0;
		float then = Clock::GetClock()->GetEngineTime();

		std::vector<Ref<const Texture>> textures(0);
		textures.push_back(m_DefaultTexture);

		Ref<Material> CubeMaterial = CreateRef<Material>(m_Shader);
		CubeMaterial->AddTexture(m_DefaultTexture);
		
		Ref<Cube> cube = CreateRef<Cube>(GM::Vector3(-10.0f, 10.0f, -5.0f), GM::Vector3::ZeroVector, GM::Vector3::UnitVector, CubeMaterial);
		m_Objects3D.emplace_back(cube);
		cube->bShowDetails = true;

		Ref<Terrain> ter = CreateRef<Terrain>(250, 250, 2.0f, std::vector<std::string>({"res/Textures/Terrain/Grass.png", "res/Textures/Terrain/GrassFlowers.png", "res/Textures/Terrain/Mud.png", "res/Textures/Terrain/Path.png"}), "res/Textures/Terrain/BlendMap.png", Vector3(-249.0f, 10.0f, 249.0f), Vector2(1.0f, 1.0f));
		m_Terrain.emplace_back(ter);

		// Load Trees
		Ref<Material> TreeMaterial = CreateRef<Material>(m_Shader);
		TreeMaterial->AddTexture(CreateRef<const Texture>("res/Textures/tree.png"));

		Model3D TreeModel("res/Models/tree.obj", TreeMaterial);
		Ref<Mesh3D> TreeMesh = TreeModel.GetMeshes()->at(0);
		TreeMesh->Scale = 2.5f * Vector3::UnitVector;
		unsigned int NumTree = 100;
		for (unsigned int i = 0; i < NumTree; i++)
		{
			Vector3 Position((2 * (float)EngineUtil::GetRandomValue() - 1) * ter->GetWidth() / 2, 0.0f, (2 * (float)EngineUtil::GetRandomValue() - 1) * ter->GetDepth() / 2);
			TreeMesh->Position = Position;
			m_Objects3D.emplace_back(new Mesh3D(*TreeMesh));
		}

		// Load Low Poly Trees
		Ref<Material> LowPolyTreeMaterial = CreateRef<Material>(m_Shader);
		LowPolyTreeMaterial->AddTexture(CreateRef<const Texture>("res/Textures/lowPolyTree.png"));
		
		Model3D LowPolyTreeModel("res/Models/lowPolyTree.obj", LowPolyTreeMaterial);
		Ref<Mesh3D> LowPolyTreeMesh = LowPolyTreeModel.GetMeshes()->at(0);
		LowPolyTreeMesh->Scale = Vector3::UnitVector;
		NumTree = 10;
		for (unsigned int i = 0; i < NumTree; i++)
		{
			Vector3 Position((2 * (float)EngineUtil::GetRandomValue() - 1) * ter->GetWidth() / 2, 0.0f, (2 * (float)EngineUtil::GetRandomValue() - 1) * ter->GetDepth() / 2);
			LowPolyTreeMesh->Position = Position;
			m_Objects3D.emplace_back(new Mesh3D(*LowPolyTreeMesh));
		}

		// Load Stall
		Ref<Material> StallMaterial = CreateRef<Material>(m_Shader);
		StallMaterial->AddTexture(CreateRef<const Texture>("res/Textures/stallTexture.png"));

		Model3D StallModel("res/Models/stall.obj", StallMaterial);
		StallModel.GetMeshes()->at(0)->Position = Vector3(75.0f, 0.0f, -100.0f);
		m_Objects3D.emplace_back(StallModel.GetMeshes()->at(0));

		m_Shader->UnBind();

		Ref<Texture> particleTex = CreateRef<Texture>("res/Textures/Particles/particleAtlas.png", false, 4);
		ParticleSystem particleSys(m_ParticlesManager, particleTex, 50.0f, 2.0f, 0.5f, 2.0f, 1.0f, 0.5f, 0.4f, 0.5f, 1.0f);

		// Draw while the window doesn't close
		while (m_IsRunning)
		{
			// Frame Time in seconds
			float DeltaTime = Clock::GetClock()->GetDeltaTime();
			
			// Tick the clock every frame to get the delta time
			Clock::GetClock()->Tick();

			// Calculate the fps
			times++;
			float now = Clock::GetClock()->GetEngineTime();
			if ((now - then) > 1.0f)
			{
				GX_ENGINE_INFO("Frame Rate: {0} FPS", times);
				then = now;
				times = 0;
			}

			// Update the Gui
			GraphXGui::Update();

			// No need to update or render stuff if the application (window) is minimised
			if (!m_IsMinimised)
			{
				if (GX_ENABLE_PARTICLE_EFFECTS)
				{
					particleSys.SpawnParticles(GM::Vector3::ZeroVector, DeltaTime);
				}

				// Update all the elements of the scene
				Update(DeltaTime);

				/****** Normally render the scene *****/
				// Clear the window 
				m_Window->Clear();

				// Start a scene
				Renderer::BeginScene(m_CameraController->GetCamera());

				for (unsigned int i = 0; i < m_Objects3D.size(); i++)
					Renderer::Submit(m_Objects3D[i]);

				// Calculate the shadow maps
				if (GX_ENABLE_SHADOWS)
					RenderShadowMap();

				// Draw the debug quad to show the depth map
				//RenderShadowDebugQuad();

				RenderSkybox();

				// Bind the shader and draw the objects
				m_Shader->Bind();
				m_ShadowBuffer->BindDepthMap(GX_ENGINE_SHADOW_MAP_TEXTURE_SLOT);
				ConfigureShaderForRendering(*m_Shader);

				RenderScene();

				m_ParticlesManager->RenderParticles();

				// End the scene
				Renderer::EndScene();
			}

			// Renders ImGUI
			RenderGui();

			//Update the mouse
			Mouse::GetMouse()->Update();

			//Poll events and swap buffers
			m_Window->OnUpdate();
		}
	}

	void Application::Update(float DeltaTime)
	{
		// Update the camera
		m_CameraController->Update(DeltaTime);
		
		// Update the lights
		for (unsigned int i = 0; i < m_Lights.size(); i++)
			m_Lights[i]->Update(DeltaTime);

		// Update the meshes
		for (unsigned int i = 0; i < m_Objects2D.size(); i++)
			m_Objects2D[i]->Update(DeltaTime);
		
		for (unsigned int i = 0; i < m_Objects3D.size(); i++)
			m_Objects3D[i]->Update(DeltaTime);

		for (unsigned int i = 0; i < m_Terrain.size(); i++)
			m_Terrain[i]->Update(DeltaTime);

		m_ParticlesManager->Update(DeltaTime);

		DayNightCycleCalculations(DeltaTime);

		m_CurrentSkybox->Update(DeltaTime);

		if (m_CameraController->GetCamera()->IsRenderStateDirty())
		{
			for (unsigned int i = 0; i < m_Shaders.size(); i++)
			{
				const Ref<Shader>& shader = m_Shaders.at(i);
				if (!shader)
				{
					m_Shaders.erase(m_Shaders.begin() + i);		// TODO : Fix Memory leak
					continue;
				}
				shader->Bind();
				shader->SetUniform3f("u_CameraPos", m_CameraController->GetCameraPosition());

				//TODO: Uniforms need to be set in the renderer
				shader->SetUniformMat4f("u_ProjectionView", m_CameraController->GetCamera()->GetProjectionViewMatrix());
			}

			// Update the terrain Material shader (TODO: Find a better way)
			for (unsigned int i = 0; i < m_Terrain.size(); i++)
			{
				const Ref<Shader>& shader = m_Terrain[i]->GetMaterial()->GetShader();
				shader->Bind();
				shader->SetUniform3f("u_CameraPos", m_CameraController->GetCameraPosition());
				shader->SetUniformMat4f("u_ProjectionView", m_CameraController->GetCamera()->GetProjectionViewMatrix());
			}

			// Set the state back to rendered
			m_CameraController->GetCamera()->SetRenderStateDirty(false);
		}
	}

	void Application::RenderShadowMap()
	{
		// Render the shadow maps
		m_DepthShader->Bind();
		m_DepthShader->SetUniformMat4f("u_LightSpaceMatrix", m_SunLight->GetShadowInfo()->LightViewProjMat);
		m_ShadowBuffer->Bind();
	
		m_Window->ClearDepthBuffer();

		RenderScene(true);

		m_ShadowBuffer->UnBind();
	}

	void Application::RenderSkybox()
	{
		// Render the sky box
		m_CurrentSkybox->Enable();
		Renderer::RenderIndexed(*m_CurrentSkybox->GetIBO());	// Change this to directly submit skybox
		m_CurrentSkybox->Disable();
	}

	void Application::RenderScene(bool IsShadowPhase)
	{
		
		if (IsShadowPhase)
		{
			Renderer::RenderDepth(*m_DepthShader);
		}
		else
		{
			Renderer::Render();
		}
		
		RenderTerrain(IsShadowPhase);
	}

	void Application::RenderTerrain(bool IsShadowPhase)
	{
		// TODO: Design a better API
		Ref<Shader> shader = m_DepthShader;

		for (unsigned int i = 0; i < m_Terrain.size(); i++)
		{
			Ref<Terrain> terrain = m_Terrain[i];

			// Configure the terrain shaders
			terrain->Enable();
			if (!IsShadowPhase)
			{
				shader = terrain->GetMaterial()->GetShader();
				ConfigureShaderForRendering(*shader);
			}

			shader->Bind();

			// Set the transformation matrix
			GM::Matrix4 Model = terrain->GetMesh()->GetModelMatrix();
			shader->SetUniformMat4f("u_Model", Model);

			// Render the Terrain
			Renderer::RenderIndexed(*terrain->GetMesh()->GetIBO());

			terrain->Disable();
		}
	}

	void Application::RenderGui()
	{
		GraphXGui::DetailsWindow(m_Objects3D[0]);
		if (m_SelectedObject3D != nullptr)
			GraphXGui::DetailsWindow(m_SelectedObject3D, "Selected Object");

		GraphXGui::LightProperties(m_Light);
		GraphXGui::CameraProperties(m_CameraController);
		GraphXGui::Models();
		if (m_Terrain.size() > 0 && m_Terrain[0] != nullptr)
		{
			GraphXGui::TerrainDetails(m_Terrain[0]);
		}
		GraphXGui::GlobalSettings(m_CurrentSkybox, m_EngineDayTime, m_SunLight->Intensity, GX_ENABLE_PARTICLE_EFFECTS);
		GraphXGui::Render();
	}

	void Application::RenderShadowDebugQuad()
	{
		static std::vector<Vertex2D> quadVertices = {
			{ Vector2(-0.5f, -0.5f), Vector2(0.0f, 0.0f) },	//0
			{ Vector2( 0.5f, -0.5f), Vector2(1.0f, 0.0f) },	//1
			{ Vector2( 0.5f,  0.5f), Vector2(1.0f, 1.0f) },	//2
			{ Vector2(-0.5f,  0.5f), Vector2(0.0f, 1.0f) }	//3
		};

		static std::vector<unsigned int> quadIndices = {
			0, 1, 2,
			0, 2, 3
		};

		static Shader shader("res/shaders/Basic.shader");
		static Ref<Material> DebugMat = CreateRef<Material>(shader);

		static Ref<Mesh2D> QuadMesh = CreateRef<Mesh2D>(GM::Vector3::ZeroVector, GM::Vector3::ZeroVector, GM::Vector2::UnitVector, quadVertices, quadIndices, DebugMat);

		DebugMat->Bind();
		m_ShadowBuffer->BindDepthMap(GX_ENGINE_SHADOW_MAP_TEXTURE_SLOT);
		shader.SetUniform1i("u_Tex", GX_ENGINE_SHADOW_MAP_TEXTURE_SLOT);
		Renderer::Submit(QuadMesh);
		shader.UnBind();
	}

	void Application::ConfigureShaderForRendering(Shader& shader)
	{
		shader.SetUniform1i("u_ShadowMap", GX_ENGINE_SHADOW_MAP_TEXTURE_SLOT);
		shader.SetUniform3f("u_LightPos", m_Light->Position);
		shader.SetUniform4f("u_LightColor", m_Light->Color);

		if(GX_ENABLE_SHADOWS)
			shader.SetUniformMat4f("u_LightSpaceMatrix", m_SunLight->GetShadowInfo()->LightViewProjMat);

		m_SunLight->Enable(shader, "u_LightSource");
	}

	void Application::OnEvent(Event& e)
	{
		// Send the event to all the layers (once the layer system is in place)
		m_CameraController->OnEvent(e);

		bool handled = false;
		EventDispatcher dispatcher(e);

		if (e.IsInCategory(GX_EVENT_CATEGORY_WINDOW))
		{
			// handle the window events
			if (!handled)
			{
				handled = dispatcher.Dispatch<WindowResizedEvent>(BIND_EVENT_FUNC(Application::OnWindowResize));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<WindowMovedEvent>(BIND_EVENT_FUNC(Application::OnWindowMoved));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<WindowFocusEvent>(BIND_EVENT_FUNC(Application::OnWindowFocus));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<WindowLostFocusEvent>(BIND_EVENT_FUNC(Application::OnWindowLostFocus));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUNC(Application::OnWindowClose));
			}
		}
		else if (e.IsInCategory(GX_EVENT_CATEGORY_KEYBOARD))
		{
			if (!handled)
			{
				handled = dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNC(Application::OnKeyPressed));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FUNC(Application::OnKeyReleased));
			}
		}
		else if (e.IsInCategory(GX_EVENT_CATEGORY_MOUSE))
		{
			if (!handled)
			{
				handled = dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNC(Application::OnMouseButtonPressed));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FUNC(Application::OnMouseButtonReleased));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNC(Application::OnMouseMoved));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FUNC(Application::OnMouseScrolled));
			}
		}
		else if (e.IsInCategory(GX_EVENT_CATEGORY_GUI))
		{
			if (!handled)
			{
				handled = dispatcher.Dispatch<AddTextureEvent>(BIND_EVENT_FUNC(Application::OnAddTexture));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<AddModelEvent>(BIND_EVENT_FUNC(Application::OnAddModel));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<CreateTerrainEvent>(BIND_EVENT_FUNC(Application::OnCreateTerrain));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<CameraFOVChangedEvent>(BIND_EVENT_FUNC(Application::OnCameraFOVChanged));
			}
			if (!handled)
			{
				handled = dispatcher.Dispatch<CameraProjectionModeChange>(BIND_EVENT_FUNC(Application::OnCameraProjectionModeChanged));
			}
		}
		
		// Raise an error if the event is not handled
		if (!handled)
		{
			GX_ENGINE_ERROR("Unhandled Event: \"{0}\" ", e);
		}
	}

	void Application::DayNightCycleCalculations(float DeltaTime)
	{
		// Convert the time into hours
		float EngineTime = Clock::GetClock()->GetEngineTime() / (60.0f * 60.0f);
		int Days = (int)(EngineTime / m_EngineDayTime);
		float DayTime = EngineTime - m_EngineDayTime * Days;
		float TimeOfDay = DayTime * 24.0f / m_EngineDayTime;

		if (TimeOfDay >= DayTime::GX_START && TimeOfDay < DayTime::GX_EARLY_MORNING)
		{
			m_CurrentSkybox->BlendFactor = 0.0f;
		}
		else if (TimeOfDay >= DayTime::GX_EARLY_MORNING && TimeOfDay < DayTime::GX_SUNRISE)
		{
			TimeOfDay -= DayTime::GX_EARLY_MORNING;
			m_CurrentSkybox->BlendFactor = 0.5f;
		}
		else if (TimeOfDay >= DayTime::GX_SUNRISE && TimeOfDay < DayTime::GX_MORNING)
		{
			TimeOfDay = DayTime::GX_MORNING - TimeOfDay;
			m_CurrentSkybox = m_DaySkybox;
			m_SunLight->Color = GM::Vector4(0.5f, 0.5f, 0.0f, 1.0f);
			m_CurrentSkybox->BlendFactor = 0.6f;
		}
		else if (TimeOfDay >= DayTime::GX_MORNING && TimeOfDay < DayTime::GX_AFTERNOON)
		{
			TimeOfDay = DayTime::GX_AFTERNOON - TimeOfDay;
			m_CurrentSkybox->BlendFactor = 0.0f;
		}
		else if (TimeOfDay >= DayTime::GX_AFTERNOON && TimeOfDay < DayTime::GX_EVENING)
		{
			TimeOfDay -= DayTime::GX_AFTERNOON;
			m_CurrentSkybox->BlendFactor = 0.3f;
		}
		else if (TimeOfDay >= DayTime::GX_EVENING && TimeOfDay < DayTime::GX_NIGHT)
		{
			TimeOfDay -= DayTime::GX_EVENING;
			m_CurrentSkybox->BlendFactor = 0.7f;
		}
		else
		{
			TimeOfDay = DayTime::GX_NIGHT - TimeOfDay;
			m_CurrentSkybox = m_NightSkybox;
			m_SunLight->Color = GM::Vector4::UnitVector;
			m_CurrentSkybox->BlendFactor = 0.2f;
		}

		if (TimeOfDay > 0.0f)
		{
			m_CurrentSkybox->BlendFactor *= (TimeOfDay / 3.0f);
		}

		float angle = DeltaTime * 25.0f / (m_EngineDayTime * 10.0f);
		GM::Rotation rotation(angle, GM::Vector3::YAxis);
		m_SunLight->Direction = GM::Vector3(rotation * GM::Vector4(m_SunLight->Direction, 1.0f));
	}

#pragma region eventHandlers

	bool Application::OnWindowResize(WindowResizedEvent& e)
	{
		// If either of the width or height is zero, it means the window is minimised
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_IsMinimised = true;
			return true;
		}

		m_IsMinimised = false;

		// Set the new size of the window
		m_Window->OnResize();
		
		return true;
	}

	bool Application::OnWindowMoved(WindowMovedEvent& e)
	{
		// Stuff to be added later
		return true;
	}

	bool Application::OnWindowFocus(WindowFocusEvent& e)
	{
		// Stuff to be added later
		return true;
	}

	bool Application::OnWindowLostFocus(WindowLostFocusEvent& e)
	{
		// Stuff to be added later
		return true;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_IsRunning = false;
		return true;
	}

	bool Application::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		Mouse::GetMouse()->OnEvent(e);
		return true;
	}

	bool Application::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		Mouse::GetMouse()->OnEvent(e);
		return true;
	}

	bool Application::OnMouseMoved(MouseMovedEvent& e)
	{
		Mouse::GetMouse()->OnEvent(e);
		return true;
	}

	bool Application::OnMouseScrolled(MouseScrolledEvent& e)
	{
		Mouse::GetMouse()->OnEvent(e);
		return true;
	}

	bool Application::OnKeyPressed(KeyPressedEvent& e)
	{
		Keyboard::GetKeyboard()->OnEvent(e);
		return true;
	}

	bool Application::OnKeyReleased(KeyReleasedEvent& e)
	{
		Keyboard::GetKeyboard()->OnEvent(e);
		return true;
	}

	bool Application::OnAddTexture(AddTextureEvent& e)
	{
		FileOpenDialog dialog(ResourceType::TEXTURES);
		dialog.Show();

		std::string TexName = EngineUtil::ToByteString(dialog.GetAbsolutePath());
		Ref<Texture> texture = CreateRef<Texture>(TexName);

		if (m_SelectedObject3D)
		{
			Ref<Material> SelectedMat = m_SelectedObject3D->GetMaterial();
			SelectedMat->AddTexture(texture);
		}
		else if (m_SelectedObject2D)
		{
			Ref<Material> SelectedMat = m_SelectedObject2D->GetMaterial();
			SelectedMat->AddTexture(texture);
		}

		return true;
	}

	bool Application::OnAddModel(AddModelEvent& e)
	{
		if (e.GetModelType() == ModelType::CUBE)
		{
			m_Objects3D.emplace_back(new Cube(GM::Vector3::ZeroVector, GM::Vector3::ZeroVector, GM::Vector3::UnitVector, m_DefaultMaterial));
			m_SelectedObject3D = m_Objects3D[m_Objects3D.size() - 1];
		}
		else if (e.GetModelType() == ModelType::CUSTOM)
		{
			FileOpenDialog dialog(ResourceType::MODELS);
			dialog.Show();
			
			Ref<Model3D> model = CreateRef<Model3D>(EngineUtil::ToByteString(dialog.GetAbsolutePath()), m_DefaultMaterial);
			const Ref<std::vector<Ref<Mesh3D>>>& meshes = model->GetMeshes();
			
			for(unsigned int i = 0; i < meshes->size(); i++)
				m_Objects3D.emplace_back(meshes->at(i));

			m_SelectedObject3D = m_Objects3D[m_Objects3D.size() - 1];
		}
		// Add more model types once added

		m_SelectedObject3D->bShowDetails = true;
		return true;
	}

	bool Application::OnCameraFOVChanged(class CameraFOVChangedEvent& e)
	{
		CameraController* Controller = e.GetEntity().GetCameraController();
		if (Controller != nullptr)
		{
			Controller->SetFieldOfView(e.GetChangedFOV());
			return true;
		}

		GX_ENGINE_ERROR("Trying to change Camera FOV for a camera without a controller");
		return false;
	}

	bool Application::OnCameraProjectionModeChanged(class CameraProjectionModeChange& e)
	{
		CameraController* Controller = e.GetEntity().GetCameraController();
		if (Controller != nullptr)
		{
			Controller->SetProjectionMode(e.GetNewProjectionMode());
		}

		GX_ENGINE_ERROR("Trying to change Projection for a camera without a controller");
		return false;
	}

	bool Application::OnCreateTerrain(CreateTerrainEvent& e)
	{
		m_Terrain.push_back(e.GetTerrain());
		return true;
	}

#pragma endregion

	Application::~Application()
	{
		GX_ENGINE_INFO("Application: Closing Application.");

		Renderer::CleanUp();

		delete m_Window;
	}
}