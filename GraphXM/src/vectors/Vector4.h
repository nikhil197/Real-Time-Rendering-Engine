#pragma once

namespace gm
{
	namespace vector
	{
		struct Vector2;

		struct Vector3;

		struct Vector4
		{
		public:
			// W - Component of the vector
			float w;

			// X - Component of the vector
			float x;

			// Y - Component of the vector
			float y;

			// Z - Component of the vector
			float z;

			// 4D Zero Vector constant (0, 0, 0, 0)
			static const Vector4 ZeroVector;

			// 4D Unit Vector constant (1, 1, 1, 1)
			static const Vector4 UnitVector;

			// Number of components in the vector
			static const int Components;

			// Constructors
			explicit Vector4();

			explicit Vector4(float value);

			Vector4(float w, float x, float y, float z);

			Vector4(const Vector2& Vec, float y, float z);

			Vector4(float w, const Vector2& Vec, float z);

			Vector4(float w, float x, const Vector2& Vec);

			Vector4(const Vector3& Vec, float z);

			Vector4(float w, const Vector3& Vec);

			// Operators
			Vector4& operator=(const Vector4& OtherVector);

			bool operator==(const Vector4& OtherVector) const;

			bool operator!=(const Vector4& OtherVector) const;

			// Arithmetic Operators
			const Vector4 operator+(const Vector4& OtherVector) const;

			const Vector4 operator-(const Vector4& OtherVector) const;

			const Vector4 operator*(const Vector4& OtherVector) const;

			const Vector4 operator/(const Vector4& OtherVector) const;

			const Vector4 operator+(float Value) const;
			
			const Vector4 operator-(float Value) const;

			const Vector4 operator*(float Value) const;

			const Vector4 operator/(float Value) const;

			// Assignment Operators
			Vector4& operator+=(const Vector4& OtherVector);

			Vector4& operator-=(const Vector4& OtherVector);

			Vector4& operator*=(const Vector4& OtherVector);

			Vector4& operator/=(const Vector4& OtherVector);

			Vector4& operator+=(float Value);

			Vector4& operator-=(float Value);

			Vector4& operator*=(float Value);

			Vector4& operator/=(float Value);

			/* Returns the magnitude of the vector */
			float Magnitude() const;

			/* Returns the square of magnitude of the vector */
			float MagnitudeSquare() const;

			/* Returns whether vector is zero */
			bool IsZero() const;

			/*********** Static Member functions ***********/
			/* Dot product of the two vectors */
			static float DotProduct(const Vector4& V1, const Vector4& V2);

			/* Cross product of the two vectors */
			static const Vector4 CrossProduct(const Vector4& V1, const Vector4& V2);

			/* Return the distance between two vectors */
			static float Distance(const Vector4& V1, const Vector4& V2);

			/* Return the square of the distance between two vectors */
			static float DistanceSquared(const Vector4& V1, const Vector4& V2);

		};
	}
}