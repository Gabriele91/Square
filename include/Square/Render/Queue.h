//
//  Queue.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
namespace Render
{
    class QueueElement;
    class Queue;
    class QueueIterator;
    class PoolQueues;

    enum QueueType
    {
        RQ_SPOT_LIGHT,
        RQ_POINT_LIGHT,
        RQ_DIRECTION_LIGHT,
        RQ_OPAQUE,
        RQ_TRANSLUCENT,
        RQ_UI,
        RQ_BACKGROUND,
        RQ_MAX
    };

    class SQUARE_API QueueElement
    {
    public:
        //Element of queue
        QueueElement();
        QueueElement(Weak<BaseObject> ref);
        //ref to object
        Weak<BaseObject> m_ref;
        //list
        QueueElement* m_next{ nullptr };
        float         m_depth{ ~0 };
        //fake lock
        Shared<BaseObject> lock() const;
        template < typename T >
        Shared< T > lock() const
        {
            if(auto obj = lock()) return DynamicPointerCast<T>(obj);
            return nullptr;
        }
    };
    
    class SQUARE_API QueueIterator
    {
    public:
        //queue
        QueueIterator(QueueElement* queue = nullptr);
        Shared<BaseObject> lock() const;
        template < typename T >
        Shared< T > lock() const
        {
            if(auto obj = lock()) return DynamicPointerCast<T>(obj);
            return nullptr;
        }
        //iterator ops
        QueueIterator operator++();
        bool operator == (const QueueIterator& other) const;
        bool operator!= (const QueueIterator& other) const;
        
        QueueElement* operator*();
        const QueueElement* operator* () const;
        
    protected:
        //element
        QueueElement* m_ref{ nullptr };
    };
    
    class SQUARE_API Queue
    {
        friend class PoolQueues;
        friend class std::array< Queue, RQ_MAX >;
        //init/delete private
        Queue() = default;
        virtual ~Queue() = default;

    public:
        
        //init
        Queue(Allocator* allocator, size_t capacity = 512);

        //allow move
        Queue(Queue&& other) noexcept = default;
        Queue& operator=(Queue&& other) noexcept = default;

        //no copy
        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;
        
        //add element
        void push_front_to_back(Weak<BaseObject> entity, float depth);
        void push_back_to_front(Weak<BaseObject> entity, float depth);
        
        //info
        QueueElement* get_first() const;
        size_t size() const;
        
        //iterator
        QueueIterator begin();
        QueueIterator end();
        
        //iterator
        QueueIterator begin() const;
        QueueIterator end()   const;
        
        //clear
        void clear();
        
        // Allocator
        Allocator* allocator() const { return m_allocator; };

    private:
        //static size
        static const size_t page_capacity = 128;
        
        //pool type
        using Page  = std::array< QueueElement, page_capacity >;
        using Pages = std::vector< Unique<Page> >;
        
        //pool utilities
        void new_page();
        QueueElement* get_new_element();
        
        //fields
        size_t        m_size{ 0 };
        Pages         m_pages;
        QueueElement* m_first{ nullptr };
        Allocator*    m_allocator{ nullptr };
    };
    
    //all queue
    class SQUARE_API PoolQueues
    {
    public:
        //queues
        std::array< Queue, RQ_MAX > m_queues;
        //get a queue
        Queue& operator[](QueueType type);
        const Queue& operator[](QueueType type) const;
        //Init
        explicit PoolQueues(Allocator* allocator);
        // Allow move
        PoolQueues(PoolQueues&& other) noexcept = default;
        PoolQueues& operator=(PoolQueues && other) noexcept = default;
        //No copy
        PoolQueues(const PoolQueues&) = delete;
        PoolQueues& operator=(const PoolQueues&) = delete;
        //Delete
        virtual ~PoolQueues() = default;
    };
}
}
