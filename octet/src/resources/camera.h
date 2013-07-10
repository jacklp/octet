////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene camera
//

class camera : public resource {
  // camera parameters
  int node_;
  bool is_ortho;

  // size (at near plane)
  float left;
  float right;
  float bottom;
  float top;
  float nearVal;
  float farVal;

  // generated matrices
  mat4t worldToCamera;
  mat4t cameraToProjection;

public:
  camera() {
  }

  // call this once to set up the camera
  void set_params(int node, float left, float right, float bottom, float top, float nearVal, float farVal, bool is_ortho) {
    this->left = left;
    this->right = right;
    this->bottom = bottom;
    this->top = top;
    this->nearVal = nearVal;
    this->farVal = farVal;
    this->is_ortho = is_ortho;
    node_ = node;
  }

  void set_perspective(int node_index, float xfov, float yfov, float aspect_ratio, float n, float f)
  {
    float xscale = 0.5f;
    float yscale = 0.5f;
    if (xfov && yfov) {
      xscale = tanf(xfov * (3.14159f/180/2));
      yscale = tanf(yfov * (3.14159f/180/2));
    } else if (yfov && aspect_ratio) {
      yscale = tanf(yfov * (3.14159f/180/2));
      xscale = yscale / aspect_ratio;
    } else if (xfov && aspect_ratio) {
      xscale = tanf(xfov * (3.14159f/180/2));
      yscale = xscale * aspect_ratio;
    }
    //printf("%f %f\n", xscale, yscale);
    set_params(node_index, -n * xscale, n * xscale, -n * yscale, n * yscale, n, f, false);
  }

  void set_ortho(int node_index, float xmag, float ymag, float aspect_ratio, float n, float f)
  {
    set_params(node_index, -xmag, xmag, -ymag, ymag, n, f, true);
  }


  // call this once a frame to set up the camera position
  void set_cameraToWorld(const mat4t &cameraToWorld) {
    // flip it around to transform from world to camera
    cameraToWorld.invertQuick(worldToCamera);

    // build a projection matrix to add perspective
    cameraToProjection.loadIdentity();
    if (is_ortho) {
      cameraToProjection.ortho(left, right, bottom, top, nearVal, farVal);
    } else {
      cameraToProjection.frustum(left, right, bottom, top, nearVal, farVal);
    }
  }

  // call this many times to build matrices for uniforms.
  void get_matrices(mat4t &modelToProjection, mat4t &modelToCamera, const mat4t &modelToWorld)
  {
    // model -> world -> camera
    modelToCamera = modelToWorld * worldToCamera;

    // model -> world -> camera -> projection
    modelToProjection = modelToCamera * cameraToProjection;
  }

  // call this many times to build matrices for uniforms.
  const mat4t &get_cameraToProjection()
  {
    return cameraToProjection;
  }

  // use this to get the camera from the scene
  int node() {
    return node_;
  }
};

