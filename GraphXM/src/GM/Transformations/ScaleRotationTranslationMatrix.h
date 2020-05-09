#pragma once

#include "Matrices/Matrix4.h"
#include "MathUtility.h"

namespace GM
{
	class ScaleRotationTranslationMatrix
		: public Matrix4
	{
	public:
		/* Construct a combined scale, rotation and translaiton matrix based on the given values */
		ScaleRotationTranslationMatrix(const Vector3& Scale, const Vector3& Rot, const Vector3& Origin)
		{
			Make(*this, Scale, Rot, Origin);
		}

		/* Construct a combined scale, rotation (about given axis) and translaiton matrix based on the given values */
		ScaleRotationTranslationMatrix(const Vector3& Scale, float Angle, const Vector3& RotAxis, const Vector3& Origin)
		{
			Make(*this, Scale, Angle, RotAxis, Origin);
		}

		/* Returns a combined matrix for given scale, rotation and translaiton values */
		static Matrix4 Make(const Vector3& Scale, const Vector3& Rot, const Vector3& Origin)
		{
			return ScaleRotationTranslationMatrix(Scale, Rot, Origin);
		}

		/* Returns a combined matrix for given scale, rotation (about a given axis) and translaiton values */
		static Matrix4 Make(const Vector3& Scale, float Angle, const Vector3& RotAxis, const Vector3& Origin)
		{
			return ScaleRotationTranslationMatrix(Scale, Angle, RotAxis, Origin);
		}

		/* Convert a given matrix into a combined scale, rotation and translaiton matrix based on the given values */
		static void Make(Matrix4& Mat, const Vector3& Scale, const Vector3& Rot, const Vector3& Origin)
		{
			float cosA = (float)Utility::Cos(Rot.x);
			float cosB = (float)Utility::Cos(Rot.y);
			float cosC = (float)Utility::Cos(Rot.z);

			float sinA = (float)Utility::Sin(Rot.x);
			float sinB = (float)Utility::Sin(Rot.y);
			float sinC = (float)Utility::Sin(Rot.z);

			Mat(0, 0) = (cosB * cosC) * Scale.x;
			Mat(0, 1) = (sinA * sinB * cosC - cosA * sinC) * Scale.y;
			Mat(0, 2) = (cosA * sinB * cosC + sinA * sinC) * Scale.z;
			Mat(0, 3) = Origin.x;

			Mat(1, 0) = (cosB * sinC) * Scale.x;
			Mat(1, 1) = (sinA * sinB * sinC + cosA * cosC) * Scale.y;
			Mat(1, 2) = (cosA * sinB * sinC - sinA * cosC) * Scale.z;
			Mat(1, 3) = Origin.y;

			Mat(2, 0) = (-sinB) * Scale.x;
			Mat(2, 1) = (sinA * cosB) * Scale.y;
			Mat(2, 2) = (cosA * cosB) * Scale.z;
			Mat(2, 3) = Origin.z;

			Mat(3, 0) = 0.0f;
			Mat(3, 1) = 0.0f;
			Mat(3, 2) = 0.0f;
			Mat(3, 3) = 1.0f;
		}

		/* Convert a given matrix into a combined scale, rotation (about a given axis) and translaiton matrix based on the given values */
		static void Make(Matrix4& Mat, const Vector3& Scale, float Angle, const Vector3& RotAxis, const Vector3& Origin)
		{
			// Get the normalized Axis
			Vector3 nAxis = RotAxis.Normal();

			float cos = (float)Utility::Cos(Angle);
			float sin = (float)Utility::Sin(Angle);
			float oneMinusCos = 1 - cos;

			float xy = nAxis.x * nAxis.y;
			float yz = nAxis.y * nAxis.z;
			float zx = nAxis.z * nAxis.x;

			Mat(0, 0) = (cos + (nAxis.x * nAxis.x) * oneMinusCos) * Scale.x;
			Mat(0, 1) = (xy * oneMinusCos - nAxis.z * sin) * Scale.y;
			Mat(0, 2) = (zx * oneMinusCos + nAxis.y * sin) * Scale.z;
			Mat(0, 3) = Origin.x;
						
			Mat(1, 0) = (xy * oneMinusCos + nAxis.z * sin) * Scale.x;
			Mat(1, 1) = (cos + (nAxis.y * nAxis.y) * oneMinusCos) * Scale.y;
			Mat(1, 2) = (yz * oneMinusCos - nAxis.x * sin) * Scale.z;
			Mat(1, 3) = Origin.y;
						
			Mat(2, 0) = (zx * oneMinusCos - nAxis.y * sin) * Scale.x;
			Mat(2, 1) = (yz * oneMinusCos + nAxis.x * sin) * Scale.y;
			Mat(2, 2) = (cos + (nAxis.z * nAxis.z) * oneMinusCos) * Scale.z;
			Mat(2, 3) = Origin.z;

			Mat(3, 0) = 0.0f;
			Mat(3, 1) = 0.0f;
			Mat(3, 2) = 0.0f;
			Mat(3, 3) = 1.0f;
		}
	};
}