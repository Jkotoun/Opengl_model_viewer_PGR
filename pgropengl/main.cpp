#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

//libs
#include <SDL.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>

//headers
#include "FirstPersonCamera.h"
#include "OrbitCamera.h"


//globals
int gScreenWidth = 1280;
int gScreenHeight = 720;
bool lMouseDown = false;
bool rMouseDown = false;
bool gQuit = false;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
GLuint gVAO = 0;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gPipelineProgram = 0;
enum cameraMode {FirstPerson, Orbit};
cameraMode g_currentCameraMode = Orbit;
SDL_bool gFirstPersonFloating = SDL_FALSE;
FirstPersonCamera firstPersonCamera(glm::vec3(0,0,3), 0.05, 0.05);
OrbitCamera orbitCamera(glm::vec3(0, 0, 0),3, 0.05,0.05, 0.005);
glm::vec3 lightPos(3.f, 3.0f, 0.5f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);




void getInfo() {
	std::cout << "Vendor:" << glGetString(GL_VENDOR) << "\n";
	std::cout << "Renderer:" << glGetString(GL_RENDERER) << "\n";
	std::cout << "Version:" << glGetString(GL_VERSION) << "\n";
	std::cout << "GLSL:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}
 
//errors checking, delete later
static void glClearErrors() {
	while (glGetError() != GL_NO_ERROR);
}	
static bool GLCheckErrorStatus(const char* functionName, const char* fileName, int line) {
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << ")" << functionName << " " << fileName << ":" << line << std::endl;
		return true;
	}
	return false;
}

#define GLCheck(x) glClearErrors();\
	x;\
	if(GLCheckErrorStatus(#x,__FILE__,__LINE__)) exit(1);



std::string loadShaderFile(const std::string& fileName) {
	std::ifstream file(fileName);
	if (!file.is_open()) {
		std::cout << "Failed to open file: " << fileName << std::endl;
		exit(1);
	}
	std::string line;
	std::string output;
	while (std::getline(file, line)) {
		output += line + "\n";
	}
	return output;
}

void InitProgram() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gWindow = SDL_CreateWindow("OpenGL Window", gScreenWidth/2, gScreenHeight/2, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);
	if (gWindow == nullptr) {
		std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		exit(1);
	}
	gOpenGLContext = SDL_GL_CreateContext(gWindow);
	if (gOpenGLContext == nullptr) {
		std::cout << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	//init GLAD
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(1);
	}

	getInfo();
}

void VertexSpecification() {
	//xyz position and rgb color
	std::vector<float> vertices = {
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // Bottom-left
		 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // Bottom-right
		 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,   // Top-right
		-0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // Top-left

		// Back face
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,  // Bottom-left
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  // Bottom-right
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,  // Top-right
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  // Top-left

		// Right face
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Bottom-back
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Bottom-front
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,   // Top-front
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Top-back

		 // Left face
		 -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // Bottom-back
		 -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // Bottom-front
		 -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,  // Top-front
		 -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // Top-back

		 // Top face
		 -0.5f, 0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,    // Back-left
		  0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,    // Back-right
		  0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,    // Front-right
		 -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,    // Front-left

		 // Bottom face
		 -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,   // Back-left
		  0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,   // Back-right
		  0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,   // Front-right
		 -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f    // Front-left
	};
	

    // Define the vertices of a cube

	std::vector<GLuint> indices {
		0, 1, 2, 2, 3, 0,   // Front face
		4, 5, 6, 6, 7, 4,   // Back face
		8, 9, 10, 10, 11, 8, // Right face
		12, 13, 14, 14, 15, 12, // Left face
		16, 17, 18, 18, 19, 16, // Top face
		20, 21, 22, 22, 23, 20  // Bottom face
	
	};


	glCreateVertexArrays(1, &gVAO);


	glCreateBuffers(1, &gIBO);
	glNamedBufferData(gIBO, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
	glVertexArrayElementBuffer(gVAO, gIBO);


	glCreateBuffers(1, &gVBO);
	glNamedBufferData(gVBO, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	

	glVertexArrayAttribBinding(gVAO, 0, 0);
	glEnableVertexArrayAttrib(gVAO, 0);
	glVertexArrayAttribFormat(gVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(gVAO, 0, gVBO, 0, 9 * sizeof(GLfloat));

	glVertexArrayAttribBinding(gVAO, 1, 0);
	glEnableVertexArrayAttrib(gVAO, 1);
	glVertexArrayAttribFormat(gVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));
	glVertexArrayVertexBuffer(gVAO, 1, gVBO, 0, 9 * sizeof(GLfloat));

	glVertexArrayAttribBinding(gVAO, 2, 0);
	glEnableVertexArrayAttrib(gVAO, 2);
	glVertexArrayAttribFormat(gVAO, 2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat));
	glVertexArrayVertexBuffer(gVAO, 2, gVBO, 0, 9 * sizeof(GLfloat));

}

GLuint CompileShader(GLuint type, const std::string& src) {
	GLuint shader;
	if (type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER) {
		shader = glCreateShader(type);
	}
	else {
		std::cout << "Invalid shader type" << std::endl;
		return 0;
	}
	const char* srcPtr = src.c_str();
	glShaderSource(shader, 1, &srcPtr, nullptr);
	glCompileShader(shader);
	return shader;
}



void CreatePipelineProgram() {

	std::string vsSrc = loadShaderFile("shaders/vertexShader.glsl");
	std::string fsSrc = loadShaderFile("shaders/fragmentShader.glsl");
	gPipelineProgram = glCreateProgram();
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);

	glAttachShader(gPipelineProgram, vs);
	glAttachShader(gPipelineProgram, fs);
	glLinkProgram(gPipelineProgram);

	glValidateProgram(gPipelineProgram);
	
}

void HandleInput() {
	SDL_Event event;

	static int mouseX = gScreenWidth / 2;
	static int mouseY = gScreenHeight / 2;
	
	
	if (g_currentCameraMode == FirstPerson) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if(event.button.button == SDL_BUTTON_RIGHT){
					std::cout<<"RIGHT BUTTON"<<std::endl;
					gFirstPersonFloating = SDL_TRUE;
				}
			}
			if (gFirstPersonFloating == SDL_TRUE) {
				if (event.type == SDL_MOUSEMOTION) {
					firstPersonCamera.mouseLook(glm::vec2(-event.motion.xrel, -event.motion.yrel));
				}
			}
		}

		const Uint8* state = SDL_GetKeyboardState(NULL);
		if (gFirstPersonFloating == SDL_TRUE) {
			if (state[SDL_SCANCODE_W]) {
				firstPersonCamera.MoveForward();
			}
			if (state[SDL_SCANCODE_S]) {
				firstPersonCamera.MoveBackward();
			}
			if (state[SDL_SCANCODE_A]) {
				firstPersonCamera.MoveLeft();
			}
			if (state[SDL_SCANCODE_D]) {
				firstPersonCamera.MoveRight();
			}
			if (state[SDL_SCANCODE_ESCAPE]) {
				gFirstPersonFloating = SDL_FALSE;
			}
		}
		else {
		}
	}
	else if (g_currentCameraMode == Orbit) {
		while (SDL_PollEvent(&event)) {

			//scroll up
			if (event.type == SDL_MOUSEWHEEL) {
				std::cout<<event.wheel.y<<std::endl;
				orbitCamera.zoom(event.wheel.y);
			}

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					lMouseDown = true;
				}
				if (event.button.button == SDL_BUTTON_RIGHT) {
					rMouseDown = true;
				}
			}

			if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					lMouseDown = false;
				}
				if (event.button.button == SDL_BUTTON_RIGHT) {
					rMouseDown = false;
				}
			}

			if (lMouseDown) {
				if (event.type == SDL_MOUSEMOTION) {
					orbitCamera.rotate(-event.motion.xrel, -event.motion.yrel);
				}
			}

			if (rMouseDown) {
				if (event.type == SDL_MOUSEMOTION) {
					orbitCamera.pan(-event.motion.xrel, event.motion.yrel);
				}
			}
		}
		const Uint8* state = SDL_GetKeyboardState(NULL);
	}
}

void Draw() {

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(0.16, 0.16, 0.16, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(gPipelineProgram);



	glm::mat4 modelMatrix = glm::mat4(1.f);

	//TODO volba kamery uniformní promìnnou

	glm::mat4 viewMatrix;
	if (g_currentCameraMode == FirstPerson) {
		viewMatrix = firstPersonCamera.getViewMatrix();
	}
	else if (g_currentCameraMode == Orbit) {
		viewMatrix = orbitCamera.getViewMatrix();
	}


	glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(45.f), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 100.f);

	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "modelMatrix"),1,GL_FALSE,&modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "projectionMatrix"), 1, GL_FALSE, &perspectiveMatrix[0][0]);

	glUniform3fv(glGetUniformLocation(gPipelineProgram, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(gPipelineProgram, "lightColor"), 1, &lightColor[0]);

	glBindVertexArray(gVAO);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


void MainLoop() {
	SDL_WarpMouseInWindow(gWindow, gScreenWidth / 2, gScreenHeight / 2);
	while (!gQuit) {
		SDL_SetRelativeMouseMode(gFirstPersonFloating);

		HandleInput();
		Draw();
		//update window every frame
		SDL_GL_SwapWindow(gWindow);
	}
}

void CleanUp() {
	SDL_GL_DeleteContext(gOpenGLContext);
	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}
int main(int argc, char* argv[]) {
	InitProgram();
	VertexSpecification();

	CreatePipelineProgram();

	MainLoop();
	
	CleanUp();
	return 0;
}


