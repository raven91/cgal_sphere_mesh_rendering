//
// Created by Nikita Kruk on 12.03.21.
//

#ifndef CGAL_SPHERE_MESH_RENDERING_RENDERER_HPP
#define CGAL_SPHERE_MESH_RENDERING_RENDERER_HPP

#include "ScreenshotHandler.hpp"

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <vector>
#include <fstream>

class Renderer
{
 public:

  Renderer();
  ~Renderer();

  void Start();

 private:

  ScreenshotHandler screenshot_handler_;

  std::vector<glm::vec3> sphere_vertices_;
  std::vector<glm::ivec3> sphere_faces_;
  std::vector<glm::vec3> sphere_normals_for_vertices_;
  std::vector<glm::vec3> plane_vertices_;
  std::vector<glm::ivec3> plane_faces_;

  std::ifstream file_for_nodes_;
  std::ifstream file_for_normals_;

  static GLfloat x_rot_;
  static GLfloat y_rot_;
  static GLfloat z_rot_;
  static glm::vec3 camera_pos_;
  static glm::vec3 camera_front_;
  static glm::vec3 camera_up_;
  static float delta_time_;  // Time between current frame and last frame
  static float last_frame_; // Time of last frame
  static float fov_;
  static int screenshot_count_;
  static bool stop_flag_;
  static bool pause_flag_;
  static bool take_screenshot_flag_;

  static glm::vec3 light_pos_;

  static void ErrorCallback(int error, const char *description);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  void InitShaders(GLuint shader_program[]);
  void SetShaderParameter(GLuint shader_program, GLfloat parameter_value, const std::string &parameter_name_in_shader);
  void SetShaderParameter(GLuint shader_program,
                          const glm::vec3 &parameter_value,
                          const std::string &parameter_name_in_shader);
  void FinFunc();
  void InitializeSphereDcm();
  void ReadNewSphereDcm();
  void CentralizeSphere();
  void ReadSphereOff();
  void ReadPlaneOff();
  void CreateTransformationMatrices(int width, int height, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
  void ImportTransformationMatrices(GLuint shader_program,
                                    const glm::mat4 &model,
                                    const glm::mat4 &view,
                                    const glm::mat4 &projection);
  void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint element_buffer[], GLuint shader_program[]);
  void RenderSphereMesh(GLuint vao, GLuint vbo, GLuint element_buffer, GLuint shader_program);
  void RenderSolidSphere(GLuint vao, GLuint vbo, GLuint element_buffer, GLuint shader_program);
  void RenderPlane(GLuint vao, GLuint vbo, GLuint element_buffer, GLuint shader_program);

  void ReadShaderSource(const std::string &fname, std::vector<char> &buffer);
  GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type);
  GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
  GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
                                 const std::string &geometry_shader_path,
                                 const std::string &fragment_shader_path);

};

#endif //CGAL_SPHERE_MESH_RENDERING_RENDERER_HPP
