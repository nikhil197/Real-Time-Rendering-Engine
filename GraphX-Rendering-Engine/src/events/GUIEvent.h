#pragma once

#include "Event.h"
#include "Model/ModelTypes.h"

namespace engine
{
	class AddTextureEvent
		: public Event
	{
	public:
		AddTextureEvent() {}

		EVENT_CLASS_TYPE(GX_ADD_TEXTURE)
		EVENT_CLASS_CATEGORY(GX_EVENT_CATEGORY_GUI)
	};

	class AddModelEvent
		: public Event
	{
	private:
		/* Model Type to be added */
		ModelType m_ModelType;

	public:
		AddModelEvent(ModelType type)
			: Event(), m_ModelType(type)
		{}

		/* Returns the type of the model */
		inline ModelType GetModelType() const { return m_ModelType; }

		EVENT_CLASS_TYPE(GX_LOAD_MODEL)
		EVENT_CLASS_CATEGORY(GX_EVENT_CATEGORY_GUI)
	};

	class CreateTerrainEvent
		: public Event
	{
	private:
		/* Terrain Object */
		class Terrain* m_Terrain;
	public:
		CreateTerrainEvent(Terrain* terrain)
			: m_Terrain(terrain)
		{}

		/* Returns the terrain */
		inline class Terrain* GetTerrain() const { return m_Terrain; }

		EVENT_CLASS_TYPE(GX_CREATE_TERRAIN)
		EVENT_CLASS_CATEGORY(GX_EVENT_CATEGORY_GUI)
	};

	/* Entity changed events */
	class EntityChangedEvent
		: public Event
	{
	protected:
		class Entity& m_Entity;

	protected:
		EntityChangedEvent(Entity& e)
			: m_Entity(e)
		{}

		/* Returns the entity */
		virtual class Entity& GetEntity() const = 0;

		EVENT_CLASS_CATEGORY(GX_EVENT_CATEGORY_GUI | GX_EVENT_CATEGORY_ENTITY_CHANGE)
	};

	/* Camera Events */
	class CameraFOVChangedEvent
		: public EntityChangedEvent
	{
	private:
		float m_ChangedFOV;

	public:
		CameraFOVChangedEvent(Camera& cam, float FOV)
			: EntityChangedEvent(cam), m_ChangedFOV(FOV)
		{}

		float GetChangedFOV() const { return m_ChangedFOV; }

		virtual class Camera& GetEntity() const override { return *(static_cast<Camera*>(&m_Entity)); }

		EVENT_CLASS_TYPE(GX_FOV_CHANGED)
	};
}
