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
//  it. A directional light with stable cascades (Scene::CascadeFit::STABLE) keeps a cache for each
//  cascade: drawn again only when the cascade moved or its static casters changed (their levels
//  of detail as they are shown: the fading ones settled); its dynamic casters only in the nearer
//  cascades (dynamic_cascades), over a copy of their caches; the farther ones are their caches.
//  A directional light whose cascades follow the camera draws all of them every frame.
//
#pragma once
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Square/Config.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Pipeline/Drawer.h"
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
		//the cache of the cascades of the directional lights that keep them (on by default; off:
		//all of their casters drawn every frame)
		void cascades_cache(bool enable);
		bool cascades_cache() const;
		//the cascades the dynamic casters are drawn in (the nearer ones; 3 by default): farther,
		//no shadow of what moves
		void dynamic_cascades(int cascades);
		int  dynamic_cascades() const;

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

		//the cache of a directional light: its static casters in each cascade (its buffer, a layer
		//each), a layer at the far depth (to clear one), the place of each cascade when they were
		//drawn, the hash of the static casters of each then
		struct CascadeCache
		{
			std::unique_ptr<ShadowBuffer> m_buffer;
			std::unique_ptr<ShadowBuffer> m_blank;
			std::array< std::vector<unsigned char>, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES > m_keys;
			std::array< uint64, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES > m_casters{};
			size_t m_used{ 0 }; //the draw of the pass it was last used at
		};

		//a renderable among the casters asked
		bool takes(const Renderable& renderable, Casters casters) const;
		//its level of detail settled (a cache: the one more than half shown drawn whole, the other
		//not)
		bool settled(const Renderable& renderable) const;
		//the hash of the static casters of the queues (which ones, where, their level of detail)
		uint64 static_casters(const PoolQueues& queues) const;

		//the hash of the static casters of each cascade (which ones, where, their levels of detail
		//settled)
		void cascades_casters(const PoolQueues& queues, const UniformDirectionShadowLight& cascades, std::array<uint64, DIRECTION_SHADOW_CSM_NUMBER_OF_FACES>& hashes) const;

		//the casters of the queues into the target enabled (its uniforms set); layers: the
		//cascades they are drawn in at most; settle: their levels of detail settled (a cache)
		void draw_casters
		(
		  const PoolQueues& queues
		, const std::string& technique_name
		, EffectPassInputs& inputs
		, const UniformDirectionShadowLight* cascades
		, Casters casters
		, uint32 layers = MULTI_PASS_ALL_LAYERS
		, bool settle = false
		);
		//a caster (in the cascades of the inputs): its transform, its materials, their passes;
		//settle: drawn whole (no fade)
		void draw_caster
		(
		  Renderable& randerable
		, const std::string& technique_name
		, EffectPassInputs& inputs
		, UniformBufferTransform& utransform
		, bool settle
		);
		//the shadow map of a directional light with stable cascades: the caches of the cascades
		//(drawn again if out of date), the nearer ones copied in, the dynamic casters over them
		void draw_cascades
		(
		  const Light& light
		, const UniformDirectionShadowLight& cascades
		, const PoolQueues& queues
		, const std::string& technique_name
		, EffectPassInputs& inputs
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
		//the caches of the cascades, by light
		bool                                          m_cascades_cache{ true };
		int                                           m_dynamic_cascades{ 3 };
		std::unordered_map<const Light*, CascadeCache> m_cascade_caches;
		size_t                                       m_draws{ 0 };
	};
}
}
