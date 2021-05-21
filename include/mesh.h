#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include "vertexArray.h"
#include "vertexBuffer.h"
#include "elementBuffer.h"
#include "shader.h"
#include "texture.h"

using namespace std;
using namespace glm;

namespace Lynx {

	enum MeshType {
		MESH_2D_SPRITE,
		MESH_3D,
		MESH_3D_NORMAL,
		MESH_3D_TEXTURED,
		MESH_3D_TEXTURED_NORMAL
	};

	enum RenderType {
		RENDER_ARRAYS,
		RENDER_INDEXED
	};

	class Mesh {

		public:
			Mesh(vector<Vertex>* vertices, vector<GLuint>* indices, MeshType type); // Element buffer mode
			Mesh(); // does nothing
			~Mesh();
			MeshType GetType();
			void Render();
			char* getError();
			bool checkErrors();
			vector<Vertex>* vertices;
			vector<GLuint>* indices;
			const char* name;
			

		protected:
			VertexArray* VAO;
			GLuint VBO;
			GLuint EBO;
			bool success = true;
			char* error;
			
			RenderType renderType;
			MeshType type;



	};

	class MeshRenderer : public Mesh {
		public:
			MeshRenderer(vector<Vertex>* vertices, vector<GLuint>* indices, Shader* shader, MeshType type); // 3D mesh using indices
			MeshRenderer(const char* path, Shader* shader, MeshType type); // model mode						 
			~MeshRenderer();
			void Render(glm::mat4 projection, glm::mat4 view);
			//void Render(void(*renderfunction)());
			Texture* texture = nullptr;
		protected:
			/* WORK IN PROGRESS
			* 
			void loadModel();
			void generateMesh();
			*/
			Shader* shader;
			mat4 model;
			MeshType type;
			
			
				
	};
}

#endif