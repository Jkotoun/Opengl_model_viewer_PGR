#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

//stb image for loading textures in files
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//3rd party libs
#include <SDL.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <assimp/Importer.hpp>W
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//project headers
#include "FreeLookCamera.h"
#include "OrbitCamera.h"
#include "model.h"


//globals
int gScreenWidth = 1280;
int gScreenHeight = 720;
bool lMouseDown = false;
bool rMouseDown = false;
bool gQuit = false;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
GLuint gPipelineProgram = 0;
enum cameraMode {FreeLook, Orbit}; 
cameraMode g_currentCameraMode = FreeLook;
SDL_bool gFreeLookMode = SDL_FALSE;
FreeLookCamera freeLookCamera(glm::vec3(0,0,10),1, 0.05);
OrbitCamera orbitCamera(glm::vec3(0, 0, 0),10, 0.05,0.05, 0.005);
glm::vec3 lightPosition(3.f, 3.0f, 0.5f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);




void getInfo() {
	std::cout << "Vendor:" << glGetString(GL_VENDOR) << "\n";
	std::cout << "Renderer:" << glGetString(GL_RENDERER) << "\n";
	std::cout << "Version:" << glGetString(GL_VERSION) << "\n";
	std::cout << "GLSL:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}


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

	gWindow = SDL_CreateWindow("OpenGL Window", gScreenWidth/4, gScreenHeight/4, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);
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

	std::string vsSrc = loadShaderFile("shaders/modelVS.glsl");
	std::string fsSrc = loadShaderFile("shaders/modelFS.glsl");
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
	
	
	if (g_currentCameraMode == FreeLook) {
		while (SDL_PollEvent(&event)) {

			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if(event.button.button == SDL_BUTTON_RIGHT){
					gFreeLookMode = SDL_TRUE;
				}
			}
			if (gFreeLookMode == SDL_TRUE) {
				if (event.type == SDL_MOUSEMOTION) {
					freeLookCamera.mouseLook(glm::vec2(-event.motion.xrel, -event.motion.yrel));
				}
			}
		}

		const Uint8* state = SDL_GetKeyboardState(NULL);
		if (gFreeLookMode == SDL_TRUE) {
			if (state[SDL_SCANCODE_W]) {
				freeLookCamera.MoveForward();
			}
			if (state[SDL_SCANCODE_S]) {
				freeLookCamera.MoveBackward();
			}
			if (state[SDL_SCANCODE_A]) {
				freeLookCamera.MoveLeft();
			}
			if (state[SDL_SCANCODE_D]) {
				freeLookCamera.MoveRight();
			}
			if (state[SDL_SCANCODE_ESCAPE]) {
				gFreeLookMode = SDL_FALSE;
			}
		}
		else {
		}
	}
	else if (g_currentCameraMode == Orbit) {
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);

			//scroll up
			if (event.type == SDL_MOUSEWHEEL) {
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
void Draw(Model &testModel) {


	glEnable(GL_DEPTH_TEST);
		


	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(0.15, 0.15, 0.15, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(gPipelineProgram);





	glm::mat4 modelMatrix = glm::mat4(1.f);
	//TODO volba kamery uniformní promìnnou

	glm::mat4 viewMatrix;
	if (g_currentCameraMode == FreeLook) {
		viewMatrix = freeLookCamera.getViewMatrix();
	}
	else if (g_currentCameraMode == Orbit) {
		viewMatrix = orbitCamera.getViewMatrix();
	}


	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.f), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 10000.f);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "modelMatrix"),1,GL_FALSE,&modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(gPipelineProgram, "lightPos"), 1, &lightPosition[0]);
	glUniform3fv(glGetUniformLocation(gPipelineProgram, "lightColor"), 1, &lightColor[0]);

	testModel.Draw(gPipelineProgram);
	
}


void MainLoop() {
	SDL_WarpMouseInWindow(gWindow, gScreenWidth / 2, gScreenHeight / 2);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;\
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(gWindow, gOpenGLContext);
	ImGui_ImplOpenGL3_Init("#version 460");
	
	//stbi_set_flip_vertically_on_load(true);

	//Model testModel("models/octavia_obj/model.obj");
	Model testModel("models/golfmk1_obj/model.obj");
	//Model testModel("models/golfmk5_gti/model.obj");
	//Model testModel("models/audia4/model.obj");




	while (!gQuit) {
		SDL_SetRelativeMouseMode(gFreeLookMode);

		HandleInput();
		Draw(testModel);
		//update window every frame


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(gWindow);

		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
			ImGui::SetNextWindowSize(
				ImVec2(350, 500),
				ImGuiCond_Always
			);
			ImGui::Begin("Settings");

			ImGui::Text("Camera mode");
			ImGui::RadioButton("Free Look", (int*)&g_currentCameraMode, FreeLook);
			ImGui::RadioButton("Orbit", (int*)&g_currentCameraMode, Orbit);

			ImGui::Text("Camera options");
			if (g_currentCameraMode == FreeLook) {
				ImGui::BulletText("Right click to enter free look mode");
				ImGui::BulletText("WASD to move");
				ImGui::BulletText("ESC to exit free look mode");
			}
			else if (g_currentCameraMode == Orbit) {
				ImGui::BulletText("Left click to rotate");
				ImGui::BulletText("Right click to pan");
				ImGui::BulletText("Scroll to zoom");
			}


			if (ImGui::Button("Reset Camera")) {
				if (g_currentCameraMode == FreeLook) {
					freeLookCamera.resetCamera();
				}
				else if (g_currentCameraMode == Orbit) {
					orbitCamera.resetCamera();
				}
			}
			ImGui::ColorEdit3("Light Color", &lightColor[0]);
			ImGui::SliderFloat3("Light Position", &lightPosition[0], -10.0f, 10.0f);
			ImGui::End();
		}

		

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(gWindow);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void CleanUp() {
	SDL_GL_DeleteContext(gOpenGLContext);
	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}




int main(int argc, char* argv[]) {
	InitProgram();
	//VertexSpecification();

	CreatePipelineProgram();

	MainLoop();
	
	CleanUp();
	return 0;
}


