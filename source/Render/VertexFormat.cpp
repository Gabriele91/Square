//
//  VertexFormat.cpp
//  Square
//
//  Created by Gabriele Di Bari on 26/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
#include "Square/Render/VertexLayout.h"


namespace Square
{
namespace Render
{
namespace Layout
{
    SQUARE_REGISTERED_VERTEX(Position2D)
    SQUARE_REGISTERED_VERTEX(Position2DUV)
    SQUARE_REGISTERED_VERTEX(Position3D)
    SQUARE_REGISTERED_VERTEX(Position3DUV)
    SQUARE_REGISTERED_VERTEX(Position3DNormalUV)
    SQUARE_REGISTERED_VERTEX(Position3DNormalTangetBinomialUV)

    //item
    struct VertexItem
    {
        ObjectInfo    m_info;
        AttributeList m_attributes;
        unsigned long m_type;
        size_t        m_size;
		//layout 
#if 0
        Shared< Render::InputLayout > m_layout;
#endif 
		//get input layout
		Shared< Render::InputLayout > layout(Render::Context* render)
		{
#if 0
			//test
			if (m_layout) return m_layout;
			//build
			m_layout = Render::input_layout(render, m_attributes);
			//return
			return m_layout;
#else
            return  Render::input_layout(render, m_attributes);
#endif 
		}
    };
    
    //map
    static std::vector < VertexItem >& vertex_list()
    {
        static std::vector < VertexItem > list;
        return list;
    };
    
    static void clean_up_gpu_layouts()
    {
#if 0
        for (auto& item : vertex_list())
        {
            item.m_layout = nullptr;
        }
#endif 
    }

    //Add a vertex as commond vertex
    bool Collection::append(const ObjectInfo& info, const AttributeList& attrs, unsigned long type, size_t vertex_size)
    {
        vertex_list().push_back({ info, attrs, type, vertex_size });
        return true;
    }
    //index of layout vector
	Shared< Render::InputLayout > Collection::index_by_type(Render::Context* render, unsigned long type)
    {
        auto it = std::find_if(vertex_list().begin(), vertex_list().end(), [type](const VertexItem& element){
            return element.m_type == type;
        });
		if (it == vertex_list().end()) return nullptr;
		return it->layout(render);
    }
	Shared< Render::InputLayout > Collection::index_by_object_id(Render::Context* render, ::Square::uint64 id)
    {
        auto it = std::find_if(vertex_list().begin(), vertex_list().end(), [id](const VertexItem& element){
            return element.m_info.id() == id;
        });
		if (it == vertex_list().end()) return nullptr;
		return it->layout(render);
    }
	Shared< Render::InputLayout > Collection::index_by_object_name(Render::Context* render, const std::string& name)
    {
        auto it = std::find_if(vertex_list().begin(), vertex_list().end(), [&name](const VertexItem& element){
            return element.m_info.name() == name;
        });
        if(it ==  vertex_list().end()) return nullptr;
        return it->layout(render);
    }
}
}
}
