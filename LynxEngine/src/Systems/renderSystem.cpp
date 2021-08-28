#include <stdio.h>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Core/simpleGameEngine.h"
#include "Core/windowManager.h"
#include "Core/logger.h"

#include "Core/ECS/components.h"
#include "Systems/renderSystem.h"

using namespace glm;

extern Lynx::Application gApplication;

namespace Lynx {
    
    void RenderSystem::Init()
    {
        log_debug("Creating Main Camera");
        cameraEntity = gApplication.CreateEntity("Main camera");
        const auto& mCameraSystem = gApplication.GetSystem<CameraSystem>();
        
        gApplication.AddComponent(cameraEntity, Transform{
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0)
        });

        gApplication.AddComponent(cameraEntity, Camera{
            60, // Field of view
            vec2(gApplication.GetResolutionWidth(), gApplication.GetResolutionHeight()), // Resolution
            CAMERA_PERSPECTIVE, // Camera type
            true, // Is it a main camera ?
            vec3(0.0f,1.0f,0.0f)   // Up vector
        });

        mCameraSystem->CalculateProjections();

        directionalLight = gApplication.CreateEntity("Directional Light");

        gApplication.AddComponent(directionalLight, Transform{
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0)
        });

        gApplication.AddComponent(directionalLight, DirectionalLight{
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f),
            glm::vec3(1.0f),
            glm::vec3(0.8f),
            0.2f
        });

    }

    void RenderSystem::SetCubemap(Graphics::Cubemap* cubemap)
    {
        m_cubemap = cubemap;
    }

    void RenderSystem::Update()
    {
        const auto& mCameraComponent = gApplication.GetComponent<Camera>(cameraEntity);
        const auto& mCameraTransform = gApplication.GetComponent<Transform>(cameraEntity);
        const auto& mDirLightComponent = gApplication.GetComponent<DirectionalLight>(directionalLight);
        const auto& mLightingSystem = gApplication.GetSystem<LightingSystem>();

        for (auto const& entity : entities) {
            const auto& mTransform = gApplication.GetComponent<Transform>(entity);
            const auto& mRenderComponent = gApplication.GetComponent<MeshRenderer>(entity);

            if(mRenderComponent->mesh == nullptr){log_error("Render component not bind to a mesh"); continue;}

            mRenderComponent->shader->use();
            mat4 model = mTransform->GetModel();
            
            if(mRenderComponent->shader == NULL){log_error("Invalid shader for entity %d!", entity); return;}

            mRenderComponent->shader->setMat4("projection", mCameraComponent->projection);
            mRenderComponent->shader->setMat4("view", mCameraComponent->view);
            mRenderComponent->shader->setMat4("model", model);
            mRenderComponent->shader->setVec3("color", mRenderComponent->ambient);
            mRenderComponent->shader->setVec3("viewPos", mCameraTransform->position);

            //log_debug("\n--------------------------\n Render3D() :\nCamera nº%d \n Camera projection : %s\n Camera view : %s\n Camera position : %s\n Render Object : %d\n--------------------------", 
            //    cameraEntity,glm::to_string(mCameraComponent->projection).c_str(),glm::to_string(mCameraComponent->view).c_str(),glm::to_string(mCameraTransform->position).c_str(), entity);
            
			
            
			if(mRenderComponent->lighting | mLightingSystem->entities.size()){
				// Set lighting shader values
				int i = 0;
				for (auto lightEnt : mLightingSystem->entities){
					auto lightComponent = gApplication.GetComponent<PointLight>(lightEnt);
                    auto transform = gApplication.GetComponent<Transform>(lightEnt);
					char buffer[64];
                    
					sprintf(buffer, "pointLights[%d].position", i);
					mRenderComponent->shader->setVec3(buffer , transform->position);
					
					sprintf(buffer, "pointLights[%d].constant", i);
					mRenderComponent->shader->setFloat(buffer, lightComponent->constant);
					
					sprintf(buffer, "pointLights[%d].linear", i);
					mRenderComponent->shader->setFloat(buffer, lightComponent->linear);
					
					sprintf(buffer, "pointLights[%d].quadratic", i);
					mRenderComponent->shader->setFloat(buffer, lightComponent->quadratic);
					
					sprintf(buffer, "pointLights[%d].ambient", i);
					mRenderComponent->shader->setVec3(buffer, lightComponent->ambient);

                    mRenderComponent->shader->setVec3("material.ambient", mRenderComponent->ambient);
					mRenderComponent->shader->setVec3("material.diffuse", mRenderComponent->diffuse);
                    mRenderComponent->shader->setVec3("material.specular", mRenderComponent->specular);
                    mRenderComponent->shader->setFloat("material.shininess", mRenderComponent->shininess);
                    if(mRenderComponent->texture_diffuse != nullptr){
                        mRenderComponent->shader->setBool("diffuse_map", true);
                        mRenderComponent->shader->setInt("material.diffuse_tex", 0);
                        mRenderComponent->texture_diffuse->use();
                    }else{
                        mRenderComponent->shader->setBool("diffuse_map", false);
                    }

                    if(mRenderComponent->texture_specular != nullptr){
                        mRenderComponent->shader->setBool("specular_map", true);
                        mRenderComponent->shader->setInt("material.specular_tex", 0);
                        mRenderComponent->texture_specular->use();
                    }else{
                        mRenderComponent->shader->setBool("specular_map", false);
                    }

                    mRenderComponent->shader->setVec3("directionalLight.direction", mDirLightComponent->direction);
                    mRenderComponent->shader->setVec3("directionalLight.ambient", mDirLightComponent->ambient);
                    mRenderComponent->shader->setVec3("directionalLight.diffuse", mDirLightComponent->diffuse);
                    mRenderComponent->shader->setVec3("directionalLight.specular", mDirLightComponent->specular);
                    
                    mRenderComponent->shader->setFloat("directionalLight.intensity", mDirLightComponent->intensity);
                    

					i++;
				}
			}


            
            // Check if mesh has a texture, if so, render it
            if(mRenderComponent->texture != nullptr){    
                mRenderComponent->texture->use();
            }   

            mRenderComponent->mesh->VAO->Bind();
            mRenderComponent->mesh->Render();

            if(m_cubemap)
                m_cubemap->Use(mCameraComponent->projection, mCameraComponent->view);
        }

    }

}
