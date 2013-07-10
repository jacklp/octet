////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

class mesh_state : public resource {
public:
  // standard attribute names
  enum attribute {
    attribute_position = 0,
    attribute_pos = 0,
    attribute_blendweight = 1,
    attribute_normal = 2,
    attribute_diffuse = 3,
    attribute_color = 3,
    attribute_specular = 4,
    attribute_tessfactor = 5,
    attribute_fogcoord = 5,
    attribute_psize = 6,
    attribute_blendindices = 7,
    attribute_texcoord = 8,
    attribute_uv = 8,
    attribute_tangent = 14,
    attribute_bitangent = 15,
    attribute_binormal = 15,
  };

  // a resource can be stored in a gl buffer or memory
  struct resource {
    unsigned char *ptr;
    GLuint buffer;
  };

  // how to draw a mesh
  struct mesh_instance {
    // which node (model to world matrix) to use
    int node;

    // which mesh to render
    int mesh;

    // what material to use
    int material;

    // for characters, which skin to use
    int skin;

    // for characters, which skeleton to use
    int skeleton;
  };

  // extra information for skinned meshes
  struct skin {
    // the original transform of the skin to world space (bind space)
    mat4t modelToBind;

    // for each node, map from world space (bind space) to model space
    dynarray<mat4t> bindToModel;

    // a name for each joint.
    dynarray<string> joints;
  };

private:
  // todo: make this private
  resource vertices;
  resource indices;

  // compressed format: aaaassttt
  enum { max_slots = 16 };
  unsigned short format[max_slots];

  unsigned num_indices;
  unsigned num_vertices;
  unsigned short stride;
  unsigned short mode;
  unsigned short index_type;

  unsigned char use_vbo;
  unsigned char num_slots;

  chars<allocator> geometry_name;
  chars<allocator> component_name;

  // optional skin
  ptr<skin> skin_;

  mesh_state(mesh_state &rhs);

public:
  mesh_state() {
    init();
  }

  ~mesh_state() {
    release();
  }

  void init(const char *geometry_name="", const char *component_name="", skin *_skin=0) {
    memset(format, 0, sizeof(format));

    num_indices = 0;
    num_vertices = 0;
    stride = 0;
    mode = 0;
    index_type = 0;

    use_vbo = 0;
    num_slots = 0;
    index_type = GL_UNSIGNED_SHORT;
    mode = GL_TRIANGLES;

    this->geometry_name = geometry_name;
    this->component_name = component_name;

    skin_ = _skin;
  }

  void release() {
    if (use_vbo) {
      glDeleteBuffers(1, &vertices.buffer);
      glDeleteBuffers(1, &indices.buffer);
      vertices.buffer = 0;
      indices.buffer = 0;
      use_vbo = 0;
    } else if (index_type && num_vertices) {
      unsigned vsize = stride * num_vertices;
      unsigned isize = mesh_state::kind_size(index_type) * num_indices;
      allocator::free(vertices.ptr, vsize);
      allocator::free(indices.ptr, isize);
      vertices.ptr = 0;
      indices.ptr = 0;
    }
  }

  // eg. add_attribute(attribute_pos, 3, GL_FLOAT)
  unsigned add_attribute(unsigned attr, unsigned size, unsigned kind, unsigned offset) {
    format[num_slots] = (offset << 9) + (attr << 5) + ((size-1) << 3) + (kind - GL_BYTE);
    return num_slots++;
  }

  static unsigned kind_size(unsigned kind) {
    static const unsigned char bytes[] = { 1, 1, 2, 2, 4, 4, 4, 4 };
    return kind < GL_BYTE || kind > GL_FLOAT ? 0 : bytes[kind - GL_BYTE];
  }

  unsigned get_offset(unsigned slot) const {
    return ( format[slot] >> 9 ) & 0x3f;
  }

  unsigned get_attr(unsigned slot) const {
    return ( format[slot] >> 5 ) & 0x0f;
  }

  unsigned get_size(unsigned slot) const {
    return ( ( format[slot] >> 3 ) & 0x03 ) + 1;
  }

  unsigned get_kind(unsigned slot) const {
    return ( ( format[slot] >> 0 ) & 0x07 ) + GL_BYTE;
  }

  unsigned get_stride() const {
    return stride;
  }

  unsigned get_num_vertices() const {
    return num_vertices;
  }

  unsigned get_num_indices() const {
    return num_indices;
  }

  unsigned get_mode() const {
    return mode;
  }

  unsigned get_index_type() const {
    return index_type;
  }

  unsigned get_use_vbo() const {
    return use_vbo;
  }

  unsigned get_num_slots() const {
    return num_slots;
  }

  // get the optional skin data
  skin *get_skin() const {
    return (skin*)skin_;
  }

  // set the optional skin
  // note: the mesh state owns the skin.
  void set_skin(skin *value) {
    skin_ = value;
  }

  // return true if this mesh has a particular attribute
  bool has_attribute(unsigned attr) {
    for (unsigned i = 0; i != num_slots; ++i) {
      if (get_attr(i) == attr) {
        return true;
      }
    }
    return false;
  }

  // avoid using these, please! Just for testing
  const void *get_vertices() const { return vertices.ptr; }
  const void *get_indices() const { return indices.ptr; }
  unsigned get_vertices_size() const { return num_vertices * stride; }
  unsigned get_indices_size() const { return num_indices * kind_size(index_type); }

  unsigned get_slot(unsigned attr) const {
    for (unsigned i = 0; i != max_slots; ++i) {
      if (!format[i]) break;
      if (get_attr(i) == attr) {
        return i;
      }
    }
    return ~0;
  }

  vec4 get_value(unsigned slot, unsigned index) const {
    if (!use_vbo && get_kind(slot) == GL_FLOAT) {
      const float *src = (float*)(vertices.ptr + stride * index + get_offset(slot));
      unsigned size = get_size(slot);
      float x = src[0];
      float y = size > 1 ? src[1] : 0;
      float z = size > 2 ? src[2] : 0;
      float w = size > 3 ? src[3] : 1;
      return vec4(x, y, z, w);
    }
    return vec4(0, 0, 0, 0);
  }

  // get name of geometry for debugging and scene management
  const char *get_geometry_name() {
    return geometry_name.c_str();
  }

  // get name of geometry component for debugging and scene management
  const char *get_component_name() {
    return component_name.c_str();
  }

  void set_value(unsigned slot, unsigned index, vec4 value) {
    if (!use_vbo && get_kind(slot) == GL_FLOAT) {
      float *src = (float*)(vertices.ptr + stride * index + get_offset(slot));
      unsigned size = get_size(slot);
      src[0] = value[0];
      if (size > 1) src[1] = value[1];
      if (size > 2) src[2] = value[2];
      if (size > 3) src[3] = value[3];
    }
  }

  void set_mode(unsigned value) {
    mode = value;
  }

  unsigned get_index(unsigned index) const {
    if (!use_vbo && index_type == GL_UNSIGNED_SHORT) {
      unsigned short *src = (unsigned short*)(indices.ptr + index*2);
      return *src;
    }
    return 0;
  }

  void begin_render(unsigned char **base, unsigned char **index_base) {
    if (use_vbo) {
      glBindBuffer(GL_ARRAY_BUFFER, vertices.buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.buffer);
      *base = 0;
      *index_base = 0;
    } else {
      *base = vertices.ptr;
      *index_base = indices.ptr;
    }
  }

  void allocate(size_t vsize, size_t isize, bool use_vbo_) {
    if (use_vbo_) {
      glGenBuffers(1, &vertices.buffer);
      glGenBuffers(1, &indices.buffer);
      use_vbo = 1;
    } else {
      vertices.ptr = (unsigned char*)allocator::malloc(vsize);
      indices.ptr = (unsigned char*)allocator::malloc(isize);
      use_vbo = 0;
    }
  }

  void assign(unsigned vsize, unsigned isize, unsigned char *vsrc, unsigned char *isrc) {
    if (use_vbo) {
      glBindBuffer(GL_ARRAY_BUFFER, vertices.buffer);
      glBufferData(GL_ARRAY_BUFFER, vsize, vsrc, GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, isrc, GL_STATIC_DRAW);
    } else {
      memcpy(vertices.ptr, vsrc, vsize);
      memcpy(indices.ptr, isrc, isize);
    }
  }

  void copy_format(const mesh_state &src) {
    memcpy(format, src.format, sizeof(format));
  }

  void copy_params(const mesh_state &src) {
    num_indices = src.num_indices;
    num_vertices = src.num_vertices;
    stride = src.stride;
    mode = src.mode;
    index_type = src.index_type;

    use_vbo = src.use_vbo;
    num_slots = src.num_slots;
  }

  void copy_indices(const mesh_state &src) {
    memcpy(indices.ptr, src.indices.ptr, src.num_indices * kind_size(index_type));
  }

  void copy_vertices(const mesh_state &src) {
    memcpy(vertices.ptr, src.vertices.ptr, src.num_vertices * src.stride);
  }

  void set_params(unsigned stride_, unsigned num_indices_, unsigned num_vertices_, unsigned mode_, unsigned index_type_) {
    stride = stride_;
    num_indices = num_indices_;
    num_vertices = num_vertices_;
    mode = mode_;
    index_type = index_type_;
  }

  void dump(FILE *file) {
    fprintf(file, "<model mode=%04x index_type=%04x stride=%d>\n", mode, index_type, stride);
    for (unsigned slot = 0; slot != num_slots; ++slot) {
      fprintf(file, "  <slot n=%d attr=%d kind=%04x size=%d offset=%d>\n", slot, get_attr(slot), get_kind(slot), get_size(slot), get_offset(slot)); 
      const char *fmt[] = { "", "    [%d %f]\n", "    [%d %f %f]\n", "    [%d %f %f %f]\n", "    [%d %f %f %f %f]\n" };
      for (unsigned i = 0; i != num_vertices; ++i) {
        vec4 value = get_value(slot, i);
        fprintf(file, fmt[get_size(slot)], i, value[0], value[1], value[2], value[3]);
      }
      fprintf(file, "  </slot>\n");
    }
    fprintf(file, "  <indices>\n    ");
    for (unsigned i = 0; i != num_indices; ++i) {
      fprintf(file, "%d ", ((short*)indices.ptr)[i]);
    }
    fprintf(file, "\n  </indices>\n");
    fprintf(file, "</model>\n");
  }

  // render a mesh with OpenGL
  // assume the shader, uniforms and render params are already set up.
  void render() {
    unsigned char *base = 0;
    unsigned char *index_base = 0;
    begin_render(&base, &index_base);

    for (unsigned slot = 0; slot != get_num_slots(); ++slot) {
      unsigned size = get_size(slot);
      unsigned kind = get_kind(slot);
      unsigned attr = get_attr(slot);
      unsigned offset = get_offset(slot);
      glVertexAttribPointer(attr, size, kind, GL_FALSE, get_stride(), (void*)(base + offset));
      glEnableVertexAttribArray(attr);
    }

    glDrawElements(get_mode(), get_num_indices(), get_index_type(), (GLvoid*)index_base);

    for (unsigned slot = 0; slot != get_num_slots(); ++slot) {
      unsigned attr = get_attr(slot);
      glDisableVertexAttribArray(attr);
    }
  }
};

