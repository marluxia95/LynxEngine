#include <stdio.h>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "simpleGameEngine.h"
#include "components.h"
#include "renderSystem.h"
#include "logger.h"

extern Lynx::Core::Game game;

using namespace glm;

namespace Lynx {
    
    
    void RenderSystem::Init()
    {
        log_debug("Creating Main Camera");
        cameraEntity = game.CreateEntity("Main camera");

        game.AddComponent(cameraEntity, Transform{
            glm::vec3(0), 
            glm::quat(0,0,0,0), 
            glm::vec3(0)
        });

        game.AddComponent(cameraEntity, Camera{
            60, // Field of view
            vec2(game.WINDOW_WIDTH, game.WINDOW_HEIGHT), // Resolution
            CAMERA_PERSPECTIVE, // Camera type
            true, // Is it a main camera ?
            vec3(0.0f,0.0f,-1.0f), // Front vector
            vec3(0.0f,1.0f,0.0f)   // Up vector
        });

    }

    void RenderSystem::Update()
    {
        const auto& mCameraComponent = game.GetComponent<Camera>(cameraEntity);
        
        for (auto const& entity : entities) {
            const auto& mTransform = game.GetComponent<Transform>(entity);
            const auto& mRenderComponent = game.GetComponent<MeshRenderer>(entity);
            
            

            mRenderComponent->shader->use();

            mat4 model = mat4(1.0f);
            model = translate(model, mTransform->position);

            if(mRenderComponent->shader==NULL){log_error("Invalid shader for entity %d!", entity); return;}

            mRenderComponent->shader->setMat4("projection", mCameraComponent->projection);
            mRenderComponent->shader->setMat4("view", mCameraComponent->view);
            mRenderComponent->shader->setMat4("model", model);
            mRenderComponent->shader->setVec3("color", mRenderComponent->color);


            mRenderComponent->mesh->VAO->Bind();
            //log_info("Model : %s \n Projection : %s \n View : %s ", glm::to_string(model).c_str(), glm::to_string(mCameraComponent->projection).c_str(), glm::to_string(mCameraComponent->view).c_str());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRenderComponent->mesh->EBO);
            glDrawElements(GL_TRIANGLES, mRenderComponent->mesh->indices->size(), GL_UNSIGNED_INT, (void*)0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

    }

}