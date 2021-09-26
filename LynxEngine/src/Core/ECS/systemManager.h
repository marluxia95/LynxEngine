#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <iostream>
#include <memory>
#include <unordered_map>
#include "entity.h"
#include "system.h"
#include "Core/scene.h"
#include "Core/assert.h"
#include "Core/logger.h"

namespace Lynx::ECS {
	
	class SystemManager {
	public:
		/*
			Registers a system
		*/
		template<class T>
		std::shared_ptr<T> RegisterSystem(){
			const char* typeName = typeid(T).name();
			log_debug("SystemManager : Registering system %s ", typeName);
			LYNX_ASSERT(systems.find(typeName) == systems.end(), "System already exists");

			auto system = std::make_shared<T>(scene);
			systems.insert({ typeName, system });
			return system;
		}	

		/*
			Sets a system's signature
		*/
		template<typename T>
		void SetSignature(Signature signature){
			const char* typeName = typeid(T).name();

			LYNX_ASSERT(systems.find(typeName) != systems.end(), "System used before registered" );

			signatures.insert({ typeName, signature });
		}

		/*
			Returns a system instance, if exists
		*/
		template<class T>
		std::shared_ptr<T> GetSystem(){
			const char* typeName = typeid(T).name();
			LYNX_ASSERT(systems.find(typeName) != systems.end(), "System does not exist!");
			return static_pointer_cast<T>(systems.at(typeName));
		}

		void EntityDestroyed(Entity entity){
			for (auto const& pair : systems) {
				auto const& system = pair.second;
				system->entities.erase(entity);
			}
		}


		void EntitySignatureChanged(Entity entity, Signature signature){
			log_debug("SystemManager : Entity %d changed signature ! Updating systems ...", entity);
			// Notify each system that an entity's signature changed
			for (auto const& pair : systems)
			{
				auto const& type = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = signatures[type];

				// Entity signature matches system signature - insert into set
				if ((signature & systemSignature) == systemSignature)
				{
					system->entities.insert(entity);
				}
				// Entity signature does not match system signature - erase from set
				else
				{
					system->entities.erase(entity);
				}
			}
		}

		/*
			Initializes all systems
		*/
		void InitSystems() {
			log_debug("SystemManager : Initializing systems...");
			for (auto const& system : systems) {
				system.second->Init();
				log_debug("System %s initialized", system.first);
			}
		}

		/*
			Updates all systems
		*/
		void UpdateSystems() {
			for (auto const& system : systems) {
				system.second->Update();
			}
		}

		void SetScene(Lynx::Scene* scene) {
			scene = scene;
			for (auto const& system : systems) {
				system.second->SetScene(scene);
			}
		}
	private:
		std::unordered_map<const char*, Signature> signatures = {};
		std::unordered_map<const char*, std::shared_ptr<System>> systems = {};
		Lynx::Scene* scene;
	};
}

#endif