//
//  Mesh.h
//  Square
//
//  Created by Gabriele Di Bari on 07/01/2024.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/Resource.h"
#include "Square/Render/Queue.h"
#include "Square/Render/Mesh.h"
#include "Square/Data/ParserStaticMesh.h"
#include <functional>
#include <vector>

namespace Square
{
    namespace Resource
    {
        class SQUARE_API Mesh : public ResourceObject
                              , public InheritableSharedObject<Mesh>
        {
        protected:
            
            Render::Mesh m_mesh;

        public:

            //Init object
            SQUARE_OBJECT(Mesh)

            //Registration in context
            static void object_registration(Context& ctx);

            //constructor
            Mesh(Context& context);
            Mesh(Context& context, const String& path);

            //info
            unsigned int layout_type() const;
            Shared<Render::InputLayout> layout() const;
            Shared<Render::VertexBuffer> vertex_buffer() const;
            Shared<Render::IndexBuffer> index_buffer() const;

            //get surfaces
            const Render::Mesh::SubMeshList& sub_meshs() const;
            size_t number_of_sub_meshs() const { return sub_meshs().size(); }

            //draw all sub meshs (instances > 1 issues an instanced draw call)
            void draw(Render::Context& render, unsigned int instances = 1) const;
            void draw(Render::Context& render, size_t sub_mesh_id, unsigned int instances = 1) const;

            //its triangles in its own space (3 points each, added to out), read from its file (the
            //GPU keeps no copy of them); filter: the sub meshes taken (by their index), none: all
            //of them; false: its file cannot be read
            bool local_triangles(Vector<Vec3>& out, const std::function<bool(size_t submesh)>& filter = nullptr) const;
            //the same, with the uv of each vertex (a vertex without uv: 0)
            bool local_triangles(Vector<Vec3>& out, Vector<Vec2>& out_uvs, const std::function<bool(size_t submesh)>& filter = nullptr) const;

            //its vertices, indices and sub meshes, read from its file (the GPU keeps no copy of
            //them); false: its file cannot be read
            bool local_data(Parser::StaticMesh::Context& out) const;

            //a mesh of its own whose vertices the CPU rewrites (update_vertices), e.g. deformed,
            //from the data of source (its local_data): the file of source its file (its triangles,
            //its data, the collisions on it: the ones of source)
            bool build_dynamic(const Mesh& source, const Parser::StaticMesh::Context& data);
            //its vertices (a dynamic mesh): as many as it has, of its layout (size: their bytes)
            bool update_vertices(const unsigned char* data, size_t size);
            bool dynamic() const { return m_dynamic; }

            //load mesh
            bool load(const String& path) override;

        private:

            //its GPU mesh from data (cpu_access: its vertices rewritten later)
            bool build(Parser::StaticMesh::Context& data, bool cpu_access);
            bool m_dynamic{ false };

        };
    }
}