#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stdio.h>
#include <unordered_map>
#include <queue>
#include <mutex>
#include "logger.h"
#include "threadpool.h"
#include "Graphics/mesh.h"
#include "Graphics/buffers.h"
#include "Graphics/shader.h"
#include "Graphics/texture.h"
#include "Graphics/cubemap.h"
#include "resource.h"
#include "lynx_common.h"

namespace Lynx {

	enum ResourceType {
		RES_SHADER,
		RES_TEXTURE,
		RES_MESH,
		RES_MODEL
	};

	class LYNXENGINE_API ResourceManager{
		public:
			ResourceManager(ThreadPool* pool);
			~ResourceManager();
			void Clear();

			void Update(float dt);

			std::shared_ptr<Graphics::Shader> LoadShader(const char* path);

			std::shared_ptr<Graphics::TextureBase> LoadTexture(const char* path);

			std::shared_ptr<Graphics::TextureBase> LoadCubemapTexture(std::vector<const char*>* textures);

			std::shared_ptr<Graphics::Mesh> LoadMesh(const char* name, std::vector<Graphics::Vertex>* vertices, std::vector<unsigned int>* indices, Graphics::MeshType type);

			template<typename T>
			std::shared_ptr<T> GetResource(std::string path)
			{
				return std::static_pointer_cast<T>(FindResourceByPath(path));
			}

			std::shared_ptr<ResourceBase> FindResourceByPath(std::string path);

		private:
			int lastId;
			
			std::map<long int, std::shared_ptr<ResourceBase>> resource_map = {};

			std::queue<Graphics::TextureBase*> texdata_queue;
			std::mutex queue_mutex;

			//static void async_loadCubemapTexData(void* data);
			const char* getFileName(const char* path);

			ThreadPool* thpool;
	
	};

}


#endif