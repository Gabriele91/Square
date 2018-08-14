//
//  Transformation.h
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Math/Linear.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Square
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto translate(const TVec3< Scalar, P >& v) -> TMat4< Scalar, P >
	{
		return glm::translate(v);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
    inline auto translate(const TMat4<Scalar,P>& mat,const TVec3< Scalar, P >& v) -> TMat4< Scalar, P >
    {
        return glm::translate(mat,v);
    }
    
	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto scale(const TVec3< Scalar, P >& s) -> TMat4< Scalar, P >
	{
		return glm::scale(s);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto scale(const TMat4<Scalar, P>& mat, const TVec3< Scalar, P >& s) -> TMat4< Scalar, P >
	{
		return glm::scale(mat, s);
	}
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template < class T, class A, class V >
    inline T rotate(const T& q,const A& angle,const V& v)
    {
        return glm::rotate(q, angle, v);
    }

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_euler(const Scalar& x, const Scalar& y, const Scalar& z) -> TQuat< Scalar, P >
	{
		return glm::eulerAngleXYZ<Scalar>(x, y, z);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_euler(const TVec3< Scalar, P >& vec) -> TQuat< Scalar, P >
	{
		return glm::eulerAngleXYZ<Scalar, P>(vec.x, vec.y, vec.z);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_matrix_angle_X(const Scalar& x) -> TMat4< Scalar, P >
	{
		return glm::eulerAngleX<Scalar>(x);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_matrix_angle_Y(const Scalar& y) -> TMat4< Scalar, P >
	{
		return glm::eulerAngleY<Scalar>(y);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_matrix_angle_Z(const Scalar& z) -> TMat4< Scalar, P >
	{
		return glm::eulerAngleZ<Scalar>(z);
	}

	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_matrix_euler(const Scalar& x, const Scalar& y, const Scalar& z) -> TMat4< Scalar, P >
	{
		return glm::eulerAngleXYZ<Scalar>(x, y, z);
	}
	template < typename Scalar, glm::precision P = glm::defaultp >
	inline auto rotate_matrix_euler(const TVec3< Scalar, P >& vec) -> TMat4< Scalar, P >
	{
		return glm::eulerAngleXYZ<Scalar, P>(vec.x, vec.y, vec.z);
	}
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class T >
	inline auto perspective(const T& fovy, const T& aspect, const T& near, const T& far) -> decltype(glm::perspective<T>(fovy, aspect, near, far))
	{
		return glm::perspective<T>(fovy, aspect, near, far);
	}    
	template < class T >
	inline auto perspectiveZO(const T& fovy, const T& aspect, const T& near, const T& far) -> decltype(glm::perspective<T>(fovy, aspect, near, far))
	{
		return glm::perspectiveZO<T>(fovy, aspect, near, far);
	}    
	template < class T >
	inline auto perspectiveNO(const T& fovy, const T& aspect, const T& near, const T& far) -> decltype(glm::perspective<T>(fovy, aspect, near, far))
	{
		return glm::perspectiveNO<T>(fovy, aspect, near, far);
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
	template < class T >
	inline auto look_at_LH(const T& eye, const T& center, const T& up) -> decltype(glm::lookAtLH(eye, center, up))
	{
		return glm::lookAtLH(eye, center, up);
	}
	template < class T >
	inline auto look_at_RH(const T& eye, const T& center, const T& up) -> decltype(glm::lookAtRH(eye, center, up))
	{
		return glm::lookAtRH(eye, center, up);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    * Eigenvalues rotate
    */
    template < typename Scalar = float, glm::precision P = glm::defaultp >
    inline void eigenvalues_rotate(TMat3< Scalar, P >& mat, const Scalar& c, const Scalar& s, size_t i0, size_t j0, size_t i1, size_t j1)
    {
		Scalar a = c * mat[i0][j0] - s * mat[i1][j1];
		Scalar b = s * mat[i0][j0] + c * mat[i1][j1];
        mat[i0][j0] = (Scalar)a;
        mat[i1][j1] = (Scalar)b;
    }
    
    /**
     * Diagonize matrix using Jacobi rotations.
     * @remark Method does not check if matrix is diagonizable.
     * Passing non diagonizable matrix and infinite max_iter (= -1)
     * May result in infinite loop.
     */
    template < typename Scalar = float, glm::precision P = glm::defaultp >
    inline TVec3< Scalar > eigenvalues_jacobi(TMat3< Scalar, P >& mat, size_t max_iter, TMat3< Scalar >& E)
    {
        //return vector
        TVec3< Scalar, P > ret;
        
        // initial eigenvalues
        ret[0] = mat[0][0];
        ret[1] = mat[1][1];
        ret[2] = mat[2][2];
        
        mat[0][0] = 1.0;
        mat[1][1] = 1.0;
        mat[2][2] = 1.0;
        
        E = TMat3< Scalar, P >(1.0);
        
        for(size_t z = 0; z!=max_iter; ++z)
        {
            for (size_t i = 0; i != 3; ++i)
            {
                for (size_t j = i + 1; j < 3; j++)
                {
                    Scalar mii = ret[i];
                    Scalar mjj = ret[j];
                    Scalar mij = mat[i][j];
                    Scalar phi = Scalar(0.5) * std::atan2(Scalar(2.0) * mij, mjj - mii);
                    Scalar c = std::cos(phi);
                    Scalar s = std::sin(phi);
                    Scalar mii1 = c * c * mii - 2 * s * c * mij + s * s * mjj;
                    Scalar mjj1 = s * s * mii + 2 * s * c * mij + c * c * mjj;
                    if (std::abs(mii - mii1) < 0.00001 || (mjj - mjj1) < 0.00001) {
                        ret[i]    = (Scalar)mii1;
                        ret[j]    = (Scalar)mjj1;
                        mat[i][j] = (Scalar)0.0;
                        for (size_t k = 0;     k < i; ++k) eigenvalues_rotate<Scalar>(mat, c, s, k, i, k, j);
                        for (size_t k = i + 1; k < j; ++k) eigenvalues_rotate<Scalar>(mat, c, s, i, k, k, j);
                        for (size_t k = j + 1; k < 3; ++k) eigenvalues_rotate<Scalar>(mat, c, s, i, k, j, k);
                        //rotate eigenvectors
                        for (size_t k = 0; k < 3; k++) eigenvalues_rotate(E, c, s, k, i, k, j);
                    }
                }
            }
        }
        return ret;
    }
}
