#include <iostream>
#include <SDL.h>
#include <glad/glad.h>
#include <vector>
#include <string>
#include <fstream>
int gScreenWidth = 640;
int gScreenHeight = 480;
SDL_Window* gWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
bool gQuit = false;
GLuint gVAO = 0;
GLuint gVBO = 0;
GLuint gPipelineProgram = 0;
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
			-0.5,-0.5,0,0,1,0, //left bot vertex
			0.5,-0.5,0,0,0,1, //right bot vertex
			-0.5,0.5,0,1,0,0, //top left vertex

			//second
		 	0.5,-0.5,0,0,0,1, //right bot vertex	
			0.5,0.5,0,0,1,0, //top right vertex
			-0.5,0.5,0,1,0,0, //top left vertex
	};

	glCreateVertexArrays(1, &gVAO);

	glCreateBuffers(1, &gVBO);
	glNamedBufferData(gVBO, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glVertexArrayAttribBinding(gVAO, 0, 0);
	glEnableVertexArrayAttrib(gVAO, 0);
	glVertexArrayAttribFormat(gVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(gVAO, 0, gVBO, 0, 6 * sizeof(GLfloat));

	glVertexArrayAttribBinding(gVAO, 1, 0);
	glEnableVertexArrayAttrib(gVAO, 1);
	glVertexArrayAttribFormat(gVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));
	glVertexArrayVertexBuffer(gVAO, 1, gVBO, 0, 6 * sizeof(GLfloat));
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
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			std::cout << "SDL_QUIT" << std::endl;
			gQuit = true;
		}
	}
}

void Draw() {
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(1.f, 1.f, 0, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(gPipelineProgram);
	glBindVertexArray(gVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


void MainLoop() {
	while (!gQuit) {
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
