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
        Shared<BaseObject> lock();
    };
    
    class SQUARE_API QueueIterator
    {
    public:
        //queue
        QueueIterator(QueueElement* queue = nullptr);
        Shared<BaseObject> lock() const;
        //iterator ops
        QueueIterator operator++();
        bool operator == (const QueueIterator& other);
        bool operator!=(const QueueIterator& other);
        Shared<BaseObject> operator*();
    protected:
        //element
        QueueElement* m_ref{ nullptr };
    };
    
    class SQUARE_API Queue
    {
    public:
        
        //init
        Queue(size_t capacity = 512);
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
        
        //clear
        void clear();
        
    private:
        //static size
        static const size_t page_capacity = 128;
        
        //pool type
        using Page  = QueueElement[];
        using Pages = std::vector< Unique<Page> >;
        
        //pool utilities
        void new_page();
        QueueElement* get_new_element();
        
        //fields
        size_t        m_size{ 0 };
        Pages         m_pages;
        QueueElement* m_first{ nullptr };
    };
    
    
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
    
    
    //all queue
    class SQUARE_API PoolQueues
    {
    public:
        //queues
        Queue m_queues[RQ_MAX];
        //get a queue
        Queue& operator[](QueueType type);
        const Queue& operator[](QueueType type) const;
    };
}
}
