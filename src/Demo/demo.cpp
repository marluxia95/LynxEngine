#include "Core/entity_manager.h"
#include "Core/input.h"
#include "Core/event_manager.h"
#include "Events/keyEvent.h"
#include "Events/mouseEvent.h"
#include "Graphics/renderer_forward.h"
#include "Graphics/model.h"
#include "demo.h"

using namespace Lynx;

Demo::Demo(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
		if(std::string(argv[i]) == "--debug")
			log_set_level(LOG_DEBUG);
	}

    log_debug("test");

    Initialise(0);
    m_renderer.reset(new Graphics::ForwardRenderer());
    m_renderer->Initialise();

    m_camera = new Camera();
    m_camera->CalcPerspective(GetResolutionWidth(), GetResolutionHeight(), 0.1f, 1000.0f);
    
    m_renderer->SetCamera(m_camera);

    Entity* model;
    {
        Graphics::ModelLoader loader(m_entityManager);
        model = loader.LoadModel("res/models/cube.fbx");
    }

    auto texture = m_resourceManager->LoadTexture("res/textures/box.dds");
    Graphics::Material material(texture);

    model->GetChildByIndex(0)->GetRenderHndl()->SetMaterial(material);
    model->PrintHierarchy();

    EventManager::AddListener(Render, [this, model](const Event& ev){
        m_renderer->PushRender(model);
    });

    EventManager::AddListener(MouseKeyPressed, [this](const Event& ev){
        const MouseButtonEvent& button_event = static_cast<const MouseButtonEvent&>(ev);
        if(button_event.m_keyCode == MOUSE_BUTTON_2){
            mouse_active = button_event.m_action;
            Input::EnableCursor(mouse_active);
        }
        
    });

    EventManager::AddListener(UpdateTick, [this](const Event& ev){
        movement();
    });
    
    Run();
}

Demo::~Demo()
{
    
}

void Demo::movement()
{
    float speed = GetDeltaTime() * speed_mul;
    float forward = Input::IsKeyDown(KEY_W) - Input::IsKeyDown(KEY_S);
    float left = Input::IsKeyDown(KEY_D) - Input::IsKeyDown(KEY_A);

    if(forward)
        m_camera->position += speed * m_camera->rotation * forward;
    if(left)
        m_camera->position += speed * glm::normalize(glm::cross(m_camera->rotation, m_camera->Up()) * left);

    speed_mul = 3.0f + Input::IsKeyDown(KEY_LEFT_SHIFT) * 2.0f;

    if(!mouse_active) {
        return;
    }

    glm::vec2 pos = Lynx::Input::GetMousePos();
    glm::vec2 offset = glm::vec2(pos.x - prev_pos.x, prev_pos.y - pos.y);
    prev_pos = pos;

    offset *= sensitivity;

    pitch += offset.y;
    yaw += offset.x;

    if(pitch > 89.9f)
        pitch = 89.9f;
    
    if(pitch < -89.9f)
        pitch = -89.9f;

    //log_debug("%f %f (%f %f) (%f %f)", pitch, yaw, pos.x, pos.y, prev_pos.x, prev_pos.y);
    
    m_camera->rotation = glm::normalize(
        glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), 
        sin(glm::radians(pitch)), 
        sin(glm::radians(yaw)) * cos(glm::radians(pitch) )));
}
