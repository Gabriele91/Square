//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright ? 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Geometry : public ResourceObject
                              , public SharedObject<Geometry>
	{
		//gpu
		Render::InputLayout*  m_layout{ nullptr };
		Render::VertexBuffer* m_vertex_buffer{ nullptr };
		Render::IndexBuffer*  m_index_buffer{ nullptr };
		//cpu
		Render::AttributeList m_attrs;
		size_t m_index_size;
		size_t m_vertex_n;
		size_t m_vertex_stride;

	public:

		//Geometry
		Geometry(Context& context);

		//data init
		void input_layout(const Render::AttributeList& attrs);
		void vertex_buffer(const unsigned char* data, size_t stride, size_t n);
		void index_buffer(const unsigned char* data, size_t size);

		//data init
		void get_input_layout(const Render::AttributeList& attrs);
		void get_vertex_buffer(const unsigned char* data, size_t stride, size_t n);
		void get_index_buffer(const unsigned char* data, size_t size);

		//load		
		bool load(const std::string& path) override;

		//draw
		void draw();

		//info buffers
		size_t index_size() const;
		size_t vertex_size() const;
		size_t vertex_n() const;
		size_t vertex_stride() const;

		//info layout
		const Render::AttributeList& attributes() const;

		//memory ptr
		Render::InputLayout* input_layout() const;
		Render::InputLayout* vertex_buffer() const;
		Render::IndexBuffer* index_buffer() const;
	};
}
}