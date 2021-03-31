#include <stdio.h>
#include <iostream>
#include <vector>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "vertexArray.h"
#include "vertexBuffer.h"
#include "elementBuffer.h"
#include "model.h"
#include "mesh.h"


using namespace glm;
using namespace std;

namespace Lynx {

Mesh::Mesh(vector<Vertex>* vertices, vector<GLuint>* indices, MeshType type ) : vertices(vertices), indices(indices), type(type){


	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);  
	glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex), &(vertices->at(0)), GL_STATIC_DRAW);
	

	VAO = new VertexArray();
	VAO->Bind();
	

	// VAO Configuration

	switch( type ){

		case MESH_2D_SPRITE:
			// Vertex attribute
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    		glEnableVertexAttribArray(0);
			break;
		case MESH_3D:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			break;
		case MESH_3D_TEXTURED:
			// Vertex attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    		glEnableVertexAttribArray(0);
    		// Texture attribute
    		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));
    		glEnableVertexAttribArray(1);
			break;
		case MESH_3D_TEXTURED_NORMAL:
			// Vertex attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			// Texture attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));
			glEnableVertexAttribArray(1);
			// Normal attribute
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			glEnableVertexAttribArray(2);
			break;
	}
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), &(indices->at(0)), GL_STATIC_DRAW );
    VAO->Unbind();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 

}

Mesh::~Mesh(){

}

void Mesh::Render(){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
}


Mesh3D::Mesh3D(vector<Vertex>* vertices, vector<GLuint>* indices, Shader* shader, MeshType type)
	: Mesh(vertices, indices, type), shader(shader){
	if(type<MESH_3D){printf("Invalid mesh type\n"); return;}

}

Mesh3D::Mesh3D(Model model, Shader* shader, MeshType type)
	: Mesh(&model.vertices, &model.indices, type), shader(shader){
	if(type<MESH_3D){printf("Invalid mesh type\n"); return;}

}


Mesh3D::~Mesh3D(){
	shader->destroy();
}

void Mesh3D::Render(mat4 projection, mat4 view){
	VAO->Bind();
	model = mat4(1.0f);
	model = translate(model, this->pos);
	
	if(shader == nullptr) {printf("Shader is NULL!\n"); return;}
	// Set shader values
	shader->use();
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);
	shader->setMat4("model", model);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
}

}

