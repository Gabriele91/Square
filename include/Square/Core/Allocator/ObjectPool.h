//
//  Square
//
//  Created by Gabriele Di Bari on 14/10/2026.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Allocator/PoolAllocator.h"
#include <utility>

namespace Square
{
	//The objects of a type in a pool:
	//    ObjectPool<Particle> particles(context.allocator());
	//    Particle* particle = particles.create(...); ... particles.destroy(particle);
	//Not thread safe.
	template < class T >
	class ObjectPool
	{
	public:
		explicit ObjectPool(Allocator* parent, size_t objects_per_chunk = 64)
		: m_pool(parent, sizeof(T), alignof(T), objects_per_chunk)
		{
		}

		template < typename... Args >
		T* create(Args&&... args)
		{
			void* ptr = m_pool.alloc(sizeof(T), typeid(T).name(), ALCT_DEFAULT);
			if (!ptr) throw std::bad_alloc();
			try
			{
				return new (ptr) T(std::forward<Args>(args)...);
			}
			catch (...)
			{
				m_pool.free(ptr);
				throw;
			}
		}

		void destroy(T* object)
		{
			if (object)
			{
				object->~T();
				m_pool.free(object);
			}
		}

		size_t size() const { return m_pool.used(); }
		PoolAllocator& allocator() { return m_pool; }

	private:
		PoolAllocator m_pool;
	};
}
