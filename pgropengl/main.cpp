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
//SDL and OpenGL context
int gScreenWidth = 1366;
int gScreenHeight = 768;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
GLuint gPipelineProgram = 0;

//input handling helpers
bool lMouseDown = false;
bool rMouseDown = false;
bool gQuit = false;

//cameras
enum cameraMode {FreeLook, Orbit}; 
cameraMode g_currentCameraMode = Orbit;
SDL_bool gFreeLookMode = SDL_FALSE;
FreeLookCamera freeLookCamera(glm::vec3(0,0,3),0.05, 0.05);
OrbitCamera orbitCamera(glm::vec3(0, 0, 0),3, 0.05,0.05, 0.005);

//projections
enum projectionMode {Perspective, Orthographic};
projectionMode g_currentProjectionMode = Perspective;

//models
enum modelsEnum {Octavia, GolfMk1, GolfMk5, AudiA4, MercedesV8};
modelsEnum g_currentModel = GolfMk1;

//other GUI globals
glm::vec3 lightPosition(3.f, 3.0f, 0.5f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);



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

void Init() {
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
	ImGuiIO& io = ImGui::GetIO();
	
	if (g_currentCameraMode == FreeLook) {
		while (SDL_PollEvent(&event)) {

			ImGui_ImplSDL2_ProcessEvent(&event);
			//if event in imgui, don't handle it by scene
			if (!io.WantCaptureMouse) {
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					if (event.button.button == SDL_BUTTON_RIGHT) {
						gFreeLookMode = SDL_TRUE;
					}
				}
				if (gFreeLookMode == SDL_TRUE) {
					if (event.type == SDL_MOUSEMOTION) {
						freeLookCamera.mouseLook(glm::vec2(-event.motion.xrel, -event.motion.yrel));
					}
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

			if (!io.WantCaptureMouse) {

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
		}
		const Uint8* state = SDL_GetKeyboardState(NULL);
	}
}
void Draw(Model &model, glm::mat4 &modelMatrix) {

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(0.85, 0.85, 0.85, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(gPipelineProgram);

	glm::mat4 viewMatrix;
	if (g_currentCameraMode == FreeLook) {
		viewMatrix = freeLookCamera.getViewMatrix();
	}
	else if (g_currentCameraMode == Orbit) {
		viewMatrix = orbitCamera.getViewMatrix();
	}

	glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(45.f), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 100.f);
	float aspectRatio = (float)gScreenWidth / (float)gScreenHeight;
	glm::mat4 orthographicMatrix = glm::ortho(-2.f, 2.f, -2.f / aspectRatio,2.f / aspectRatio, 0.1f, 100.f);
	glm::mat4 projectionMatrix = g_currentProjectionMode == Perspective ? perspectiveMatrix : orthographicMatrix;

	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "modelMatrix"),1,GL_FALSE,&modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(gPipelineProgram, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(gPipelineProgram, "lightPos"), 1, &lightPosition[0]);
	glUniform3fv(glGetUniformLocation(gPipelineProgram, "lightColor"), 1, &lightColor[0]);
	model.Draw(gPipelineProgram);
}

glm::mat4 computeModelMatrix(Model& model) {

	float minx = 0;
	float miny = 0;
	float minz = 0;
	float maxx = 0;
	float maxy = 0;
	float maxz = 0;
	for (const auto& mesh : model.opaqueMeshes) {
		for (const auto& vertex : mesh.vertices) {
			minx = std::min(minx, vertex.Position.x);
			miny = std::min(miny, vertex.Position.y);
			minz = std::min(minz, vertex.Position.z);
			maxx = std::max(maxx, vertex.Position.x);
			maxy = std::max(maxy, vertex.Position.y);
			maxz = std::max(maxz, vertex.Position.z);
		}
	}

	for (const auto& mesh : model.transparentMeshes) {
		for (const auto& vertex : mesh.vertices) {
			minx = std::min(minx, vertex.Position.x);
			miny = std::min(miny, vertex.Position.y);
			minz = std::min(minz, vertex.Position.z);
			maxx = std::max(maxx, vertex.Position.x);
			maxy = std::max(maxy, vertex.Position.y);
			maxz = std::max(maxz, vertex.Position.z);
		}
	}


	float extentX = maxx - minx;
	float extentY = maxy - miny;
	float extentZ = maxz - minz;


	float scaleFactor = 2.f/std::max(extentX, std::max(extentY, extentZ));

	float translateY = -(miny + maxy) / 2.f;
	float translateX = -(minx + maxx) / 2.f;
	float translateZ = -(minz + maxz) / 2.f;
	// Create scale matrix
	glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, translateY, translateZ));
	return modelMatrix;
}


void MainLoop() {
	SDL_WarpMouseInWindow(gWindow, gScreenWidth / 2, gScreenHeight / 2);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(gWindow, gOpenGLContext);
	ImGui_ImplOpenGL3_Init("#version 460");
	
	std::map<modelsEnum, std::string> modelPaths;
	std::map<modelsEnum, Model*> models;

	modelPaths.emplace(Octavia, "models/skoda_octavia/scene.gltf");
	modelPaths.emplace(GolfMk1, "models/golfmk1_obj/model.obj");
	modelPaths.emplace(GolfMk5, "models/golfmk5_gti/model.obj");
	modelPaths.emplace(AudiA4, "models/audia4/model.obj");
	modelPaths.emplace(MercedesV8, "models/mercedesv8/scene.gltf");

	models.emplace(GolfMk1, new Model(modelPaths.find(GolfMk1)->second));

	modelsEnum prevModel = g_currentModel;
	glm::mat4 modelMatrix = computeModelMatrix(*models.find(GolfMk1)->second);

	while (!gQuit) {
		SDL_SetRelativeMouseMode(gFreeLookMode);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(gWindow);

		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(1000, 10), ImGuiCond_Always);
			ImGui::SetNextWindowSize(
				ImVec2(350, 600),
				ImGuiCond_Always
			);
			ImGui::Begin("Settings");
			ImGui::Text("Model");
			ImGui::Combo("Model", (int*)&g_currentModel, "Skoda Octavia\0Volkswagen Golf Mk1\0Volkswagen Golf Mk5\0Audi A4\0Mercedes V8 Biturbo\0\0");

			if(prevModel != g_currentModel){
				prevModel = g_currentModel;
				//load model if not loaded
				if (models.find(g_currentModel) == models.end()) {
					ImGui::Text("Loading model...");
					ImGui::End();
					ImGui::Render();
					ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
					SDL_GL_SwapWindow(gWindow);
					models.emplace(g_currentModel, new Model(modelPaths.find(g_currentModel)->second));
					modelMatrix = computeModelMatrix(*models.find(g_currentModel)->second);
					if (g_currentModel == GolfMk5) {
						//golf mk5 is rotated 180 degrees
						modelMatrix = glm::rotate(modelMatrix, glm::radians(180.f), glm::vec3(0, 1, 0));
					}
					continue;
				}
				//recompute model matrix for selected model
				else {
					modelMatrix = computeModelMatrix(*models.find(g_currentModel)->second);
				}
			}

			ImGui::Text("Projection mode");
			ImGui::RadioButton("Perspective", (int*)&g_currentProjectionMode, Perspective);
			ImGui::RadioButton("Orthographic", (int*)&g_currentProjectionMode, Orthographic);

			ImGui::Text("Camera mode");
			ImGui::RadioButton("Orbit", (int*)&g_currentCameraMode, Orbit);

			if (g_currentProjectionMode == Perspective) {
				ImGui::RadioButton("Free Look", (int*)&g_currentCameraMode, FreeLook);
			}
			else {
				g_currentCameraMode = Orbit;
			}

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
			ImGui::SliderFloat3("Light Position", &lightPosition[0], -100.0f, 100.0f);
			ImGui::End();
		}

		HandleInput();
		Draw(*models.find(g_currentModel)->second, modelMatrix);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(gWindow);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

int main(int argc, char* argv[]) {
	//init SDL and OpenGL context
	Init();

	//load and compile shaders and create pipeline program
	CreatePipelineProgram();

	MainLoop();
	
	//cleanup
	SDL_GL_DeleteContext(gOpenGLContext);
	SDL_DestroyWindow(gWindow);
	SDL_Quit();
	return 0;
}


