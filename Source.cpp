#include <stdio.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

const GLchar* computeShaderPath = "./compute.shader",
* vertShaderPath = "./compute.vert",
* fragShaderPath = "./compute.frag";
GLuint vao;
GLuint texture_out;
int workgroups[3];
int compute_work_group_size[3];
GLuint computeProgram;
GLuint paletteBuffer;
GLuint drawProgram;
GLuint texture;

vec3 eye;
vec3 lookAt;
vec3 Up;
float fov = 90.;

vec3 cameraU;
vec3 cameraV;
vec3 w;
vec2 dim;
vec2 resolution;

void setupScene() {
	resolution = vec2(WindowWidth, WindowHeight);
	eye = vec3(0, 0, 2);
	lookAt = vec3(0, 0, 0);
	Up = vec3(0, 1, 0);
	float height = 2. * glm::tan(glm::radians(fov/2));

	dim = vec2(((float)resolution.x/(float)resolution.y) * height, height);
	w = glm::normalize(eye-lookAt);
	vec3 u = glm::normalize(glm::cross(Up,w));
	vec3 v = glm::normalize(glm::cross(w,u));
	cameraU = dim.x / 2 * u;
	cameraV = dim.y / 2 * v;
	//fprintf(stdout, "dim: %f,%f\n", dim.x, dim.y);
	//fprintf(stdout, "resolution: %f,%f\n", resolution.x, resolution.y);
	//fprintf(stdout, "w: %f,%f,%f\n", w.x, w.y, w.z);
	//fprintf(stdout, "cameraU: %f,%f,%f\n", cameraU.x, cameraU.y, cameraU.z);
	//fprintf(stdout, "cameraV: %f,%f,%f\n", cameraV.x, cameraV.y, cameraV.z);
}

void makeUniformVariables() {
	glUniform1i(glGetUniformLocation(computeProgram, "max_iter"),max_iter);
	glUniform3fv(glGetUniformLocation(computeProgram, "eye"), 1, &eye[0]);
	glUniform3fv(glGetUniformLocation(computeProgram, "cameraU"), 1, &cameraU[0]);
	glUniform3fv(glGetUniformLocation(computeProgram, "cameraV"), 1, &cameraV[0]);
	glUniform3fv(glGetUniformLocation(computeProgram, "w"), 1, &w[0]);
	glUniform2fv(glGetUniformLocation(computeProgram, "dim"), 1, &dim[0]);
	glUniform2fv(glGetUniformLocation(computeProgram, "resolution"), 1, &resolution[0]);
}

void compute() {
	glUseProgram(computeProgram);
	makeUniformVariables();
	glBindImageTexture(0, texture_out, 0, GL_FALSE, 0,
		GL_WRITE_ONLY, GL_RGBA32F);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1,
		paletteBuffer);
	glDispatchCompute(workgroups[0], workgroups[1], 1);
}

GLuint setImageStore() {
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WindowWidth, WindowHeight, 0,
		GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0); // unbind
	return texture;
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(drawProgram);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_out);
	// drawing call
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void setupBuffers() {
	vec3 RGB_Palette_3[] = { {0.03137255, 0.19635525, 0.43166474},
{0.03137255, 0.20853518, 0.4497501 },
{0.03137255, 0.21665513, 0.461807  },
{0.03137255, 0.22883506, 0.47989235},
{0.03137255, 0.23695502, 0.49194925},
{0.03137255, 0.24913495, 0.5100346 },
{0.03137255, 0.2572549 , 0.5220915 },
{0.03137255, 0.26943483, 0.54017686},
{0.03137255, 0.27755479, 0.55223376},
{0.03137255, 0.28973472, 0.57031911},
{0.03137255, 0.29785467, 0.58237601},
{0.03137255, 0.3100346 , 0.60046136},
{0.03175702, 0.31813918, 0.61214917},
{0.04098424, 0.32995002, 0.62137639},
{0.05021146, 0.34176086, 0.63060361},
{0.05636294, 0.34963476, 0.63675509},
{0.06559016, 0.3614456 , 0.64598231},
{0.07174164, 0.36931949, 0.65213379},
{0.08096886, 0.38113033, 0.66136101},
{0.08712034, 0.38900423, 0.6675125 },
{0.09634756, 0.40081507, 0.67673972},
{0.10249904, 0.40868897, 0.6828912 },
{0.11172626, 0.42049981, 0.69211842},
{0.11787774, 0.4283737 , 0.6982699 },
{0.12710496, 0.44018454, 0.70749712},
{0.13448674, 0.44821223, 0.7124183 },
{0.14666667, 0.46039216, 0.71869281},
{0.15478662, 0.46851211, 0.72287582},
{0.16696655, 0.48069204, 0.72915033},
{0.17914648, 0.49287197, 0.73542484},
{0.18726644, 0.50099193, 0.73960784},
{0.19944637, 0.51317186, 0.74588235},
{0.20756632, 0.52129181, 0.75006536},
{0.21974625, 0.53347174, 0.75633987},
{0.22786621, 0.5415917 , 0.76052288},
{0.24004614, 0.55377163, 0.76679739},
{0.24816609, 0.56189158, 0.77098039},
{0.26071511, 0.57384083, 0.77720877},
{0.27080354, 0.58073049, 0.78114571},
{0.28593618, 0.59106498, 0.78705113},
{0.29602461, 0.59795463, 0.79098808},
{0.31115725, 0.60828912, 0.7968935 },
{0.32124567, 0.61517878, 0.80083045},
{0.33637832, 0.62551326, 0.80673587},
{0.35151096, 0.63584775, 0.81264129},
{0.36159938, 0.64273741, 0.81657824},
{0.37673203, 0.6530719 , 0.82248366},
{0.38682045, 0.65996155, 0.82642061},
{0.40195309, 0.67029604, 0.83232603},
{0.41204152, 0.6771857 , 0.83626298},
{0.42901961, 0.68752018, 0.84124567},
{0.44156863, 0.69440984, 0.84395233},
{0.46039216, 0.70474433, 0.8480123 },
{0.47294118, 0.71163399, 0.85071895},
{0.49176471, 0.72196847, 0.85477893},
{0.50431373, 0.72885813, 0.85748558},
{0.52313725, 0.73919262, 0.86154556},
{0.54196078, 0.7495271 , 0.86560554},
{0.5545098 , 0.75641676, 0.86831219},
{0.57333333, 0.76675125, 0.87237216},
{0.58588235, 0.77364091, 0.87507882},
{0.60470588, 0.78397539, 0.87913879},
{0.6172549 , 0.79086505, 0.88184544},
{0.63252595, 0.79764706, 0.88687428},
{0.64236832, 0.80183007, 0.89031911},
{0.65713187, 0.80810458, 0.89548635},
{0.66697424, 0.81228758, 0.89893118},
{0.68173779, 0.81856209, 0.90409842},
{0.69158016, 0.8227451 , 0.90754325},
{0.70634371, 0.82901961, 0.9127105 },
{0.71618608, 0.83320261, 0.91615532},
{0.73094963, 0.83947712, 0.92132257},
{0.74571319, 0.84575163, 0.92648981},
{0.75555556, 0.84993464, 0.92993464},
{0.77031911, 0.85620915, 0.93510188},
{0.77868512, 0.86029988, 0.93799308},
{0.78754325, 0.86620531, 0.94094579},
{0.79344867, 0.87014225, 0.94291426},
{0.80230681, 0.87604767, 0.94586697},
{0.80821223, 0.87998462, 0.94783545},
{0.81707036, 0.88589004, 0.95078816},
{0.82297578, 0.88982699, 0.95275663},
{0.83183391, 0.89573241, 0.95570934},
{0.83773933, 0.89966936, 0.95767782},
{0.84659746, 0.90557478, 0.96063053},
{0.85250288, 0.90951173, 0.962599  },
{0.86136101, 0.91541715, 0.96555171},
{0.87021915, 0.92132257, 0.96850442},
{0.87635525, 0.92525952, 0.9704729 },
{0.88558247, 0.93116494, 0.97342561},
{0.89173395, 0.93510188, 0.97539408},
{0.90096117, 0.9410073 , 0.97834679},
{0.90711265, 0.94494425, 0.98031526},
{0.91633987, 0.95084967, 0.98326797},
{0.92249135, 0.95478662, 0.98523645},
{0.93171857, 0.96069204, 0.98818916},
{0.93787005, 0.96462899, 0.99015763},
{0.94709727, 0.97053441, 0.99311034},
{0.95324875, 0.97447136, 0.99507882},
{0.96247597, 0.98037678, 0.99803153} };

	glGenBuffers(1, &paletteBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, paletteBuffer);
	//Create data
	float* paletteData = new float[/*bufferSize*/ 400];
	for (int i = 0, index = 0; i < /*RGB_Palette_Size*/100; i++) {
		paletteData[index++] = RGB_Palette_3[i].x;
		paletteData[index++] = RGB_Palette_3[i].y;
		paletteData[index++] = RGB_Palette_3[i].z;
		paletteData[index++] = 1.;
		//store the data
		glBufferData(GL_SHADER_STORAGE_BUFFER, /*bufferSize*/ 400 * sizeof(float), paletteData, GL_STATIC_DRAW);
	}
}

GLuint setVertices() {
	// set vertices to vao and vbo for rendering quad
	float vertices[] = {
		// 5 float: 3 for viewport position followed by 2 for texture coords 
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
		-1.0f, 1.0f,  0.0f, 0.0f, 1.0f, // bottom right
		1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // top left
		1.0f,  1.0f,  0.0f, 1.0f, 1.0f  // top right
	};
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
		(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	return vao;
}

void checkShaderCompilation(GLuint shader, const char* shaderType) {
	// check the shader compilation
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == 0) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << shaderType << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void checkShaderProgram(GLuint program) {
	int success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER PROGRAM LINK_FAILED\n"
			<< infoLog << std::endl;
	}
}

GLuint loadShader(const GLchar* shaderFilePath,
	const char* shaderType) {
	// load shader file from system
	GLuint shader;
	std::string stype(shaderType);
	if (stype == "FRAGMENT") {
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	}
	else if (stype == "VERTEX") {
		shader = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (stype == "COMPUTE") {
		shader = glCreateShader(GL_COMPUTE_SHADER);
	}
	else {
		std::cout << "Unknown shader type:\n" << shaderType << std::endl;
	}
	std::ifstream shdrFileStream;
	shdrFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	std::string shaderCodeStr;
	try {
		shdrFileStream.open(shaderFilePath);
		std::stringstream shaderSStream;
		shaderSStream << shdrFileStream.rdbuf();
		shdrFileStream.close();
		shaderCodeStr = shaderSStream.str();
	}
	catch (std::ifstream::failure e) {
		//
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* shaderCode = shaderCodeStr.c_str();
	// lets source the shader
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	// a sanity check for unsuccessful compilations
	checkShaderCompilation(shader, shaderType);
	return shader;
}

void setupDrawProgram()
{
	fprintf(stdout, "Setting up the draw Program\n");
	vao = setVertices();
	texture_out = setImageStore();
	GLuint vertShader = loadShader(vertShaderPath, "VERTEX");
	GLuint fragShader = loadShader(fragShaderPath, "FRAGMENT");
	drawProgram = glCreateProgram();
	glAttachShader(drawProgram, vertShader);
	glAttachShader(drawProgram, fragShader);
	glLinkProgram(drawProgram);
	checkShaderProgram(drawProgram);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	fprintf(stdout, "Draw program set up complete\n");
}

void setupComputeProgram()
{
	fprintf(stdout, "Setting up the Compute Program\n");
	computeProgram = glCreateProgram();
	GLuint cshader = loadShader(computeShaderPath, "COMPUTE");
	glAttachShader(computeProgram, cshader);
	glLinkProgram(computeProgram);
	checkShaderProgram(computeProgram);
	glDeleteShader(cshader);
	fprintf(stdout, "Compute program set up complete\n");
	glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE,
		compute_work_group_size);
	fprintf(stdout, "Locsl Work Group size: %d, %d, %d\n", compute_work_group_size[0], compute_work_group_size[1],
compute_work_group_size[2]);
	workgroups[0] = (WindowWidth + compute_work_group_size[0] - 1) /
		compute_work_group_size[0];
	workgroups[1] = (WindowHeight + compute_work_group_size[1] - 1) /
		compute_work_group_size[1];
	workgroups[2] = 1;
	fprintf(stdout, "Compute Work GroupS : % d, % d, % d\n", workgroups[0],
		workgroups[1], workgroups[2]);
	setupBuffers();
	setupScene();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, paletteBuffer);
}