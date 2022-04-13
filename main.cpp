#include "main.h"

void ConvertMap(cy::GLSLProgram &shader, GLenum fromtype, unsigned int from,
                unsigned int to, GLuint type, int w, int h, int VAO, int nv,
                bool mipmap);

void GenerateBRDFMap(cy::GLSLProgram &shader, GLenum type, int brdfMap, int w,
                     int h); 

int main(void) {
  //----------
  // Set up
  //----------
  SetUpGLFW();
  SetUpIMGUI();
  SetUpGLEW();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  //----------
  // Load mesh
  //----------
  cy::TriMesh teapot, cube, sphere;

  teapot.LoadFromFileObj("./resources/teapot.obj");
  cube.LoadFromFileObj("./resources/cube.obj");
  sphere.LoadFromFileObj("./resources/sphere.obj");

  int datasize = 0;
  float *datateapot = GetMeshInfo(teapot, true, false, datasize);
  int sizes[2] = {3, 3};
  int VAO = CreateVAO(datateapot, datasize, 2, sizes);

  datasize = 0;
  float *datacube = GetMeshInfo(cube, false, false, datasize);
  int sizescube[1] = {3};
  int VAOCube = CreateVAO(datacube, datasize, 1, sizescube);

  datasize = 0;
  float *datasphere = GetMeshInfo(sphere, false, false, datasize);
  int sizessphere[1] = {3};
  int VAOSphere = CreateVAO(datasphere, datasize, 1, sizessphere);

  //----------
  // Load shaders
  //----------
  cy::GLSLProgram teapotShader, toCubemapShader, envShader, convolutionShader,
      prefilterShader, brdfShader, lightShader;
  teapotShader.BuildFiles("./shaders/teapot.vs", "./shaders/teapot.fs");
  toCubemapShader.BuildFiles("./shaders/cube.vs", "./shaders/cube.fs");
  envShader.BuildFiles("./shaders/cube.vs", "./shaders/env.fs");
  convolutionShader.BuildFiles("./shaders/cube.vs", "./shaders/convo.fs");
  prefilterShader.BuildFiles("./shaders/cube.vs", "./shaders/prefilter.fs");
  brdfShader.BuildFiles("./shaders/brdf.vs", "./shaders/brdf.fs");
  lightShader.BuildFiles("./shaders/light.vs", "./shaders/light.fs");

  //----------
  // Load texture
  //----------
  int hdrMap = LoadTexture((char *)"./resources/scene.hdr");

  //-----------
  // Create render buffer and frame buffer to convert hdr image to a env map
  //-----------
  int w = 512;
  int h = 512;
  int envMap = CreateEnvMap(w, h, false);
  ConvertMap(toCubemapShader, GL_TEXTURE_2D, hdrMap, envMap,
             GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3, false);

  //-----------
  // Convert given irradianceMap to a cubemap
  //-----------
  int diffuseMap = LoadTexture((char *)"./resources/diffusemap.hdr");
  int irradianceMapGiven = CreateEnvMap(w, h, false);
  ConvertMap(toCubemapShader, GL_TEXTURE_2D, diffuseMap, irradianceMapGiven,
             GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3, false);

  //-----------
  // Create render buffer and frame buffer to convert env cube map to diffuse
  // irradiance lighting map
  //-----------
  w = 32;
  h = 32;
  int irradianceMapMy = CreateEnvMap(w, h, false);
  ConvertMap(convolutionShader, GL_TEXTURE_CUBE_MAP, envMap, irradianceMapMy,
             GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3, false);

  //-----------
  // Generate prefilter mipmap
  //-----------
  w = 128;
  h = 128;
  int prefilterMap = CreateEnvMap(w, h, true);
  ConvertMap(prefilterShader, GL_TEXTURE_CUBE_MAP, envMap, prefilterMap,
             GL_DEPTH_COMPONENT24, w, h, VAOCube, cube.NF() * 3, true);
  
  //-----------
  // Generate BRDF map
  //-----------
  w = 512;
  h = 512;
  int brdfMap = CreateTexture( GL_TEXTURE_2D, w, h); 
  GenerateBRDFMap(brdfShader, GL_TEXTURE_2D, brdfMap, w, h);

  //----------
  // Matrix transformation set up
  //----------
  cy::Matrix4f M, M2, V, P, S, R, T, S2;

  // For Teapot
  P.SetPerspective(45.0f * M_PI / 180.0, (float)SCR_W / (float)SCR_H, 0.1f,
                   100.0f);
  S.SetScale(0.05f, 0.05f, 0.05f);
  R.SetRotationX(-90.0f * M_PI / 180.0);
  M = S * R;

  // For light
  S2.SetScale(0.01f, 0.01f, 0.01f);

  // For convert cube to center
  cy::Matrix4f M3, S3;
  S3.SetScale(10.0f, 10.0f, 10.0f);
  T.SetTranslation(cy::Vec3f(-0.5f, -0.5f, -0.5f));
  M3 = S3 * T;

  ImGuiIO &io = ImGui::GetIO();
  const char* maps[3] = {"cube map", "diffuse light map", "prefilter map"};
  glViewport(0, 0, SCR_W, SCR_H);

  teapotShader.Bind();
  teapotShader["irradianceMap"] = 0;
  teapotShader["prefilterMap"] = 1;
  teapotShader["brdfMap"] = 2;

  envShader.Bind();
  envShader["envMap"] = 0;

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();

    //----------
    // IMGUI
    //----------

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    onImgui = io.WantCaptureMouse;
    ImGui::Begin("Window");
    ImGui::SliderFloat("metallic", &metallic, 0.0f, 1.0f);
    ImGui::SliderFloat("roughness", &roughness, 0.0f, 1.0f);
    ImGui::SliderFloat("ao", &ao, 0.0f, 1.0f);
    ImGui::Checkbox("gamma correction", (bool *)&gammacorrect);
    ImGui::Checkbox("convolute?", &convolute);
    ImGui::ColorEdit3("albedo", albedo);
    ImGui::ListBox("environment map", &lightmap, maps, 3, -1);
    ImGui::End();
    ImGui::Render();

    if (convolute)
      irradianceMap = irradianceMapMy;
    else
      irradianceMap = irradianceMapGiven;

    if (lightmap == 0)
      enviromentMap = envMap;
    else if(lightmap == 1)
      enviromentMap = irradianceMap;
    else
      enviromentMap = prefilterMap;

    //----------
    // Opengl Render
    //----------
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    V.SetView(camera.pos, camera.lookat, camera.up);
    //------
    // Enviroment
    //-----
    glDepthMask(GL_FALSE);
    envShader.Bind();
    glBindVertexArray(VAOCube);
    envShader["mvp"] = P * cy::Matrix4f(V.GetSubMatrix3()) * M3;
    envShader["model"] = M3;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, enviromentMap);
    envShader.SetUniform("gammacorrect", gammacorrect);
    glDrawArrays(GL_TRIANGLES, 0, cube.NF() * 3);
    glDepthMask(GL_TRUE);

    // ------
    // Light
    // ------
    lightShader.Bind();
    glBindVertexArray(VAOSphere);
    T.SetTranslation(lightPosition);
    M2 = T * S2;
    lightShader["mvp"] = P * V * M2;
    glDrawArrays(GL_TRIANGLES, 0, sphere.NF() * 3);

    // ------
    // Teapot
    // ------
    teapotShader.Bind();
    glBindVertexArray(VAO);
    teapotShader["mvp"] = P * V * M;
    teapotShader["model"] = M;
    teapotShader["metallic"] = metallic;
    teapotShader["roughness"] = roughness;
    teapotShader["ao"] = ao;
    teapotShader["camPos"] = camera.pos;
    teapotShader["lightPosition"] = lightPosition;
    teapotShader["lightColor"] = lightColor;
    teapotShader.SetUniform("gammacorrect", gammacorrect);
    teapotShader.SetUniform3("albedo", 1, albedo);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfMap);

    glDrawArrays(GL_TRIANGLES, 0, teapot.NF() * 3);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  free(datateapot);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void ConvertMap(cy::GLSLProgram &shader, GLenum fromtype, unsigned int from,
                unsigned int to, GLuint type, int w, int h, int VAO, int nv,
                bool mipmap) {

  int fbo = CreateFramebuffer();
  int rbo = CreateRenderbuffer(type, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

  cy::Matrix4f P, v1, v2, v3, v4, v5, v6, M, S, T;
  cy::Vec3f origin(0.0f);
  P.SetPerspective(90.0f * M_PI / 180.0, 1.0f, 0.1f, 100.0f);
  v1.SetView(origin, cy::Vec3f(1.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, -1.0f, 0.0f));
  v2.SetView(origin, cy::Vec3f(-1.0f, 0.0f, 0.0f),
             cy::Vec3f(0.0f, -1.0f, 0.0f));
  v3.SetView(origin, cy::Vec3f(0.0f, 1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f));
  v4.SetView(origin, cy::Vec3f(0.0f, -1.0f, 0.0f),
             cy::Vec3f(0.0f, 0.0f, -1.0f));
  v5.SetView(origin, cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec3f(0.0f, -1.0f, 0.0f));
  v6.SetView(origin, cy::Vec3f(0.0f, 0.0f, -1.0f),
             cy::Vec3f(0.0f, -1.0f, 0.0f));
  cy::Matrix4f Vs[] = {v1, v2, v3, v4, v5, v6};

  S.SetScale(2.0f, 2.0f, 2.0f);
  T.SetTranslation(cy::Vec3f(-0.5f, -0.5f, -0.5f));
  M = S * T;

  shader.Bind();
  shader["model"] = M;
  shader["textureMap"] = 0;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(fromtype, from);

  glViewport(0, 0, w, h);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  if (mipmap) {
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
      // reisze framebuffer according to mip-level size.
      unsigned int mipWidth = w * std::pow(0.5, mip);
      unsigned int mipHeight = h * std::pow(0.5, mip);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth,
                            mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);
      float roughnessmap = (float)mip / (float)(maxMipLevels - 1);
      shader["roughness"] = roughnessmap;

      for (unsigned int i = 0; i < 6; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, to, mip);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader["mvp"] = P * Vs[i] * M;
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, nv);
      }
    }

  } else {
    for (unsigned int i = 0; i < 6; ++i) {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, to, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader["mvp"] = P * Vs[i] * M;
      glBindVertexArray(VAO);
      glDrawArrays(GL_TRIANGLES, 0, nv);
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GenerateBRDFMap(cy::GLSLProgram &shader, GLenum type, int brdfMap, int w,
                     int h) {
  int fbo = CreateFramebuffer();
  int rbo = CreateRenderbuffer(type, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
  
  float quad[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
  int sizes[2] = {3,2};
  int VAO = CreateVAO(quad, 20*sizeof(float), 2, sizes);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfMap, 0);

  glViewport(0, 0, w, h);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader.Bind();
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
