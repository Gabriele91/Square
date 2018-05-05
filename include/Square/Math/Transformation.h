//
//  Transformation.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include <glm/gtc/matrix_transform.hpp>
#include "Square/Math/Linear.h"

namespace Square
{
    
    template < class T, class V >
    inline T translate(const T& mat,const V& v)
    {
        return glm::translate(mat,v);
    }
    
    template < class T, class V >
    inline T scale(const T& mat,const V& v)
    {
        return glm::scale(mat,v);
    }
    
    template < class T, class A, class V >
    inline T rotate(const T& q,const A& angle,const V& v)
    {
        return glm::rotate(q, angle, v);
    }
    
    template < class T >
    inline auto perspective(const T& fovy,const T& aspect,const T& near,const T& far) -> decltype( glm::perspective<T>(fovy, aspect, near, far) )
    {
        return glm::perspective<T>(fovy, aspect, near, far);
    }
    
    template < class T >
    inline auto ortho(const T& left,const T& right,const T& top,const T& bottom,const T& zNear, const T& zFar) -> decltype( glm::ortho(left, right, bottom, top, zNear, zFar) )
    {
        return glm::ortho(left, right, bottom, top, zNear, zFar);
    }
    
    template < class T >
    inline auto look_at(const T& eye,const T& center, const T& up) -> decltype( glm::lookAt(eye, center, up) )
    {
        return glm::lookAt(eye, center, up);
    }
}
