#include <iostream>
#include <vector>
#include "GLM/glm.hpp"
#include "GLM/gtc/constants.hpp"
#include "GLM/gtx/rotate_vector.hpp"
#include "GLM/gtc/type_ptr.hpp"
#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

using std::cout;
using std::endl;
using glm::dvec3;

static struct {
	glm::dvec2 mouse_position;
	glm::dvec2 screen_dimensions = glm::dvec2(960,960);
} Globals;

/* GLFW Callback functions */
static void ErrorCallback(int error, const char* description) {
	std::cerr << "Error: " << description << endl;
}

static void CursorPositionCallback(GLFWwindow* window, double x, double y) {
	Globals.mouse_position.x = x / Globals.screen_dimensions.x;
	Globals.mouse_position.y = 1 - y / Globals.screen_dimensions.y;
}
static void WindowSizeCallback(GLFWwindow*window, int width, int height) {
	Globals.screen_dimensions.x = width;
	Globals.screen_dimensions.y = height;
	glViewport(0, 0, width, height);
}


glm::dvec2 ParametricHalfCircle(double t, double radius) {
	//t is 0 to 1
	t -= 0.5;//-0.5 to 0.5
	t *= glm::pi<double>();//-0.5pi to 0.5 pi
	return glm::dvec2(cos(t), sin(t))*radius;//for -1/2pi to 1/2pi
}
glm::dvec2 ParametricCircle(double t, glm::dvec2 center, double radius) {
	//t is 0 to 1
	t -= 0.5;
	t *= 2;
	t *= glm::pi<double>();//-pi to pi
	return glm::dvec2(cos(t), sin(t))*radius + center;//for -pi to pi
}
glm::dvec2 ParametricSpikes(double t, double radius, int a) {
	//t is 0 to 1
	t -= 0.5;//-0.5 to 0.5
	t *= glm::pi<double>();//-0.5pi to 0.5 pi
	return glm::dvec2((cos(t) + sin(a*t)/a), (sin(t)+cos(a*t)/a))*radius/2.;//for -1/2pi to 1/2pi
}
glm::dvec2 ParametricNewCurve(double t, double radius, int a) {
	//t is 0 to 1
	t *= glm::pi<double>();//-pi to pi
	return glm::dvec2(sin(t) * sin(t) * sin(t) * 16, 13 * cos(t) - 4 * cos(2 * t) - 2 * cos(3 * t) - cos(68 * t)) * 1. / 10.;//for -1/2pi to 1/2pi
}
int VRtoIndex(int v, int r, int vertical_segment, int rotation_segment) {
	return (r%rotation_segment) * vertical_segment + (v);
}
glm::dvec3 ParametricSphere(double t, double r, double radius) {
	auto p = glm::dvec3(ParametricHalfCircle(t, radius), 0);
	return glm::rotateY(p, r*glm::two_pi<double>());
};
glm::dvec3 ParametricTorus(double t, double r, glm::dvec2 center_of_circle, double radius) {
	auto p = glm::dvec3(ParametricCircle(t, center_of_circle, radius), 0);
	return glm::rotateY(p, r*glm::two_pi<double>());
};
glm::dvec3 ParametricSpikesSurface(double t, double r, double radius, int a) {
	auto p = glm::dvec3(ParametricSpikes(t, radius, a), 0);
	return glm::rotateY(p, r*glm::two_pi<double>());
};
glm::dvec3 ParametricNewShapeSurface(double t, double r, double radius, int a) {
	auto p = glm::dvec3(ParametricNewCurve(t, radius, a), 0);
	if (r < 0.3 || r>0.7)
		return glm::rotateY(p, r * glm::two_pi<double>());
	return glm::rotateY(p, r * glm::two_pi<double>());
};
/* OpenGL Utility Structs */
struct VAO {

	GLuint id;
	GLuint position_buffer;
	GLuint normals_buffer;
	GLuint element_array_buffer;
	GLsizei element_array_count;

	VAO(const std::vector<glm::vec3>& positions,const std::vector<glm::vec3>& normals, const std::vector<GLuint>& indices) {
		glGenVertexArrays(1, &id);
		glBindVertexArray(id);

		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*positions.size(), positions.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &normals_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, normals_buffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &element_array_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), indices.data(), GL_STATIC_DRAW);
		
		element_array_count = int(indices.size());
	}
};
VAO SphereVAO(const glm::dvec3& center, const double& radius, const int& vertical_segment, const int & rotation_segment) {
	//TODO fix radius
	std::vector<glm::vec3> positions;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			positions.push_back(ParametricSphere(v / double(vertical_segment - 1), r / double(rotation_segment),radius) + center);
		}
	}
	/*
	std::vector<glm::vec3> normals;
	for (const auto& position : positions)
		normals.push_back(glm::normalize(position));
		*/
	std::vector<glm::vec3> normals;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			auto nv = v / double(vertical_segment - 1);
			auto nr = r / double(rotation_segment);
			auto epsilon = glm::epsilon<double>();

			auto tangent_to_next_v = ParametricSphere(nv + epsilon, nr,radius) - ParametricSphere(nv, nr,radius);
			auto tangent_to_prev_v = -ParametricSphere(nv - epsilon, nr, radius) + ParametricSphere(nv, nr, radius);
			auto tangent_v = (tangent_to_next_v + tangent_to_prev_v) / 2.;

			auto tangent_to_next_r = ParametricSphere(nv, nr + epsilon,radius) - ParametricSphere(nv, nr,radius);
			auto tangent_to_prev_r = -ParametricSphere(nv, nr - epsilon, radius) + ParametricSphere(nv, nr, radius);
			auto tangent_r = (tangent_to_next_r + tangent_to_prev_r) / 2.;

			auto surface_normal = glm::normalize(glm::cross(tangent_r, tangent_v));
			normals.push_back(surface_normal);
		}
	}
	std::vector<GLuint> indices;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment - 1; ++v) {
			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r, vertical_segment, rotation_segment));

			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v + 1, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
		}
	}
	return VAO(positions, normals, indices);
}

VAO TorusVAO(const glm::dvec3& center, const double& radius,glm::dvec2 center_of_circle, const int& vertical_segment, const int & rotation_segment) {
	std::vector<glm::vec3> positions;
	for (int r = 0; r < rotation_segment*2; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			positions.push_back(ParametricTorus(v / double(vertical_segment - 1), r / double(rotation_segment),center_of_circle,radius) + center);
		}
	}
	std::vector<glm::vec3> normals;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			auto nv = v / double(vertical_segment - 1);
			auto nr = r / double(rotation_segment);
			auto epsilon = glm::epsilon<double>();//x+i i->0 from positive side, x+0.0000001

			auto tangent_to_next_v = ParametricTorus(nv + epsilon, nr, center_of_circle,radius) - ParametricTorus(nv, nr, center_of_circle,radius);
			auto tangent_to_prev_v = ParametricTorus(nv, nr, center_of_circle, radius) - ParametricTorus(nv - epsilon, nr, center_of_circle, radius);
			auto tangent_v = (tangent_to_next_v + tangent_to_prev_v) / 2.;

			auto tangent_to_next_r = ParametricTorus(nv, nr + epsilon, center_of_circle,radius) - ParametricTorus(nv, nr, center_of_circle,radius);
			auto tangent_to_prev_r = ParametricTorus(nv, nr, center_of_circle, radius) - ParametricTorus(nv, nr - epsilon, center_of_circle, radius);
			auto tangent_r = (tangent_to_next_r + tangent_to_prev_r) / 2.;

			auto surface_normal = glm::normalize(glm::cross(tangent_r, tangent_v));
			normals.push_back(surface_normal);
		}
	}
	std::vector<GLuint> indices;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment-1; ++v) {
			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r, vertical_segment, rotation_segment));

			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v + 1, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
		}
	}
	return VAO(positions, normals, indices);
}
VAO SpikesVAO(const glm::dvec3& center, const double& radius,int a, const int& vertical_segment, const int & rotation_segment) {
	//TODO fix radius
	std::vector<glm::vec3> positions;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			positions.push_back(ParametricSpikesSurface(v / double(vertical_segment - 1), r / double(rotation_segment), radius,a) + center);
		}
	}
	std::vector<glm::vec3> normals;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			auto nv = v / double(vertical_segment - 1);
			auto nr = r / double(rotation_segment);
			auto epsilon = glm::epsilon<double>();

			auto tangent_to_next_v = ParametricSpikesSurface(nv + epsilon, nr, radius, a) - ParametricSpikesSurface(nv, nr, radius, a);
			auto tangent_to_prev_v = -ParametricSpikesSurface(nv - epsilon, nr, radius, a) + ParametricSpikesSurface(nv, nr, radius, a);
			auto tangent_v = (tangent_to_next_v + tangent_to_prev_v) / 2.;

			auto tangent_to_next_r = ParametricSpikesSurface(nv, nr + epsilon, radius, a) - ParametricSpikesSurface(nv, nr, radius, a);
			auto tangent_to_prev_r = -ParametricSpikesSurface(nv, nr - epsilon, radius, a) + ParametricSpikesSurface(nv, nr, radius, a);
			auto tangent_r = (tangent_to_next_r + tangent_to_prev_r) / 2.;

			auto surface_normal = glm::normalize(glm::cross(tangent_r, tangent_v));
			normals.push_back(surface_normal);
		}
	}
	std::vector<GLuint> indices;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment - 1; ++v) {
			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r, vertical_segment, rotation_segment));

			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v + 1, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
		}
	}
	return VAO(positions, normals, indices);
}
VAO NewShapeVAO(const glm::dvec3& center, const double& radius, int a, const int& vertical_segment, const int& rotation_segment) {
	std::vector<glm::vec3> positions;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			positions.push_back(ParametricNewShapeSurface(v / double(vertical_segment - 1), r / double(rotation_segment), radius, a) + center);
		}
	}
	std::vector<glm::vec3> normals;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment; ++v) {
			auto nv = v / double(vertical_segment - 1);
			auto nr = r / double(rotation_segment);
			auto epsilon = glm::epsilon<double>();

			auto tangent_to_next_v = ParametricNewShapeSurface(nv + epsilon, nr, radius, a) - ParametricNewShapeSurface(nv, nr, radius, a);
			auto tangent_to_prev_v = -ParametricNewShapeSurface(nv - epsilon, nr, radius, a) + ParametricNewShapeSurface(nv, nr, radius, a);
			auto tangent_v = (tangent_to_next_v + tangent_to_prev_v) / 2.;

			auto tangent_to_next_r = ParametricNewShapeSurface(nv, nr + epsilon, radius, a) - ParametricNewShapeSurface(nv, nr, radius, a);
			auto tangent_to_prev_r = -ParametricNewShapeSurface(nv, nr - epsilon, radius, a) + ParametricNewShapeSurface(nv, nr, radius, a);
			auto tangent_r = (tangent_to_next_r + tangent_to_prev_r) / 2.;

			auto surface_normal = glm::normalize(glm::cross(tangent_r, tangent_v));
			normals.push_back(-surface_normal);//dunno why but normals were wrong, this fixes it.
		}
	}
	std::vector<GLuint> indices;
	for (int r = 0; r < rotation_segment; ++r) {
		for (int v = 0; v < vertical_segment - 1; ++v) {
			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r, vertical_segment, rotation_segment));

			indices.push_back(VRtoIndex(v + 1, r, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v + 1, r + 1, vertical_segment, rotation_segment));
			indices.push_back(VRtoIndex(v, r + 1, vertical_segment, rotation_segment));
		}
	}
	return VAO(positions, normals, indices);
}
/* OpenGL Utility Functions */
GLuint CreateShaderFromSource(GLenum shader_type, const GLchar * source) {
	GLuint shader = glCreateShader(shader_type);
	//python """ """, shader source is lambda
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int success;
	char info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		cout << "ERROR: Shader compilation failed." << endl;
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		cout << info_log << endl;
		glDeleteShader(shader);
		return NULL;
	}
	return shader;
}

GLuint CreateProgramFromSources(const GLchar* vertex_shader_source, const GLchar* fragment_shader_source) {
	GLuint program = glCreateProgram();

	//runs for each vertex
	GLuint vertex_shader = CreateShaderFromSource(GL_VERTEX_SHADER, vertex_shader_source);
	//runs for each pixel
	GLuint fragment_shader = CreateShaderFromSource(GL_FRAGMENT_SHADER, fragment_shader_source);

	int success;
	char info_log[512];
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		cout << "ERROR: Program linking failed" << endl;
		glGetProgramInfoLog(program, 512, NULL, info_log);
		cout << info_log << endl;
		glDeleteProgram(program);
		return NULL;
	}
	return program;
}

int main(void) {

	/* Set GLFW error callback */
	glfwSetErrorCallback(ErrorCallback);
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(int(Globals.screen_dimensions.x), int(Globals.screen_dimensions.y), "hasimsait", NULL, NULL);
	if (!window)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwSetWindowPos(window, 10, 50);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		glfwTerminate();
		return -1;
	}

	/* Set GLFW Callbacks*/
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	/* Configure OpenGL */
	glClearColor(0, 0, 0, 1);//transparent
	glEnable(GL_DEPTH_TEST);

	/* Creating OpenGL objects */
	
	VAO triangleVAO({
		{-0.5,0.2,0},
		{-0.9,-0.2,0},
		{-0.3,-0.2,0}
		},
		{
			{0,0,-1},
			{0,0,-1},
			{0,0,-1},
		},
		{
			0,1,2
		});
	VAO quadVAO({
		{0.2,-0.2,0},
		{0.8,-0.2,0},
		{ 0.8,0.4,0},
		{0.1,0.3,0},
		},
		{
			{0,0,-1}, 
			{0,0,-1}, 
			{0,0,-1},
			{0,0,-1},
		},
		{
		0,1,2,
		0,2,3,
		});

	VAO sphereVAO = SphereVAO(glm::dvec3(0, 0, 0.0), 0.3, 16, 16);
	VAO spikesVAO = SpikesVAO(glm::dvec3(0, 0, 0.0), 0.3,12, 16, 16);
	VAO torusVAO = TorusVAO(glm::dvec3(0, 0, 0),0.09,glm::dvec2(0.210,0),16, 16);
	VAO topacVAO = NewShapeVAO(glm::dvec3(0, 0, 0.0), 1, -10, 50, 50);
	//VAO spikesVAO = SpikesVAO();
	//center of circle is relative (transform the object to <center> after creatign the torus at origin)//
	//center of sphere+r=r_torus
	/* TODO watch from lab8 extra 1:12:24, an extra tangent method and an extra shape is there.*/
	
	GLuint program = CreateProgramFromSources(
		R"VERTEX(
		#version 330 core
		layout(location = 0) in vec3 a_position;
		layout(location = 1) in vec3 a_normal;

		uniform mat4 u_transform;
		out vec3 vertex_normal;
		out vec3 vertex_position;
		void main(){
			gl_Position=u_transform*vec4(a_position,1);
			vertex_normal=(u_transform*vec4(a_normal,0)).xyz;
			vertex_position=gl_Position.xyz;
		}
	)VERTEX",
		R"FRAGMENT(
		#version 330 core
		uniform vec2 u_mouse_position; 
		in vec3 vertex_normal;
		in vec3 vertex_position;
		out vec4 out_color;
		void main(){
			vec3 color= vec3(0);
			vec3 surface_color=vec3(0.5,0.5,0.5);
			vec3 surface_position=vertex_position;
			vec3 surface_normal= normalize(vertex_normal);

			float ambient_k = 1;
			vec3 ambient_color = vec3(0.5, 0.5, 0.5);
			color+= ambient_k* ambient_color*surface_color;

			vec3 static_light_direction=normalize(vec3(1,1,-1));//directional lighting
			vec3 static_light_color= vec3(0.4,0.4,0.4);
			float static_diffuse_k=1;
			float static_diffuse_intensity =max(0,dot(static_light_direction,surface_normal));
			color+=static_diffuse_k*static_diffuse_intensity*static_light_color*surface_color;
			
			vec3 view_dir= normalize(vec3(vertex_position.xy,-1)-surface_position);
			vec3 halfway_dir= normalize(view_dir+static_light_direction);
			float specular_k=1;
			float specular_intensity = pow(max(0,dot(halfway_dir,surface_normal)),64);
			color+=specular_k*specular_intensity*static_light_color; 
			

			//mouse point light
			/*
			vec3 light_position = vec3(u_mouse_position, -1);
			vec3 light_color= vec3(255,110,199)/255/2;
			vec3 to_light = normalize(light_position - vertex_position);//position light
			float x = dot(to_light,surface_normal);
			float diffuse_intensity = max(0,x);

			float diffuse_k=1;
			color+= light_color*diffuse_k*diffuse_intensity*surface_color;

			halfway_dir=normalize(view_dir+to_light);
			specular_k=1;
			specular_intensity = pow(max(0,dot(halfway_dir,surface_normal)),64);
			color+=specular_k*specular_intensity*light_color;
			//notice that it does not include the surface color
			*/

			out_color=vec4(color,1);
		}
	)FRAGMENT");

	glUseProgram(program);
	auto mouse_position_location = glGetUniformLocation(program, "u_mouse_position");
	auto transform_location = glGetUniformLocation(program, "u_transform");
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		auto mouse_position = glm::vec3(Globals.mouse_position*2.0 - 1.0, 0);//0,+1->-1,+1
		glUniform2fv(mouse_position_location, 1, glm::value_ptr(glm::vec2(mouse_position)));
		//translate rotate draw undo the translate -> center->rotate->translate
		//that's how it works (in reverse)
		
		
		for (int i = 0; i < 4; i++) {
			
			glm::mat4 transform(1.0);
			switch (i)
			{
			case 0:
				transform = glm::translate(transform, glm::vec3(-0.5, 0.5, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				transform = glm::rotate(transform, glm::radians(float(glfwGetTime()) * 10), glm::vec3(1, 1, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));

				glBindVertexArray(sphereVAO.id);
				glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);
				break;
			case 1:
				transform = glm::translate(transform, glm::vec3(-0.5, -0.5, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				transform = glm::rotate(transform, glm::radians(float(glfwGetTime()) * 10), glm::vec3(1, 1, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				glBindVertexArray(spikesVAO.id);
				glDrawElements(GL_TRIANGLES, spikesVAO.element_array_count, GL_UNSIGNED_INT, NULL);
				break;
			case 2:
				transform = glm::translate(transform, glm::vec3(0.5, 0.5, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				transform = glm::rotate(transform, glm::radians(float(glfwGetTime()) * 10), glm::vec3(1, 1, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				glBindVertexArray(torusVAO.id);
				glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);
				break;
			case 3:
				transform = glm::translate(transform, glm::vec3(0.5, -0.5, 0));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				transform = glm::rotate(transform, glm::radians(float(glfwGetTime()) * 10), glm::vec3(1, 1, 0));
				transform = glm::scale(transform, glm::vec3(0.25));
				glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				glBindVertexArray(topacVAO.id);
				glDrawElements(GL_TRIANGLES, topacVAO.element_array_count, GL_UNSIGNED_INT, NULL);
				break;
			default:
				break;
			}
			transform = glm::translate(transform, glm::vec3(0, 0, 0));
			glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(transform));
		}
		
		
		/*Move the objects back to their centers*/
		
		
		
		//glDrawArrays(GL_LINE_STRIP, 0, sphereVAO.element_array_count);
		
		/*Move the objects to center*/
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}