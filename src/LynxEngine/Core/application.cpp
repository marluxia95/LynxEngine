#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "application.h"

#include "window_manager.h"
#include "input.h"

#include "event_manager.h"
#include "Events/event.h"

#include "module.h"

#include "logger.h"

#ifdef __linux__
#include <unistd.h>
#endif

#include <GLFW/glfw3.h>

namespace Lynx {

    Application* Application::s_applicationInstance = nullptr;

    Application::Application() 
    {
        log_debug("Initializing subsystems");


        thread_id = std::this_thread::get_id();

#ifdef LYNX_MULTITHREAD
        log_warn("Multithreading is enabled ! Keep in mind that this is still in progress and the application might not work as intended !");
        m_threadPool.reset(new ThreadPool(4));
#endif

        s_applicationInstance = this;

        m_resourceManager.reset(new ResourceManager());
        m_windowManager.reset(new WindowManager());
        

    }

    Application::~Application()
    {
        ModuleManager::UnloadAllModules();
#ifdef LYNX_MULTITHREAD
        m_threadPool->Wait();
#endif
    }

    /**
     * @brief Initializes the application instance
     * 
     * @param flags Not used
     */
    void Application::Initialise(int flags)
    {
        log_debug("Sending event init");
        EventManager::SendEvent(InitEvent());

        log_debug("Initializing subsystems");
        m_windowManager->Init();
        Input::Init();

        log_debug("Successfully initialized application");
    }

    void Application::SetRenderer(std::shared_ptr<Graphics::Renderer> renderer)
    {
        if(m_renderer && m_renderer != renderer) 
        {
            m_renderer->Shutdown();
        }

        m_renderer = renderer;
        m_renderer->Initialise();
    }

    /**
     * @brief Calculates the frametime
     * 
     */
    void Application::CalculateFrameTime()
    {
        float current_FrameTime = glfwGetTime();
        delta_time = current_FrameTime - last_FrameTime;
        last_FrameTime = current_FrameTime;
    }

    /**
     * @brief Runs the main loop
     * 
     */
    void Application::Run()
    {       
        EventManager::SendEvent(FirstTickEvent());
        do
        {
            CalculateFrameTime();
#ifdef LYNX_MULTITHREAD
            EventManager::UpdateListeners();
#endif
            
            // Update logic
            EventManager::SendEvent(RenderEvent());
            m_renderer->Render();
            EventManager::SendEvent(UpdateTickEvent());

            m_windowManager->Update();
        } while(!m_windowManager->ShouldClose());
        EventManager::SendEvent(LastTickEvent());
        m_windowManager->Destroy();
    }

    uint Application::GetResolutionHeight()
    {
        return m_windowManager->GetHeight();
    }

    uint Application::GetResolutionWidth()
    {
        return m_windowManager->GetWidth();
    }

}
