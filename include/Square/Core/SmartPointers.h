//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <assert.h>
#include "Square/Config.h"
#include "Square/Core/Allocator.h"

namespace Square
{
    class DefaultDelete
    {
        Allocator* m_allocator;

    public:
        // Default constructor
        DefaultDelete() noexcept : m_allocator(nullptr) {}

        DefaultDelete(Allocator* allocator) noexcept
        : m_allocator(allocator)
        {
        }

        template<typename T>
        void operator() (T* ptr)
        {
            if (ptr) 
            {
                square_assert(m_allocator);
                m_allocator->free((void*)ptr);
            }
        }
    };

    template < class T >
    using Shared = std::shared_ptr< T >;
    
    template < class T >
    using Unique = std::unique_ptr< T, DefaultDelete >;
    
    template < class T >
    using Weak = std::weak_ptr< T >;
    
    template < class T, class U >
    static inline Shared<T> StaticPointerCast( const Shared<U>& sptr ) noexcept
    {
        const auto raw_ptr = static_cast<typename Shared<T>::element_type*>(sptr.get());
        return Shared<T>(sptr, raw_ptr);
    }
    
    template < class T, class U >
    static inline Shared<T> DynamicPointerCast( const Shared<U>& sptr ) noexcept
    {
        const auto raw_ptr = dynamic_cast<typename Shared<T>::element_type*>(sptr.get());
        return raw_ptr ? Shared<T>(sptr, raw_ptr) : nullptr;
    }
    
    template < class T, class U >
    static inline Shared<T> ConstPointerCast( const Shared<U>& sptr ) noexcept
    {
        const auto raw_ptr = const_cast<typename Shared<T>::element_type*>(sptr.get());
        return Shared<T>(sptr, raw_ptr);
    }

	template < class T >
	class SharedObject : public std::enable_shared_from_this<T>
    { 
        using BaseSharedFromThis_t = std::enable_shared_from_this<T>;

    public:
        
        using SharedObject_t = SharedObject;

        [[nodiscard]]
        Shared<T> shared_from_this()
        {
            return this->BaseSharedFromThis_t::shared_from_this();
        }

        [[nodiscard]]
        Shared<const T> shared_from_this() const
        {
            return this->BaseSharedFromThis_t::shared_from_this();
        }

        [[nodiscard]]
        Weak<T> weak_from_this() noexcept
        {
            return this->BaseSharedFromThis_t::weak_from_this();
        }

        [[nodiscard]] 
        Weak<const T> weak_from_this() const noexcept
        {
            return this->BaseSharedFromThis_t::weak_from_this();
        }

    protected:
        constexpr SharedObject(Allocator* allocator) noexcept 
        : BaseSharedFromThis_t()
        , m_allocator(allocator)
        {
        }

        SharedObject(const SharedObject& obj) noexcept
        : BaseSharedFromThis_t()
        , m_allocator(obj.allocator())
        {
        }

        SharedObject& operator=(const SharedObject&) noexcept
        {
            return *this;
        }

        ~SharedObject() = default;

        Allocator* allocator() const { return m_allocator; }

    private:
        friend class std::shared_ptr<T>;
        friend class std::unique_ptr<T, DefaultDelete>;

        Allocator* m_allocator;
    };

    template< class T, class... Args >
    static inline Shared<T> MakeShared(Allocator* allocator, Args&&... args) 
    { 
        return std::move(Shared<T>(SQ_NEW(allocator, T, AllocType::ALCT_DEFAULT) T(std::forward<Args>(args)...), DefaultDelete(allocator)));
    }

    template< class T, class... Args >
    static inline Unique<T> MakeUnique(Allocator* allocator, Args&&... args) 
    { 
        return std::move(Unique<T>(SQ_NEW(allocator, T, AllocType::ALCT_DEFAULT) T(std::forward<Args>(args)...), DefaultDelete(allocator)));
    }
    // Specialization
    template< class T, class U, class... Args >
    static inline std::enable_if_t<std::is_base_of<U, SharedObject<U>>::value, Shared<T> >
    MakeShared(U& shared_object, Args&&... args) 
    {
        return std::move(Shared<T>(SQ_NEW(shared_object.allocator(), T, AllocType::ALCT_DEFAULT) T(shared_object, std::forward<Args>(args)...),
                                   DefaultDelete(shared_object.allocator())));
    }

    template< class T, class U, class... Args >
    static inline std::enable_if_t<std::is_base_of<U, SharedObject<U>>::value, Unique<T> >
    MakeUnique(U& shared_object, Args&&... args) {
        return std::move(Unique<T>(SQ_NEW(shared_object.allocator(), T, AllocType::ALCT_DEFAULT) T(shared_object, std::forward<Args>(args)...),
                                   DefaultDelete(shared_object.allocator())));
    }
    
    // Specialization
    template< class T, class U, class... Args >
    static inline std::enable_if_t<std::is_base_of<U, SharedObject<U>>::value, Shared<T> >
    MakeShared(const U& shared_object, Args&&... args) 
    {
        return std::move(Shared<T>(SQ_NEW(shared_object.allocator(), T, AllocType::ALCT_DEFAULT) T(shared_object, std::forward<Args>(args)...),
                                   DefaultDelete(shared_object.allocator())));
    }

    template< class T, class U, class... Args >
    static inline std::enable_if_t<std::is_base_of<U, SharedObject<U>>::value, Unique<T> >
    MakeUnique(const U& shared_object, Args&&... args) {
        return std::move(Unique<T>(SQ_NEW(shared_object.allocator(), T, AllocType::ALCT_DEFAULT) T(shared_object, std::forward<Args>(args)...),
                                   DefaultDelete(shared_object.allocator())));
    }
}
