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
            Mesh(Context& context, const std::string& path);

            //info
            Shared<Render::InputLayout> layout() const;
            Shared<Render::VertexBuffer> vertex_buffer() const;
            Shared<Render::IndexBuffer> index_buffer() const;

            //get surfaces
            const Render::Mesh::SubMeshList& sub_meshs() const;

            //draw all sub meshs
            void draw(Render::Context& render) const;

            //load mesh
            bool load(const std::string& path) override;

        };
    }
}