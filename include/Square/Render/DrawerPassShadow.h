//
//  DrawerPassShadow.h
//  Square
//
//  Created by Gabriele Di Bari on 25/07/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
//  The shadow maps of the lights. A light that does not move (spot, point) keeps a cache of its
//  static casters (Renderable::is_static: their actor "static"), drawn again only when the light
//  moved or its static casters changed (which ones, where, their level of detail: a hash of
//  them); every frame the cache is copied into its shadow map, the dynamic casters drawn over
//  it. A directional light (its cascades follow the camera) draws all of them every frame.
//
#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include "Square/Config.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/ShadowBuffer.h"
#include "Square/Render/Transform.h"

namespace Square
{
namespace Render
{
    class SQUARE_API DrawerPassShadow : public DrawerPass
    {
    public:
        //passo
		DrawerPassShadow(Square::Context& context);
		//(its caches: not copied)
		DrawerPassShadow(const DrawerPassShadow&) = delete;
		DrawerPassShadow& operator = (const DrawerPassShadow&) = delete;
        //disegna
        virtual void draw
        (
          Drawer& drawer
        , int num_of_pass
        , const Vec4&  clear_color
        , const Vec4&  ambient_color
		, const Camera& camera
		, const Light& light
        , const Collection& collection
        , const PoolQueues& queues
        )
        override;

		//the cache of the static casters of the lights that do not move (on by default; off: all
		//of them drawn every frame)
		void static_cache(bool enable);
		bool static_cache() const;

    protected:

		//the casters drawn: all of them, the static ones (a cache), the dynamic ones (over it)
		enum class Casters
		{
			ALL,
			STATIC,
			DYNAMIC
		};

		//the cache of a light: its static casters (its buffer), the uniform of its shadow when
		//they were drawn (its place, its views), the hash of those casters then
		struct StaticCache
		{
			std::unique_ptr<ShadowBuffer> m_buffer;
			std::vector<unsigned char>    m_key;
			uint64                        m_casters{ 0 };
			size_t                        m_used{ 0 }; //the draw of the pass it was last used at
		};

		//a renderable among the casters asked
		bool takes(const Renderable& renderable, Casters casters) const;
		//the hash of the static casters of the queues (which ones, where, their level of detail)
		uint64 static_casters(const PoolQueues& queues) const;

		//the casters of the queues into the target enabled (its uniforms set)
		void draw_casters
		(
		  const PoolQueues& queues
		, const std::string& technique_name
		, EffectPassInputs& inputs
		, const UniformDirectionShadowLight* cascades
		, Casters casters
		);
		//a caster (in the cascades of the inputs): its transform, its materials, their passes
		void draw_caster
		(
		  Renderable& randerable
		, const std::string& technique_name
		, EffectPassInputs& inputs
		, UniformBufferTransform& utransform
		);
		//the shadow map of a light that does not move: its cache (drawn again if out of date)
		//copied in, the dynamic casters over it
		void draw_cached
		(
		  const Light& light
		, const std::vector<unsigned char>& key
		, const PoolQueues& queues
		, const std::string& technique_name
		, EffectPassInputs& inputs
		);
		//the caches of the lights not drawn for a while: gone
		void forget_old_caches();

        //context
        Square::Context& context();
        const Square::Context& context() const;
        //render
        Render::Context& render();
        const Render::Context& render() const;
        //CPU DATA
        Square::Context& m_context;
        //GPU DATA
        Shared<Render::ConstBuffer> m_cb_camera;
		Shared<Render::ConstBuffer> m_cb_transform;
		//SHADOW GPU DATA
		Shared<Render::ConstBuffer> m_cb_spot_light;
		Shared<Render::ConstBuffer> m_cb_point_light;
		Shared<Render::ConstBuffer> m_cb_direction_light;
		//the caches of the static casters, by light
		bool                                         m_static_cache{ true };
		std::unordered_map<const Light*, StaticCache> m_caches;
		size_t                                       m_draws{ 0 };
	};
}
}
