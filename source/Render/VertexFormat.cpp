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
        size_t m_size;
    };
    
    //map
    
    static std::vector < VertexItem >& vertex_list()
    {
        static std::vector < VertexItem > list;
        return list;
    };
    
    //Add a vertex as commond vertex
    bool Collection::append(const ObjectInfo& info, const AttributeList& attrs, unsigned long type, size_t vertex_size)
    {
        vertex_list().push_back({ info, attrs, type, vertex_size });
        return true;
    }
    //index of layout vector
    InputLayoutId Collection::index_by_type(unsigned long type)
    {
        auto it = std::find_if(vertex_list().begin(), vertex_list().end(), [type](const VertexItem& element){
            return element.m_type == type;
        });
        if(it ==  vertex_list().end()) return ~(size_t(0));
        return std::distance(vertex_list().begin(),it);
    }
    InputLayoutId Collection::index_by_object_id(::Square::uint64 id)
    {
        auto it = std::find_if(vertex_list().begin(), vertex_list().end(), [id](const VertexItem& element){
            return element.m_info.id() == id;
        });
        if(it ==  vertex_list().end()) return ~(size_t(0));
        return std::distance(vertex_list().begin(),it);
    }
    InputLayoutId Collection::index_by_object_name(const std::string& name)
    {
        auto it = std::find_if(vertex_list().begin(), vertex_list().end(), [&name](const VertexItem& element){
            return element.m_info.name() == name;
        });
        if(it ==  vertex_list().end()) return ~(size_t(0));
        return std::distance(vertex_list().begin(),it);
    }
    //build
    InputLayoutList Collection::layouts(Render::Context* render,Render::Shader* shader)
    {
        std::vector< Shared< Render::InputLayout > > output;
        for(auto& vinfo : vertex_list())
        {
            output.push_back(Render::input_layout(render, shader, vinfo.m_attributes));
        }
        return std::move(output);
    }
}
}
}
