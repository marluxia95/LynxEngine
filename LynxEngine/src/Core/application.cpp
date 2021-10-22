#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "application.h"
#include "windowManager.h"
#include "inputManager.h"

#include "eventManager.h"
#include "Events/event.h"

#include "Graphics/rendererAPI.h"
#include "Graphics/mesh.h"

#include "Systems/physicsSystem.h"

#include "Systems/lightingSystem.h"
#include "Systems/parentingSystem.h"
#include "Systems/cameraSystem.h"
#include "Systems/renderSystem.h"

#include "ECS/systemManager.h"
#include "ECS/components.h"

#include "logger.h"

#ifdef __linux__
#include <unistd.h>
#endif

namespace Lynx {

    Application* Application::s_applicationInstance = nullptr;
    GameApplication* GameApplication::gameApplicationInstance = nullptr;

    Application::Application()
    {
        log_debug("Initializing subsystems");
#ifdef LYNX_MULTITHREAD
        log_warn("Multithreading is enabled ! Keep in mind that this is still in progress and the application might not work as intended !");
        m_threadPool = std::make_shared<ThreadPool>(3);
#endif
        m_systemManager = std::make_shared<ECS::SystemManager>();
        thread_id = std::this_thread::get_id();
        s_applicationInstance = this;

        EventManager::AddListener(SignatureChanged, [this](const Event& ev){
            const SignatureChangedEvent& event = static_cast<const SignatureChangedEvent&>(ev);
            m_systemManager->EntitySignatureChanged(event.entity, event.signature);
        });

        EventManager::AddListener(EntityDestroyed, [this](const Event& ev){
            const EntityDestroyedEvent& event = static_cast<const EntityDestroyedEvent&>(ev);
            m_systemManager->EntityDestroyed(event.entity);
        });
    }

    Application::~Application()
    {
        m_threadPool->Wait();
    }

    void Application::Init(const char* title, unsigned int width, unsigned int height, int flags)
    {
        log_debug("Sending event init");
        EventManager::SendEvent(InitEvent());

        log_debug("Initializing systems");
        m_systemManager->InitSystems();

        log_debug("Successfully initialized application");
    }

    void Application::CalculateFrameTime()
    {
        float current_FrameTime = glfwGetTime();
        delta_time = current_FrameTime - last_FrameTime;
        last_FrameTime = current_FrameTime;
    }

    // Main Loop

    void Application::Run()
    {
        do
        {
            CalculateFrameTime();
            if(applicationState == STATE_ACTIVE) {
                EventManager::SendEvent(UpdateTickEvent());

                // Update logic
                EventManager::SendEvent(RenderEvent());
                UpdateSystems();
            }

        } while(applicationState != STATE_CLOSED);
        EventManager::SendEvent(LastTickEvent());
    }

    GameApplication::GameApplication() : Application()
    {
        m_resourceManager = std::make_shared<ResourceManager>(m_threadPool.get());
        m_componentManager = std::make_unique<ECS::ComponentManager>();
        m_systemManager = std::make_unique<ECS::SystemManager>();
        gameApplicationInstance = this;
    }

    GameApplication::~GameApplication()
    {
        if(m_windowManager != nullptr)
            m_windowManager->Destroy();
    }

    /**
     * @brief Initializes the game application
     * 
     * @param title Window title
     * @param width Window width
     * @param height Window height
     * @param flags Window flags
     */
    void GameApplication::Init(const char* title, unsigned int width, unsigned int height, int flags)
    {
        m_windowManager = WindowManager::Create();
        if(flags & APPLICATION_FULLSCREEN)
            m_windowManager->Init(title, width, height, true);
        else
            m_windowManager->Init(title, width, height, false);

        log_debug("Initializing renderer API");
        Graphics::RendererAPI::Init();

        Input::Init();

        log_debug("Sending event init");
        EventManager::SendEvent(InitEvent());

        log_debug("Initializing systems");
        m_systemManager->InitSystems();

        log_debug("Successfully initialized application");
    }

    void GameApplication::LoadDefaultSystems()
    {
        log_debug("Loading default systems");
        RegisterSystem<CameraSystem>();
        {
            Signature signature;
            signature.set(GetComponentType<Transform>());
            signature.set(GetComponentType<Camera>());
            SetSystemSignature<CameraSystem>(signature);
        }

        RegisterSystem<PhysicsSystem>();
        {
            Signature signature;
            signature.set(GetComponentType<Transform>());
            signature.set(GetComponentType<RigidBody>());
            SetSystemSignature<PhysicsSystem>(signature);
        }

        RegisterSystem<LightingSystem>();
        {
            Signature signature;
            signature.set(GetComponentType<Transform>());
            signature.set(GetComponentType<PointLight>());
            SetSystemSignature<LightingSystem>(signature);
        }
    }

    unsigned int GameApplication::GetResolutionWidth()
    {
        return m_windowManager->window_width;
    }

    unsigned int GameApplication::GetResolutionHeight()
    {
        return m_windowManager->window_height;
    }

    void GameApplication::Run()
    {
        EventManager::SendEvent(FirstTickEvent());
        do
        {
            CalculateFrameTime();
            
            if(applicationState == STATE_ACTIVE) {
                EventManager::SendEvent(UpdateTickEvent());

                Graphics::RendererAPI::Clear(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

                // Update logic
                EventManager::SendEvent(RenderEvent());
                m_resourceManager->Update(delta_time);
                UpdateSystems();
            }

            m_windowManager->Update();
        } while(( !glfwWindowShouldClose(m_windowManager->window) ) | applicationState == STATE_CLOSED);
        EventManager::SendEvent(LastTickEvent());
    }

    Scene* GameApplication::CreateScene()
    {
        scene = new Scene(m_componentManager.get());
        m_systemManager->SetScene(scene);
        log_debug("Successfully created scene");
        return scene;
    }

    void GameApplication::LoadDefaultComponents()
    {
        log_debug("Loading default components");
        RegisterComponent<Transform>();
        RegisterComponent<RigidBody>();
        RegisterComponent<Generic>();
        RegisterComponent<MeshRenderer>();
        RegisterComponent<Camera>();
        RegisterComponent<Parent>();
        RegisterComponent<PointLight>();
        RegisterComponent<DirectionalLight>();
        RegisterComponent<LuaScript>();
    }

}