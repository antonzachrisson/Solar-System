// model.cpp

#include "spinach.hpp"

#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>

struct model_vertex_t {
   float x, y, z;
   float u, v;
};

// static 
bool mesh::create_from_file(mesh &model, const char *filename)
{
   Assimp::Importer importer;
   auto scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);
   if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
      return false;
   }

   const auto root_node = scene->mRootNode;
   const auto mesh_index = root_node->mMeshes[0];
   const auto mesh_data  = scene->mMeshes[mesh_index];
   
   const bool has_texcoords = mesh_data->HasTextureCoords(0);
   const uint32 face_count  = mesh_data->mNumFaces;
   const uint32 vertex_count = face_count * 3;
   
   uint32 offset = 0;
   std::vector<model_vertex_t> vertices(vertex_count, model_vertex_t{});
   for (uint32 face_index = 0; face_index < face_count; face_index++) {
      const auto &face = mesh_data->mFaces[face_index];
      for (uint32 vertex_index = 0; vertex_index < 3; vertex_index++) {
         const auto at = face.mIndices[vertex_index];
         auto position = mesh_data->mVertices[at];
         auto texcoord = aiVector2D{};
         if (has_texcoords) {
            auto texcoord3 = mesh_data->mTextureCoords[0][at];
            texcoord = { texcoord3.x, texcoord3.y };
         }
   
         vertices[offset++] = model_vertex_t{ position.x, position.y, position.z, texcoord.x, texcoord.y };
      }
   }

   return model.create(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, sizeof(model_vertex_t), int(vertices.size()), vertices.data());
}

mesh::mesh(const vertex_layout *layout)
   : m_transform(1.0f)
   , m_layout(layout)
{
}

bool mesh::valid() const
{
   return m_buffer.is_valid();
}

bool mesh::create(const primitive_topology topology, const int stride, const int count, const void *data)
{
   m_primitive_count = count;
   m_topology = topology;
   return m_buffer.create(stride * count, data);
}

void mesh::update(const int stride, const int count, const void *data)
{
   m_buffer.update(stride * count, data);
}

void mesh::destroy()
{
   m_primitive_count = 0;
   m_buffer.destroy();
}

void mesh::set_transform(const glm::mat4 &transform)
{
   m_transform = transform;
   m_material.set_parameter("u_world", m_transform);
}

void mesh::draw(render_backend &backend)
{
   m_material.bind(backend);

   backend.set_vertex_buffer(m_buffer);
   backend.set_vertex_layout(*m_layout);
   backend.set_blend_state(false);
   backend.set_depth_state(true, true);
   backend.set_rasterizer_state(CULL_MODE_BACK, FRONT_FACE_CW);
   backend.draw(m_topology, 0, m_primitive_count);
}
