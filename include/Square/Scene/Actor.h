//
//  Square
//
//  Created by Gabriele Di Bari on 20/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"

namespace Square
{
namespace Scene
{
    class SQUARE_API Actor : public SmartPointers<Actor>, public Uncopyable
    {
    public:
        
        //add a child
        void add(Actor::SPtr child);
        
        //remove a child
        void remove(Actor::SPtr child);
        void remove_from_parent();
        
        //get parent
        Actor::SPtr parent() const;
        
        //contains an actor
        bool contains(Actor::SPtr child) const;
        
        //matrix op
        void translation(const Vec3& vector);
        void move(const Vec3& vector);
        void turn(const Quat& rot);
        
        void position(const Vec3& pos);
        void rotation(const Quat& rot);
        void scale(const Vec3& pos);
        
        Vec3 position(bool global = false) const;
        Quat rotation(bool global = false) const;
        Vec3 scale(bool global = false) const;
        
        Mat4 const& local_model_matrix();
        Mat4 const& global_model_matrix();
        
    protected:
        //fake const
        Mat4 const& model_matrix() const;
        //local
        struct Tranform
        {
            bool m_dirty   { false                  };
            Vec3 m_position{ 0.0f, 0.0f, 0.0f       };
            Quat m_rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
            Vec3 m_scale   { 1.0f, 1.0f, 1.0f       };
        };
        //Local tranform
        mutable Tranform m_tranform;
        //matrix
        mutable Mat4  m_model_local;
        mutable Mat4  m_model_global;
        //compute
        void compute_matrix() const;
        //send dirty
        void    set_dirty();
        void    send_dirty();
        //parent
        Actor::SPtr m_parent;
        //child list
        std::vector< Actor::SPtr > m_childs;
    };
}
}
