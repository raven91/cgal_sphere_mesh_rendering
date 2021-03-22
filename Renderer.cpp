//
// Created by Nikita Kruk on 12.03.21.
//

#include "Renderer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

GLfloat Renderer::x_rot_ = -0.f;
GLfloat Renderer::y_rot_ = 0.f;
GLfloat Renderer::z_rot_ = 0.f;
glm::vec3 Renderer::camera_pos_ = glm::vec3(0.0f, 0.0f, 3.5f * 1e-5f);
glm::vec3 Renderer::camera_front_ = glm::normalize(glm::vec3(0.0f, 0.0f, -2.0f));
glm::vec3 Renderer::camera_up_ = glm::vec3(0.0f, 1.0f, 0.0f);
float Renderer::delta_time_ = 0.0f;  // Time between current frame and last frame
float Renderer::last_frame_ = 0.0f; // Time of last frame
float Renderer::fov_ = 45.0f;
int Renderer::screenshot_count_ = 0;
bool Renderer::stop_flag_ = true;
bool Renderer::pause_flag_ = true;
bool Renderer::take_screenshot_flag_ = false;

glm::vec3 Renderer::light_pos_ = glm::vec3(0.0f, 10.0f, 5.0f);

Renderer::Renderer() :
    screenshot_handler_(),
    sphere_vertices_(),
    sphere_faces_(),
    plane_vertices_(),
    plane_faces_()
{
  std::cout << "Renderer created" << std::endl;
}

Renderer::~Renderer()
{
  std::cout << "Renderer destroyed" << std::endl;
}

void Renderer::Start()
{
  GLFWwindow *window;

  glfwSetErrorCallback(Renderer::ErrorCallback);

  if (!glfwInit())
  {
    std::cerr << "Initialization of GLFW failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

  window = glfwCreateWindow(1280 / 1, 1280 / 1, "Sphere Mesh", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    std::cerr << "Window opening failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, Renderer::KeyCallback);

  int major, minor, rev;
  major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
  minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
  rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "GLEW initialization failure" << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // contexts for sphere mesh, solid sphere, plane mesh
  GLuint vao[3] = {0};
  GLuint vbo[3] = {0};
  GLuint element_buffer[3] = {0};
  glGenVertexArrays(3, &vao[0]);
  glGenBuffers(3, &vbo[0]);
  glGenBuffers(3, &element_buffer[0]);

  GLuint shader_program[3] = {0};
  InitShaders(shader_program);

  InitializeSphereDcm();
//  ReadSphereOff();
//  ReadPlaneOff();
  while (!glfwWindowShouldClose(window))
  {
    DisplayFunc(window, vao, vbo, element_buffer, shader_program);

    if (take_screenshot_flag_)
    {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      screenshot_handler_.TakeScreenshotPng(width, height, screenshot_count_++);
      take_screenshot_flag_ = false;
    }
    /*if (screenshot_count_ > 1000)
    {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }*/

    ReadNewSphereDcm();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FinFunc();
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Renderer::ErrorCallback(int error, const char *description)
{
  std::cerr << description << std::endl;
}

void Renderer::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  float camera_speed = 2.0f * delta_time_ * 1e-5f; // adjust accordingly

  if (GLFW_PRESS == action)
  {
    switch (key)
    {
      case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE);
        break;

      case GLFW_KEY_R: stop_flag_ = !stop_flag_;
        break;

      case GLFW_KEY_P:
        if (stop_flag_)
        {
          pause_flag_ = !pause_flag_;
        }
        break;

      case GLFW_KEY_F13: take_screenshot_flag_ = true;
        break;

      case GLFW_KEY_LEFT : z_rot_ -= 0.1f;
        break;

      case GLFW_KEY_RIGHT : z_rot_ += 0.1f;
        break;

      case GLFW_KEY_UP : x_rot_ -= 0.1f;
        break;

      case GLFW_KEY_DOWN : x_rot_ += 0.1f;
        break;

      case GLFW_KEY_PAGE_UP : y_rot_ -= 0.1f;
        break;

      case GLFW_KEY_PAGE_DOWN : y_rot_ += 0.1f;
        break;

      case GLFW_KEY_W : camera_pos_ += camera_speed * camera_front_;
        break;

      case GLFW_KEY_S : camera_pos_ -= camera_speed * camera_front_;
        break;

      case GLFW_KEY_A : camera_pos_ += glm::normalize(glm::cross(camera_up_, camera_front_)) * camera_speed;
        break;

      case GLFW_KEY_D : camera_pos_ -= glm::normalize(glm::cross(camera_up_, camera_front_)) * camera_speed;
        break;

      case GLFW_KEY_Q : camera_pos_ += camera_up_ * camera_speed;
        break;

      case GLFW_KEY_E : camera_pos_ -= camera_up_ * camera_speed;
        break;

      default: break;
    }
  }
}

void Renderer::InitShaders(GLuint *shader_program)
{
  shader_program[0] = CreateProgramFromShader(
      std::string("../Shaders/sphere_mesh_vertex_shader.glsl"),
      std::string("../Shaders/sphere_mesh_fragment_shader.glsl"));

  shader_program[1] = CreateProgramFromShader(
      std::string("../Shaders/solid_sphere_vertex_shader.glsl"),
      std::string("../Shaders/solid_sphere_geometry_shader.glsl"),
      std::string("../Shaders/solid_sphere_fragment_shader.glsl"));

  shader_program[2] = CreateProgramFromShader(
      std::string("../Shaders/plane_vertex_shader.glsl"),
      std::string("../Shaders/plane_fragment_shader.glsl"));
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLfloat parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  glUseProgram(shader_program);
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  assert(-1 != location);
  glUniform1f(location, parameter_value);
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  const glm::vec3 &parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform3fv(location, 1, glm::value_ptr(parameter_value));
  }
}

void Renderer::FinFunc()
{

}

void Renderer::InitializeSphereDcm()
{
  std::string file_name_for_nodes("/Users/nikita/Documents/Projects/DeformableCellModel/nodes.txt");
  file_for_nodes_.open(file_name_for_nodes, std::ios::in);
  if (file_for_nodes_.good())
  {
    std::string line;
    std::getline(file_for_nodes_, line);
    std::istringstream buffer(line);
    double x, y, z;
    while (buffer >> x >> y >> z)
    {
      sphere_vertices_.emplace_back(x, y, z);
    }

    CentralizeSphere();
  }

  std::string file_name_for_normals("/Users/nikita/Documents/Projects/DeformableCellModel/normals.txt");
  file_for_normals_.open(file_name_for_normals, std::ios::in);
  if (file_for_normals_.good())
  {
    std::string line;
    std::getline(file_for_normals_, line);
    std::istringstream buffer(line);
    double n_x, n_y, n_z;
    while (buffer >> n_x >> n_y >> n_z)
    {
      sphere_normals_for_vertices_.emplace_back(n_x, n_y, n_z);
    }
  }

  std::string file_name_for_faces("/Users/nikita/Documents/Projects/DeformableCellModel/faces.txt");
  std::ifstream file_for_faces(file_name_for_faces, std::ios::in);
  if (file_for_faces.good())
  {
    int n_0, n_1, n_2;
    while (file_for_faces >> n_0 >> n_1 >> n_2)
    {
      sphere_faces_.emplace_back(n_0, n_1, n_2);
    }
    file_for_faces.close();
  }
}

void Renderer::ReadNewSphereDcm()
{
  if (!stop_flag_ || !pause_flag_)
  {
    if (file_for_nodes_.good())
    {
      std::string line;
      if (std::getline(file_for_nodes_, line))
      {
        std::istringstream buffer(line);
        double x, y, z;
        for (int i = 0; i < sphere_vertices_.size(); ++i)
        {
          buffer >> x >> y >> z;
          sphere_vertices_[i] = glm::vec3(x, y, z);
        } // i
//        std::cout << "scene " << ++screenshot_count_ << std::endl;
      } else
      {
        std::cout << "EOF reached" << std::endl;
      }

      CentralizeSphere();
    }

    if (file_for_normals_.good())
    {
      std::string line;
      if (std::getline(file_for_normals_, line))
      {
        std::istringstream buffer(line);
        double n_x, n_y, n_z;
        for (int i = 0; i < sphere_normals_for_vertices_.size(); ++i)
        {
          buffer >> n_x >> n_y >> n_z;
          sphere_normals_for_vertices_[i] = glm::vec3(n_x, n_y, n_z);
        } // i
      }
    }

    pause_flag_ = true;
  }
}

void Renderer::ReadSphereOff()
{
  std::string file_name("/Users/nikita/CLionProjects/cgal_sphere_mesh_generation/cmake-build-debug/sphere.off");
  std::ifstream file(file_name, std::ios::in);
  if (file.good())
  {
    std::string header;
    std::getline(file, header);

    std::string line;
    std::getline(file, line);
    std::istringstream line_buffer(line);
    int n_vertices, n_faces, n_edges;
    line_buffer >> n_vertices >> n_faces >> n_edges;

    sphere_vertices_ = std::vector<glm::vec3>(n_vertices);
    float x, y, z;
    for (int i = 0; i < n_vertices; ++i)
    {
      file >> x >> y >> z;
      sphere_vertices_[i] = glm::vec3(x, y, z);
    } // i

    sphere_faces_ = std::vector<glm::ivec3>(n_faces);
    int n_vertices_per_face, i_0, i_1, i_2;
    for (int i = 0; i < n_faces; ++i)
    {
      file >> n_vertices_per_face >> i_0 >> i_1 >> i_2;
      sphere_faces_[i] = glm::ivec3(i_0, i_1, i_2);
    } // i
  }
}

void Renderer::ReadPlaneOff()
{
  std::string file_name("/Users/nikita/CLionProjects/cgal_sphere_mesh_generation/cmake-build-debug/plane.off");
  std::ifstream file(file_name, std::ios::in);
  if (file.good())
  {
    std::string header;
    std::getline(file, header);

    std::string line;
    std::getline(file, line);
    std::istringstream line_buffer(line);
    int n_vertices, n_faces, n_edges;
    line_buffer >> n_vertices >> n_faces >> n_edges;

    plane_vertices_ = std::vector<glm::vec3>(n_vertices);
    float x, y, z;
    float norm = 1.0f, scaling = 1.0f, sphere_radius = 10e-6;
    for (int i = 0; i < n_vertices; ++i)
    {
      file >> x >> y >> z;
//      norm = std::hypot(x, y, z);
      scaling = sphere_radius / norm;
      plane_vertices_[i] = glm::vec3(x, y, z) * scaling;
    } // i

    plane_faces_ = std::vector<glm::ivec3>(n_faces);
    int n_vertices_per_face, i_0, i_1, i_2;
    for (int i = 0; i < n_faces; ++i)
    {
      file >> n_vertices_per_face >> i_0 >> i_1 >> i_2;
      plane_faces_[i] = glm::ivec3(i_0, i_1, i_2);
    } // i
  }
}

void Renderer::CentralizeSphere()
{
  glm::vec3 center_of_mass(0.0f, 0.0f, 0.0f);
  for (const glm::vec3 &vertex : sphere_vertices_)
  {
    center_of_mass += vertex;
  } // vertex
  center_of_mass /= sphere_vertices_.size();

  for (glm::vec3 &vertex : sphere_vertices_)
  {
    vertex -= center_of_mass;
  } // vertex
}

void Renderer::CreateTransformationMatrices(int width,
                                            int height,
                                            glm::mat4 &model,
                                            glm::mat4 &view,
                                            glm::mat4 &projection)
{
  projection = glm::perspective(glm::radians(fov_), (float) width / (float) height, 0.1f * 1e-5f, 100.0f * 1e-5f);

  view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

  model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.f, -0.f, -0.f));
  model = glm::rotate(glm::mat4(1.0f), z_rot_, glm::vec3(0.0f, 1.0f, 0.0f))
      * glm::rotate(glm::mat4(1.0f), y_rot_, glm::vec3(0.0f, 0.0f, -1.0f))
      * glm::rotate(glm::mat4(1.0f), x_rot_, glm::vec3(1.0f, 0.0f, 0.0f))
      * model;
}

void Renderer::ImportTransformationMatrices(GLuint shader_program,
                                            const glm::mat4 &model,
                                            const glm::mat4 &view,
                                            const glm::mat4 &projection)
{
  glUseProgram(shader_program);
  GLint projection_id = glGetUniformLocation(shader_program, "projection");
  if (-1 != projection_id)
  {
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, glm::value_ptr(projection));
  }
  GLint view_id = glGetUniformLocation(shader_program, "view");
  if (-1 != view_id)
  {
    glUniformMatrix4fv(view_id, 1, GL_FALSE, glm::value_ptr(view));
  }
  GLint model_id = glGetUniformLocation(shader_program, "model");
  if (-1 != model_id)
  {
    glUniformMatrix4fv(model_id, 1, GL_FALSE, glm::value_ptr(model));
  }
}

void Renderer::DisplayFunc(GLFWwindow *window, GLuint *vao, GLuint *vbo, GLuint *element_buffer, GLuint *shader_program)
{
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it is closer to the camera than the former one
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glEnable(GL_MULTISAMPLE); // MSAA

  float current_frame = glfwGetTime();
  delta_time_ = current_frame - last_frame_;
  last_frame_ = current_frame;

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glm::mat4 projection, view, model;
  CreateTransformationMatrices(width, height, model, view, projection);
  ImportTransformationMatrices(shader_program[0], model, view, projection);
  ImportTransformationMatrices(shader_program[1], model, view, projection);
//  ImportTransformationMatrices(shader_program[2], model, view, projection);

  // Render a sphere as a solid object
  glUseProgram(shader_program[1]);
  RenderSolidSphere(vao[1], vbo[1], element_buffer[1], shader_program[1]);

  // Render a sphere as a mesh
  glUseProgram(shader_program[0]);
  RenderSphereMesh(vao[0], vbo[0], element_buffer[0], shader_program[0]);

//  glUseProgram(shader_program[2]);
//  RenderPlane(vao[2], vbo[2], element_buffer[2], shader_program[2]);
}

void Renderer::RenderSphereMesh(GLuint vao, GLuint vbo, GLuint element_buffer, GLuint shader_program)
{
  std::vector<float> vertices_consecutive(sphere_vertices_.size() * 3, 0.0f);
  for (int i = 0; i < sphere_vertices_.size(); ++i)
  {
    vertices_consecutive[3 * i] = sphere_vertices_[i][0];
    vertices_consecutive[3 * i + 1] = sphere_vertices_[i][1];
    vertices_consecutive[3 * i + 2] = sphere_vertices_[i][2];
  } // i
  std::vector<unsigned int> indices;//(sphere_faces_.size() * 3);
  for (int i = 0; i < sphere_faces_.size(); ++i)
  {
    /*// to draw as GL_TRIANGLES
    indices.push_back(sphere_faces_[i][0]);
    indices.push_back(sphere_faces_[i][1]);
    indices.push_back(sphere_faces_[i][2]);*/

    // to draw as GL_LINES
    indices.push_back(sphere_faces_[i][0]);
    indices.push_back(sphere_faces_[i][1]);
    indices.push_back(sphere_faces_[i][1]);
    indices.push_back(sphere_faces_[i][2]);
    indices.push_back(sphere_faces_[i][2]);
    indices.push_back(sphere_faces_[i][0]);
  } // i

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices_consecutive.size() * sizeof(float), &vertices_consecutive[0], GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid *) 0);
  glEnableVertexAttribArray(position_attribute);

//  glDrawArrays(GL_TRIANGLES, 0, sphere_vertices_.size());
  glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, (void *) 0);

  glDisableVertexAttribArray(position_attribute);
}

void Renderer::RenderSolidSphere(GLuint vao, GLuint vbo, GLuint element_buffer, GLuint shader_program)
{
  SetShaderParameter(shader_program, glm::vec3(0.6f, 0.6f, 0.6f), "material.ambient");//0.3,0.8,0.1
  SetShaderParameter(shader_program, glm::vec3(0.6f, 0.6f, 0.6f), "material.diffuse");
  SetShaderParameter(shader_program, glm::vec3(0.6f, 0.6f, 0.6f), "material.specular");//0.6,0.6,0.6
  SetShaderParameter(shader_program, std::pow(2.0f, 2.0f), "material.shininess");

  SetShaderParameter(shader_program, light_pos_, "light.position");
  SetShaderParameter(shader_program, glm::vec3(0.38f, 0.38f, 0.38f), "light.ambient");
  SetShaderParameter(shader_program, glm::vec3(0.6f, 0.6f, 0.6f), "light.diffuse");
  SetShaderParameter(shader_program, glm::vec3(1.0f, 1.0f, 1.0f), "light.specular");

  SetShaderParameter(shader_program, camera_pos_, "view_pos");

  std::vector<float> vertices_consecutive(sphere_vertices_.size() * 3, 0.0f);
  std::vector<float> normals_consecutive(sphere_normals_for_vertices_.size() * 3, 0.0f);
  for (int i = 0; i < sphere_vertices_.size(); ++i)
  {
    vertices_consecutive[3 * i] = sphere_vertices_[i][0];
    vertices_consecutive[3 * i + 1] = sphere_vertices_[i][1];
    vertices_consecutive[3 * i + 2] = sphere_vertices_[i][2];

    normals_consecutive[3 * i] = sphere_normals_for_vertices_[i][0];
    normals_consecutive[3 * i + 1] = sphere_normals_for_vertices_[i][1];
    normals_consecutive[3 * i + 2] = sphere_normals_for_vertices_[i][2];
  } // i
  std::vector<unsigned int> indices;
  for (int i = 0; i < sphere_faces_.size(); ++i)
  {
    indices.push_back(sphere_faces_[i][0]);
    indices.push_back(sphere_faces_[i][1]);
    indices.push_back(sphere_faces_[i][2]);
  } // i

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               vertices_consecutive.size() * sizeof(float) + normals_consecutive.size() * sizeof(float),
               nullptr,
               GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,
                  0,
                  vertices_consecutive.size() * sizeof(float),
                  &vertices_consecutive[0]);
  glBufferSubData(GL_ARRAY_BUFFER,
                  vertices_consecutive.size() * sizeof(float),
                  normals_consecutive.size() * sizeof(float),
                  &normals_consecutive[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  assert(-1 != position_attribute);
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid *) 0);
  glEnableVertexAttribArray(position_attribute);
  GLint normal_attribute = glGetAttribLocation(shader_program, "normal");
  assert(-1 != normal_attribute);
  glVertexAttribPointer(normal_attribute,
                        3,
                        GL_FLOAT,
                        GL_TRUE,
                        3 * sizeof(float),
                        (void *) (vertices_consecutive.size() * sizeof(float)));
  glEnableVertexAttribArray(normal_attribute);

  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *) 0);

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(normal_attribute);
}

void Renderer::RenderPlane(GLuint vao, GLuint vbo, GLuint element_buffer, GLuint shader_program)
{
  std::vector<float> vertices_consecutive(plane_vertices_.size() * 3, 0.0f);
  for (int i = 0; i < plane_vertices_.size(); ++i)
  {
    vertices_consecutive[3 * i] = plane_vertices_[i][0];
    vertices_consecutive[3 * i + 1] = plane_vertices_[i][1];
    vertices_consecutive[3 * i + 2] = plane_vertices_[i][2];
  } // i
  std::vector<unsigned int> indices;//(plane_faces_.size() * 3);
  for (int i = 0; i < plane_faces_.size(); ++i)
  {
    /*// to draw as GL_TRIANGLES
    indices[3 * i] = plane_faces_[i][0];
    indices[3 * i + 1] = plane_faces_[i][1];
    indices[3 * i + 2] = plane_faces_[i][2];
     */

    // to draw as GL_LINES
    indices.push_back(plane_faces_[i][0]);
    indices.push_back(plane_faces_[i][1]);
    indices.push_back(plane_faces_[i][1]);
    indices.push_back(plane_faces_[i][2]);
    indices.push_back(plane_faces_[i][2]);
    indices.push_back(plane_faces_[i][0]);
  } // i

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices_consecutive.size() * sizeof(float), &vertices_consecutive[0], GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid *) 0);
  glEnableVertexAttribArray(position_attribute);

//  glDrawArrays(GL_POINTS, 0, plane_vertices_.size());
  glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, (void *) 0);

  glDisableVertexAttribArray(position_attribute);
}

void Renderer::ReadShaderSource(const std::string &fname, std::vector<char> &buffer)
{
  std::ifstream in;
  in.open(fname, std::ios::binary | std::ios::in);

  if (in.is_open())
  {
    in.seekg(0, std::ios::end);
    size_t length = (size_t) in.tellg();

    in.seekg(0, std::ios::beg);

    buffer.resize(length + 1);
    in.read((char *) &buffer[0], length);
    buffer[length] = '\0';

    in.close();
  } else
  {
    std::cerr << "Unable to read the shader file \"" << fname << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }
}

GLuint Renderer::LoadAndCompileShader(const std::string &fname, GLenum shader_type)
{
  std::vector<char> buffer;
  ReadShaderSource(fname, buffer);
  const char *src = &buffer[0];

  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint compilation_test;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_test);
  if (!compilation_test)
  {
    std::cerr << "Shader compilation failed with the following message: " << std::endl;
    std::vector<char> compilation_log(512, '\0');
    glGetShaderInfoLog(shader, (GLsizei) compilation_log.size(), NULL, &compilation_log[0]);
    std::cerr << &compilation_log[0] << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  return shader;
}

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path,
                                         const std::string &geometry_shader_path,
                                         const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint geometry_shader = LoadAndCompileShader(geometry_shader_path, GL_GEOMETRY_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, geometry_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(geometry_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}