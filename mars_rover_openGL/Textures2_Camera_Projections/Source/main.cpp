#include <iostream>
#include <vector>

#define GLM_FORCE_LEFT_HANDED
#include "GLM/glm.hpp"
#include "GLM/gtc/constants.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtx/rotate_vector.hpp"
#include "GLM/gtx/string_cast.hpp"
#include "GLM/gtc/type_ptr.hpp"
#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "opengl_utilities.h"
#include "extras.h"
#define PI 3.14159265358979323846264338327950288
/* Keep the global state inside this struct */
static struct
{
	glm::dvec2 mouse_position;
	glm::ivec2 screen_dimensions = glm::ivec2(960, 960);
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;

	bool h = false;//cam left
	bool j = false;//cam down
	bool k = false;//cam up
	bool l = false;//cam right

	bool u = false;//cam look up
	bool i = false;//cam look down
	
	bool plus = false;//increase/decrease camera sensitivity
	bool minus = false;

	glm::vec3 eye = glm::vec3(0, 0, -10.2);
	glm::vec3 to = glm::vec3(0, 0, 0);
	float touches_surface= -10.02;//if the rovers are too high, lover this value
	//I already have motion sickness from this camera settings, I genuinely do not want tilt. Topdown good old racing.
	/*
	auto view = glm::lookAt(
			glm::vec3(0, 0, -10.2),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);
	*/

} Globals;

/* GLFW Callback functions */
static void ErrorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void CursorPositionCallback(GLFWwindow* window, double x, double y)
{
	Globals.mouse_position.x = x;
	Globals.mouse_position.y = y;
}

static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	Globals.screen_dimensions.x = width;
	Globals.screen_dimensions.y = height;

	glViewport(0, 0, width, height);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//std::cout << key <<" action: "<<action<< std::endl;

	switch (key)
	{
	case 68:
		Globals.d = true;
		if (action==0)
			Globals.d = false;
		break;
	case 65:
		Globals.a = true;
		if (action == 0)
			Globals.a = false;
		break;
	case 87:
		Globals.w = true;
		if (action == 0)
			Globals.w = false;
		break;
	case 83:
		Globals.s = true;
		if (action == 0)
			Globals.s = false;
		break;
	case 72:
		Globals.h = true;
		if (action == 0)
			Globals.h = false;
		break;
	case 74:
		Globals.j = true;
		if (action == 0)
			Globals.j = false;
		break;
	case 75:
		Globals.k = true;
		if (action == 0)
			Globals.k = false;
		break;
	case 76:
		Globals.l = true;
		if (action == 0)
			Globals.l = false;
		break;
	case 85:
		Globals.u = true;
		if (action == 0)
			Globals.u = false;
		break;
	case 73:
		Globals.i = true;
		if (action == 0)
			Globals.i = false;
		break;
	case 333:
		Globals.minus = true;
		if (action == 0)
			Globals.minus = false;
		break;
	case 334:
		Globals.plus = true;
		if (action == 0)
			Globals.plus = false;
		break;
	default:
		break;
	}

	
}

int main(void)
{
	/* Set GLFW error callback */
	glfwSetErrorCallback(ErrorCallback);

	/* Initialize the library */
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(
		Globals.screen_dimensions.x, Globals.screen_dimensions.y,
		"Hasim Sait Goktan", NULL, NULL
	);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Load OpenGL extensions with GLAD */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 10, 50);
	glfwSwapInterval(2);

	/* Set GLFW Callbacks */
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
	/* Configure OpenGL */
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Creating OpenGL objects */
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<GLuint> indices;
	GenerateParametricShapeFrom2D(positions, normals, uvs, indices, ParametricHalfCircle, 64, 32);
	VAO sphereVAO(positions, normals, uvs, indices);

	std::vector<glm::vec3> torus_positions;
	std::vector<glm::vec3> torus_normals;
	std::vector<glm::vec2> torus_uvs;
	std::vector<GLuint> torus_indices;
	GenerateParametricShapeFrom2D(torus_positions, torus_normals, torus_uvs, torus_indices, ParametricCircle, 32, 16);
	VAO torusVAO(torus_positions, torus_normals, torus_uvs, torus_indices);

	VAO cubeVAO(
		{
			{-0.3f, -0.5f, -0.5f},
			{0.3f, -0.5f, -0.5f},
			{0.3f, 0.5f, -0.5f},
			{0.3f, 0.5f, -0.5f},
			{-0.3f, 0.5f, -0.5f},
			{-0.3f, -0.5f, -0.5f},
			{-0.3f, -0.5f, 0.5f},
			{0.3f, -0.5f, 0.5f},
			{0.3f, 0.5f, 0.5f},
			{0.3f, 0.5f, 0.5f},
			{-0.3f, 0.5f, 0.5f},
			{-0.3f, -0.5f, 0.5f},
			{-0.3f, 0.5f, 0.5f},
			{-0.3f, 0.5f, -0.5f},
			{-0.3f, -0.5f, -0.5f},
			{-0.3f, -0.5f, -0.5f},
			{-0.3f, -0.5f, 0.5f},
			{-0.3f, 0.5f, 0.5f},
			{0.3f, 0.5f, 0.5f},
			{0.3f, 0.5f, -0.5f},
			{0.3f, -0.5f, -0.5f},
			{0.3f, -0.5f, -0.5f},
			{0.3f, -0.5f, 0.5f},
			{0.3f, 0.5f, 0.5f},
			{-0.3f, -0.5f, -0.5f},
			{0.3f, -0.5f, -0.5f},
			{0.3f, -0.5f, 0.5f},
			{0.3f, -0.5f, 0.5f},
			{-0.3f, -0.5f, 0.5f},
			{-0.3f, -0.5f, -0.5f},
			{-0.3f, 0.5f, -0.5f},
			{0.3f, 0.5f, -0.5f},
			{0.3f, 0.5f, 0.5f},
			{0.3f, 0.5f, 0.5f},
			{-0.3f, 0.5f, 0.5f},
			{-0.3f, 0.5f, -0.5f}
		},
		{
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{-1.0f, 0.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f}
		},
		{},
		{
			0, 1, 2,
			3, 4, 5,
			6, 7, 8,
			9, 10, 11,
			12, 13, 14,
			15, 16, 17,
			18, 19, 20,
			21, 22, 23,
			24, 25, 26,
			27, 28, 29,
			30, 31, 32,
			33, 34, 35
		}
		);

	stbi_set_flip_vertically_on_load(true);

	auto filename = "Assets/mars_1k_color.jpg";
	int x, y, n;
	unsigned char* texture_data = stbi_load(filename, &x, &y, &n, 0);
	if (texture_data == NULL)
	{
		std::cout << "Texture " << filename << " failed to load." << std::endl;
		std::cout << "Error: " << stbi_failure_reason() << std::endl;
	}
	else
	{
		std::cout << "Texture " << filename << " is loaded, X:" << x << " Y:" << y << " N:" << n << std::endl;
	}

	GLuint texture;
	glGenTextures(1, &texture);

	if (x * n % 4 != 0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x, y, 0, n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, texture_data
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);//either here or the sphere vao has an issue.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glGenerateMipmap(GL_TEXTURE_2D);

	if (x * n % 4 != 0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	stbi_image_free(texture_data);


	GLuint program = CreateProgramFromSources(
		R"VERTEX(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_projection_view;

out vec4 world_space_position;
out vec3 world_space_normal;
out vec2 vertex_uv;

void main()
{
	world_space_position = u_model * vec4(a_position, 1);
	world_space_normal = vec3(u_model * vec4(a_normal, 0));
	vertex_uv = a_uv;

	gl_Position = u_projection_view * world_space_position;
}
		)VERTEX",

		R"FRAGMENT(
#version 330 core

uniform vec2 u_mouse_position;
uniform sampler2D u_texture;
uniform vec3 u_surface_color;
uniform vec3 u_mars;

in vec4 world_space_position;
in vec3 world_space_normal;
in vec2 vertex_uv;

out vec4 out_color;

void main()
{
	vec3 color = vec3(0);

	vec3 surface_position = world_space_position.xyz;
	vec3 surface_normal = normalize(world_space_normal);
	vec2 surface_uv = vertex_uv;
	vec3 surface_color;
	
	if (u_mars == vec3(1)){
		 surface_color=  texture(u_texture, surface_uv).rgb;
	}
	if (u_mars == vec3(0)){
		surface_color = u_surface_color;
	}

	vec3 ambient_color = vec3(0.7);
	color += ambient_color * surface_color;

	vec3 light_direction = normalize(vec3(-1, -1, 1));
	vec3 to_light = -light_direction;

	vec3 light_color = vec3(0.3);

	float diffuse_intensity = max(0, dot(to_light, surface_normal));
	color += diffuse_intensity * light_color * surface_color;

	vec3 view_dir = vec3(0, 0, -1);	//	Because we are using an orthograpic projection, and because of the direction of the projection
	vec3 halfway_dir = normalize(view_dir + to_light);
	float shininess = 64;
	float specular_intensity = max(0, dot(halfway_dir, surface_normal));
	color += pow(specular_intensity, shininess) * light_color;

	out_color = vec4(color, 1);
}
		)FRAGMENT");
	if (program == NULL)
	{
		glfwTerminate();
		return -1;
	}
	glUseProgram(program);

	auto texture_location = glGetUniformLocation(program, "u_texture");
	glUniform1i(texture_location, 0);

	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture);

	auto mouse_location = glGetUniformLocation(program, "u_mouse_position");
	auto model_location = glGetUniformLocation(program, "u_model");
	auto projection_view_location = glGetUniformLocation(program, "u_projection_view");
	auto surface_color_location = glGetUniformLocation(program, "u_surface_color");
	auto mars_location = glGetUniformLocation(program, "u_mars");
	std::vector<glm::vec3> cube_positions{
			glm::vec3(0, 0, -10.01),
			glm::vec3(0, 0, -10.01),
			glm::vec3(-0.05, -0.05, -10.01),
	};
	auto mars_x_angle = glm::radians(0.f);
	auto mars_y_angle = glm::radians(0.f);

	auto rover_1_x_angle = glm::radians(50.f);
	auto rover_1_y_angle = glm::radians(50.f);
	auto rover_2_x_angle = glm::radians(0.f);
	auto rover_2_y_angle = glm::radians(0.f);

	auto eye_pos = glm::vec3(0, 0, -10.2);
	float sensitivity = 1;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Calculate mouse position
		auto mouse_position = Globals.mouse_position;
		mouse_position /= glm::dvec2(Globals.screen_dimensions);
		mouse_position.y = 1. - mouse_position.y;
		mouse_position = mouse_position * 2. - 1.;

		glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(mouse_position)));


		
		mouse_position *= sensitivity;
		if (Globals.h) {
			eye_pos.x -= 0.025;
		}
		else if (Globals.l) {
			eye_pos.x += 0.025;
		}
		if (Globals.k) {
			eye_pos.y += 0.025;
		}
		else if (Globals.j) {
			eye_pos.y -= 0.025;
		}
		if (Globals.u) {
			eye_pos.z -= 0.025;
		}
		else if (Globals.i) {
			eye_pos.z += 0.025;
		}
		if (Globals.plus) {
			sensitivity++;
			std::cout << "sensitivity is now " << sensitivity << std::endl;
		}
		else if (Globals.minus) {
			sensitivity--;
			std::cout << "sensitivity is now " << sensitivity << std::endl;
		}
		auto view = glm::lookAt(
			eye_pos,
			glm::vec3(mouse_position, 0),
			glm::vec3(0, 1, 0)
		);
		// Draw
		auto projection = glm::perspective(glm::radians(45.f), 1.f, 0.000001f, 100.f);

		glUniformMatrix4fv(projection_view_location, 1, GL_FALSE, glm::value_ptr(projection * view));
		const auto draw_cube = [&](glm::vec3 position,int index,glm::mat4 &mars_trans)
		{
			glBindVertexArray(cubeVAO.id);
			glm::mat4 rover_transform(1.0);
			rover_transform = glm::translate(rover_transform, position);
			rover_transform = glm::scale(rover_transform,glm::vec3(0.01f));

			if (index != 0) {
				rover_transform =  mars_trans* rover_transform;
			}

			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(rover_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
			glDrawElements(GL_TRIANGLES, cubeVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			glBindVertexArray(torusVAO.id);
			glm::mat4 wheel_transform(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(1.5, 1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			wheel_transform = glm::mat4(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(-1.5, 1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			wheel_transform = glm::mat4(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(1.5, -1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			wheel_transform = glm::mat4(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(-1.5, -1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);

		};


		
		glBindVertexArray(sphereVAO.id);
		auto mars = glm::vec3(1);


		auto position = glm::vec3(0, 0, 0);
		auto mars_transform = glm::mat4(1);

		mars_transform = glm::scale(mars_transform,glm::vec3(10.f));
		if (Globals.w) {
			mars_x_angle += glm::radians(1.f);
		}else if (Globals.s) {
			mars_x_angle -= glm::radians(1.f);
		}	
		if (Globals.a) {
			mars_y_angle -= glm::radians(1.f);
		}else if (Globals.d) {
			mars_y_angle += glm::radians(1.f);
		}
		
		mars_transform=glm::rotate(mars_transform, glm::radians(mars_x_angle), glm::vec3(-1, 0, 0));
		mars_transform = glm::rotate(mars_transform, glm::radians(mars_y_angle), glm::vec3(0, 1, 0));

		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(mars_transform));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(position * 0.5f + 0.5f));
		glUniform3fv(mars_location, 1, glm::value_ptr(mars));
		glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);
		
		mars = glm::vec3(0);
		int index = 0;
		for (const auto& position : cube_positions) {
			glUniform3fv(mars_location, 1, glm::value_ptr(mars));
			glm::mat4 mars_trans = mars_transform;
			//draw_cube(p,index,mars_trans);
			glBindVertexArray(cubeVAO.id);
			glm::mat4 rover_transform(1.0);
			//chasing_pos = glm::mix(mouse_position, chasing_pos, 0.99);

			if (index != 0) {
				
				if (index == 1) {
					rover_transform = glm::rotate(rover_transform, glm::radians(rover_1_x_angle), glm::vec3(-1, 0, 0));
					rover_transform = glm::rotate(rover_transform, glm::radians(rover_1_y_angle), glm::vec3(0, 1, 0));
					auto distance_x = mars_x_angle - rover_1_x_angle;
					if (distance_x == abs(distance_x)) {
						rover_1_x_angle += 0.5;
					}
					else {
						rover_1_x_angle -= 0.5;
					}
					auto distance_y = mars_y_angle - rover_1_y_angle;
					if (distance_y == abs(distance_y)) {
						rover_1_y_angle += 0.5;
					}
					else {
						rover_1_y_angle -= 0.5;
					}

					std::cout << rover_1_x_angle << std::endl;
					
				}else if (index == 2) {
					rover_transform = glm::rotate(rover_transform, glm::radians(mars_x_angle), glm::vec3(-1, 0, 0));
					rover_transform = glm::rotate(rover_transform, glm::radians(mars_y_angle), glm::vec3(0, 1, 0));
				}

			}
			rover_transform = glm::translate(rover_transform, position);
			rover_transform = glm::scale(rover_transform, glm::vec3(0.01f));

			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(rover_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
			glDrawElements(GL_TRIANGLES, cubeVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			glBindVertexArray(torusVAO.id);
			glm::mat4 wheel_transform(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(1.5, 1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			wheel_transform = glm::mat4(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(-1.5, 1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			wheel_transform = glm::mat4(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(1.5, -1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			wheel_transform = glm::mat4(1.0);
			wheel_transform = glm::scale(wheel_transform, glm::vec3(0.25));
			wheel_transform = glm::translate(wheel_transform, glm::vec3(-1.5, -1, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
			wheel_transform = glm::rotate(wheel_transform, glm::radians(float(glfwGetTime()) * 100), glm::vec3(0, -1, 0));

			wheel_transform = rover_transform * wheel_transform;
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(wheel_transform));
			glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);



			index++;
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
