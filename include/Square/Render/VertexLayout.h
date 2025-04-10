//
//  VertexFormat.h
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Math/Linear.h"
#include "Square/Driver/Render.h"


namespace Square
{
namespace Render
{
namespace Layout
{
    enum LayoutFields : unsigned long
    {
        //STANDARD
        LF_POSITION_2D = 0b0000000000000001,
        LF_POSITION_3D = 0b0000000000000010,
        LF_NORMAL      = 0b0000000000000100,
        LF_TANGENT     = 0b0000000000001000,
        LF_BINOMIAL    = 0b0000000000010000,
        LF_UVMAP       = 0b0000000000100000,
        //EXTRA (COSTUM)
        LF_EXTRA0      = 0b0000000001000000,
        LF_EXTRA1      = 0b0000000010000000,
        LF_EXTRA2      = 0b0000000100000000,
        LF_EXTRA3      = 0b0000001000000000,
        LF_EXTRA4      = 0b0000010000000000,
        LF_EXTRA5      = 0b0000100000000000,
        LF_EXTRA6      = 0b0001000000000000,
        LF_EXTRA7      = 0b0010000000000000,
        LF_EXTRA8      = 0b0100000000000000,
        LF_EXTRA9      = 0b1000000000000000,
    };
    //list of layouts
    using InputLayoutId   = size_t;
    using InputLayoutList = std::vector< Shared< Render::InputLayout > >;
    
    //collections
    class SQUARE_API Collection
    {
    public:
        //Add a vertex as commond vertex
        static bool append(const ObjectInfo& info, const AttributeList& attrs, unsigned long type, size_t vertex_size);
        //index of layout vector
        static std::optional<AttributeList> attributes_by_type(unsigned long type);                                  //O(N)
        static size_t size_by_type(unsigned long type);                                                              //O(N)
        static Shared< Render::InputLayout > index_by_type(Render::Context* render, unsigned long type);             //O(N)
        static Shared< Render::InputLayout > index_by_object_id(Render::Context* render, ::Square::uint64 id);       //O(N)
        static Shared< Render::InputLayout > index_by_object_name(Render::Context* render, const std::string& name); //O(N + |name|)
    };
    
    //class used for static registration of a costum vertex
    template < class T >
    class CollectionItem
    {
    public:
        CollectionItem()
        {
            Collection::append(T::static_object_info(), T::attributes(), T::type(), sizeof(T));
        }
    };
    
    
    #define SQUARE_REGISTERED_VERTEX(VertexStruct)\
    namespace \
    {  static const auto& _Square_CollectionItem_ ## VertexStruct ## _ = ::Square::Render::Layout::CollectionItem< VertexStruct >(); }
    //base vertex (2D)
    struct Position2D
    {
        SQUARE_STATIC_OBJECT(Position2D)
        
        Vec2 m_position;
        
        static inline AttributeList attributes()
        {
            return Render::AttributeList{
                { Render::ATT_POSITION,  Render::AST_FLOAT2, offsetof(Position2D,m_position) }
            };
        }
        
        static inline unsigned long type()
        {
            return LF_POSITION_2D;
        }
    };
    
    //base vertex (2D) + uv
    struct Position2DUV
    {
        SQUARE_STATIC_OBJECT(Position2DUV)
        
        Vec2 m_position;
        Vec2 m_uvmap;

		Position2DUV() = default;

		Position2DUV
		(
			const Vec2& position,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_uvmap = uvmap;
		}

		Position2DUV
		(
			const Vec2& position
		)
		{
			m_position = position;
			m_uvmap = Vec2(0.0);
		}

        static inline AttributeList attributes()
        {
            return Render::AttributeList{
                { Render::ATT_POSITION,  Render::AST_FLOAT2, offsetof(Position2DUV,m_position) },
                { Render::ATT_TEXCOORD0, Render::AST_FLOAT2, offsetof(Position2DUV,m_uvmap)    }
            };
        }
        
        static inline unsigned long type()
        {
            return LF_POSITION_2D | LF_UVMAP;
        }
    };
    
    //base vertex (3D)
    struct Position3D
    {
        SQUARE_STATIC_OBJECT(Position3D)
        
        Vec3 m_position;
        
        static inline AttributeList attributes()
        {
            return Render::AttributeList{
                { Render::ATT_POSITION,  Render::AST_FLOAT3, offsetof(Position3D,m_position) }
            };
        }
        
        static inline unsigned int type()
        {
            return LF_POSITION_3D;
        }
    };
    
    //base vertex (3D) + uv
    struct Position3DUV
    {
        SQUARE_STATIC_OBJECT(Position3DUV)
        
        Vec3 m_position;
        Vec2 m_uvmap;

		Position3DUV() = default;

		Position3DUV
		(
			const Vec3& position,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_uvmap = uvmap;
		}

		Position3DUV
		(
			const Vec3& position
		)
		{
			m_position = position;
			m_uvmap = Vec2(0.0);
		}
		
        static inline AttributeList attributes()
        {
            return Render::AttributeList{
                { Render::ATT_POSITION,  Render::AST_FLOAT3, offsetof(Position3DUV,m_position) },
                { Render::ATT_TEXCOORD0, Render::AST_FLOAT2, offsetof(Position3DUV,m_uvmap)    }
            };
        }
        
        static inline unsigned long type()
        {
            return LF_POSITION_3D | LF_UVMAP;
        }
    };
    
    //vertex (3D) + normal + uv
    struct Position3DNormalUV
    {
        SQUARE_STATIC_OBJECT(Position3DNormalUV)
        
        Vec3 m_position;
        Vec3 m_normal;
        Vec2 m_uvmap;

		Position3DNormalUV() = default;

		Position3DNormalUV
		(
			const Vec3& position,
			const Vec3& normal,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_normal = normal;
			m_uvmap = uvmap;
		}

		Position3DNormalUV
		(
			const Vec3& position,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_normal = Vec3(0.0);
			m_uvmap = uvmap;
		}

		Position3DNormalUV
		(
			const Vec3& position
		)
		{
			m_position = position;
			m_normal = Vec3(0.0);
			m_uvmap = Vec2(0.0);
		}

        static inline AttributeList attributes()
        {
            return Render::AttributeList{
                { Render::ATT_POSITION,  Render::AST_FLOAT3, offsetof(Position3DNormalUV,m_position) },
                { Render::ATT_NORMAL0,   Render::AST_FLOAT3, offsetof(Position3DNormalUV,m_normal)   },
                { Render::ATT_TEXCOORD0, Render::AST_FLOAT2, offsetof(Position3DNormalUV,m_uvmap)    }
            };
        }
        
        static inline unsigned long type()
        {
            return LF_POSITION_3D | LF_NORMAL | LF_UVMAP;
        }
    };
    
    //vertex (3D) + normal + tangent + bitangent + uv
    struct Position3DNormalTangetBinomialUV
    {
        SQUARE_STATIC_OBJECT(Position3DNormalTangetBinomialUV)
        
        Vec3 m_position;
        Vec3 m_normal;
        Vec3 m_tangent;
        Vec3 m_binomial;
        Vec2 m_uvmap;

		Position3DNormalTangetBinomialUV() = default;

		Position3DNormalTangetBinomialUV
		(
			const Vec3& position,
			const Vec3& normal,
			const Vec3& tangent,
			const Vec3& binomial,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_normal = normal;
			m_tangent = tangent;
			m_binomial = binomial;
			m_uvmap = uvmap;
		}

		Position3DNormalTangetBinomialUV
		(
			const Vec3& position,
			const Vec3& normal,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_normal = normal;
			m_tangent = Vec3(0.0);
			m_binomial = Vec3(0.0);
			m_uvmap = uvmap;
		}

		Position3DNormalTangetBinomialUV
		(
			const Vec3& position,
			const Vec2& uvmap
		)
		{
			m_position = position;
			m_normal = Vec3(0.0);
			m_tangent = Vec3(0.0);
			m_binomial = Vec3(0.0);
			m_uvmap = uvmap;
		}

		Position3DNormalTangetBinomialUV
		(
			const Vec3& position
		)
		{
			m_position = position;
			m_normal = Vec3(0.0);
			m_tangent = Vec3(0.0);
			m_binomial = Vec3(0.0);
			m_uvmap = Vec2(0.0);
		}

        static inline AttributeList attributes()
        {
            return Render::AttributeList{
                { Render::ATT_POSITION,  Render::AST_FLOAT3, offsetof(Position3DNormalTangetBinomialUV,m_position)},
                { Render::ATT_NORMAL0,   Render::AST_FLOAT3, offsetof(Position3DNormalTangetBinomialUV,m_normal)  },
                { Render::ATT_TANGENT0,  Render::AST_FLOAT3, offsetof(Position3DNormalTangetBinomialUV,m_tangent) },
                { Render::ATT_BINORMAL0, Render::AST_FLOAT3, offsetof(Position3DNormalTangetBinomialUV,m_binomial)},
                { Render::ATT_TEXCOORD0, Render::AST_FLOAT2, offsetof(Position3DNormalTangetBinomialUV,m_uvmap)   }
            };
        }
        
        static inline unsigned long type()
        {
            return LF_POSITION_3D | LF_NORMAL | LF_TANGENT | LF_BINOMIAL | LF_UVMAP;
        }
    };
    
}
}
}
