//
//  Queue.cpp
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Render/Queue.h"
namespace Square
{
namespace Render
{
    /////////////////////////////////////////////////////////////////////
    QueueElement::QueueElement() {};
    QueueElement::QueueElement(Weak<BaseObject> ref) : m_ref(ref) {};
    Shared<BaseObject> QueueElement::lock() const { return m_ref.lock(); }
    /////////////////////////////////////////////////////////////////////
    //iterator
    QueueIterator::QueueIterator(QueueElement* queue) { m_ref = queue; }
    Shared<BaseObject> QueueIterator::lock() const { return m_ref->lock();  }
    //iterator ops
    QueueIterator QueueIterator::operator++()
    {
        (*this) = QueueIterator(m_ref->m_next);
        return *this;
    }
    bool QueueIterator::operator == (const QueueIterator& other) const { return m_ref == other.m_ref; }
    bool QueueIterator::operator!=(const QueueIterator& other) const { return m_ref != other.m_ref; }
    QueueElement* QueueIterator::operator*() { return m_ref; }
    const QueueElement* QueueIterator::operator* () const { return m_ref; }
    /////////////////////////////////////////////////////////////////////
    Queue::Queue(Allocator* allocator, size_t capacity) : m_allocator(allocator)
    {
        //get page size
        size_t npage = capacity / page_capacity;
        //upper boud
        if (capacity % page_capacity) ++npage;
        //alloc
        for (size_t i = 0; i != npage; ++i) new_page();
    }
    //pages
    void Queue::new_page()
    {
        m_pages.push_back(MakeUnique<Page>(allocator()));
    }
    
    QueueElement* Queue::get_new_element()
    {
        size_t page    = m_size / page_capacity;
        size_t element = m_size % page_capacity;
        //compute new size
        ++m_size;
        //alloc
        if (page == m_pages.size()) new_page();
        //get
        return &(m_pages[page]->operator[](element));
    }
    
    //info
    QueueElement* Queue::get_first() const { return m_first; }
    size_t Queue::size() const { return m_size; }
    //iterator
    QueueIterator Queue::begin() { return QueueIterator(get_first()); }
    QueueIterator Queue::end() { return QueueIterator(); }
    QueueIterator Queue::begin() const { return QueueIterator(get_first()); }
    QueueIterator Queue::end() const { return QueueIterator(); }
    //clear
    void  Queue::clear()
    {
        m_first = nullptr;
        m_size = 0;
    }
    //add
    void Queue::push_front_to_back(Weak<BaseObject> entity, float depth)
    {
        //element get
        QueueElement *e = get_new_element();
        //init
        e->m_ref = entity;
        e->m_depth = depth;
        //next
        e->m_next = nullptr;
        //loop vars
        QueueElement* last = nullptr;
        QueueElement* current = m_first;
        //insert sort, front to back
        for (; current;
             last = current,
             current = current->m_next)
        {
            if (current->m_depth > e->m_depth) break;
        }
        //last iteration
        if (last)
        {
            e->m_next = current;
            last->m_next = e;
        }
        else
        {
            e->m_next = m_first;
            m_first = e;
        }
    }
    
    void Queue::push_back_to_front(Weak<BaseObject> entity, float depth)
    {
        //element get
        QueueElement *e = get_new_element();
        //init
        e->m_ref = entity;
        e->m_depth = depth;
        //link
        e->m_next = nullptr;
        //loop vars
        QueueElement* last = nullptr;
        QueueElement* current = m_first;
        //insert sort, back to front
        for (; current;
             last = current,
             current = current->m_next)
        {
            if (current->m_depth < e->m_depth) break;
        }
        //last iteration
        if (last)
        {
            e->m_next = current;
            last->m_next = e;
        }
        else
        {
            e->m_next = m_first;
            m_first = e;
        }
    }
    
    //PoolQueues
    Queue& PoolQueues::operator[](QueueType type)
    {
        return m_queues[type];
    }
    const Queue& PoolQueues::operator[](QueueType type) const
    {
        return m_queues[type];
    }
    //Init
    PoolQueues::PoolQueues(Allocator* allocator)
    {
        for (Queue& queue : m_queues)
        {
            new (&queue) Queue(allocator);
        }
    }
}
}
