#ifndef EE_MATHVECTOR3_H
#define EE_MATHVECTOR3_H

#include <eepp/config.hpp>

namespace EE { namespace Math {

/** @brief Utility template class for manipulating 3-dimensional vectors */
template <typename T> class Vector3 {
  public:
	T x;
	T y;
	T z;

	/** Default constructor. Creates Vector3(0,0,0) */
	Vector3() : x( 0 ), y( 0 ), z( 0 ) {}

	/** Creates a vector from its coordinates */
	Vector3( T X, T Y, T Z ) : x( X ), y( Y ), z( Z ) {}
};

template <typename T> Vector3<T> operator-( const Vector3<T>& V ) {
	return Vector3<T>( -V.x, -V.y, -V.z );
}

template <typename T> Vector3<T>& operator+=( Vector3<T>& V1, const Vector3<T>& V2 ) {
	V1.x += V2.x;
	V1.y += V2.y;
	V1.z += V2.z;
	return V1;
}

template <typename T> Vector3<T>& operator-=( Vector3<T>& V1, const Vector3<T>& V2 ) {
	V1.x -= V2.x;
	V1.y -= V2.y;
	V1.z -= V2.z;
	return V1;
}

template <typename T> Vector3<T> operator+( const Vector3<T>& V1, const Vector3<T>& V2 ) {
	return Vector3<T>( V1.x + V2.x, V1.y + V2.y, V1.z + V2.z );
}

template <typename T> Vector3<T> operator-( const Vector3<T>& V1, const Vector3<T>& V2 ) {
	return Vector3<T>( V1.x - V2.x, V1.y - V2.y, V1.z - V2.z );
}

template <typename T> Vector3<T> operator*( const Vector3<T>& V, T X ) {
	return Vector3<T>( V.x * X, V.y * X, V.z * X );
}

template <typename T> Vector3<T> operator*( T X, const Vector3<T>& V ) {
	return Vector3<T>( V.x * X, V.y * X, V.z * X );
}

template <typename T> Vector3<T>& operator*=( Vector3<T>& V, T X ) {
	V.x *= X;
	V.y *= X;
	V.z *= X;
	return V;
}

template <typename T> Vector3<T> operator/( const Vector3<T>& V, T X ) {
	return Vector3<T>( V.x / X, V.y / X, V.z / X );
}

template <typename T> Vector3<T>& operator/=( Vector3<T>& V, T X ) {
	V.x /= X;
	V.y /= X;
	V.z /= X;
	return V;
}

template <typename T> bool operator==( const Vector3<T>& V1, const Vector3<T>& V2 ) {
	return ( V1.x == V2.x ) && ( V1.y == V2.y ) && ( V1.z == V2.z );
}

template <typename T> bool operator!=( const Vector3<T>& V1, const Vector3<T>& V2 ) {
	return ( V1.x != V2.x ) || ( V1.y != V2.y ) || ( V1.z != V2.z );
}

// Define the most common types
typedef Vector3<int> Vector3i;
typedef Vector3<Float> Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector3<float> Vector3ff;

}} // namespace EE::Math

#endif
