//
//  Queue.cpp
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include <algorithm>
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
    QueueElement* Queue::get_first() const
    {
        sort();
        return m_first;
    }

    void Queue::sort() const
    {
        if (!m_sorted)
        {
            //the elements in the order they were added (the pages), sorted by their key (the
            //same key: as they were added), linked
            m_order.clear();
            m_order.reserve(m_size);
            for (size_t id = 0; id != m_size; ++id)
            {
                m_order.push_back(&(m_pages[id / page_capacity]->operator[](id % page_capacity)));
            }
            std::stable_sort(m_order.begin(), m_order.end(), [](const QueueElement* left, const QueueElement* right)
            {
                return left->m_key < right->m_key;
            });
            for (size_t id = 0; id + 1 < m_order.size(); ++id)
            {
                m_order[id]->m_next = m_order[id + 1];
            }
            if (!m_order.empty())
            {
                m_order.back()->m_next = nullptr;
            }
            m_first = m_order.empty() ? nullptr : m_order.front();
            m_sorted = true;
        }
    }
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
        m_sorted = true;
    }
    //add
    void Queue::push_front_to_back(Weak<BaseObject> entity, float depth)
    {
        //element get (front to back: by its depth)
        QueueElement *e = get_new_element();
        e->m_ref = entity;
        e->m_depth = depth;
        e->m_key = depth;
        e->m_next = nullptr;
        m_sorted = false;
    }
    
    void Queue::push_back_to_front(Weak<BaseObject> entity, float depth)
    {
        //element get (back to front: by minus its depth)
        QueueElement *e = get_new_element();
        e->m_ref = entity;
        e->m_depth = depth;
        e->m_key = -depth;
        e->m_next = nullptr;
        m_sorted = false;
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
    
    void PoolQueues::clear()
    {
        for (Queue& queue : m_queues)
        {
            queue.clear();
        }
    }
}
}
