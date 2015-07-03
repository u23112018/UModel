#ifndef __UNMESHTYPES_H__
#define __UNMESHTYPES_H__

/*-----------------------------------------------------------------------------
	4-byte packed normal vector
-----------------------------------------------------------------------------*/

// used in Bioshock and in UE3
struct FPackedNormal
{
	unsigned				Data;

	friend FArchive& operator<<(FArchive &Ar, FPackedNormal &N)
	{
		return Ar << N.Data;
	}

	operator FVector() const
	{
		// "x / 127.5 - 1" comes from Common.usf, TangentBias() macro
		FVector r;
		r.X = ( Data        & 0xFF) / 127.5f - 1;
		r.Y = ((Data >> 8 ) & 0xFF) / 127.5f - 1;
		r.Z = ((Data >> 16) & 0xFF) / 127.5f - 1;
		return r;
	}

	FPackedNormal &operator=(const FVector &V)
	{
		Data = int((V.X + 1) * 255)
			+ (int((V.Y + 1) * 255) << 8)
			+ (int((V.Z + 1) * 255) << 16);
		return *this;
	}

	float GetW() const
	{
		return (Data >> 24) / 127.5 - 1;
	}
};


/*-----------------------------------------------------------------------------
	Different compressed quaternion types
-----------------------------------------------------------------------------*/

// really have data, when W*W == -0.0f, and sqrt(wSq) was returned -INF
#define RESTORE_QUAT_W(r)									\
		float wSq = 1.0f - (r.X*r.X + r.Y*r.Y + r.Z*r.Z);	\
		r.W = (wSq > 0) ? sqrt(wSq) : 0;


struct FQuatFloat96NoW
{
	float			X, Y, Z;

	inline operator FQuat() const
	{
		FQuat r;
		r.X = X;
		r.Y = Y;
		r.Z = Z;
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFloat96NoW &Q)
	{
		return Ar << Q.X << Q.Y << Q.Z;
	}
};

SIMPLE_TYPE(FQuatFloat96NoW, float)


// normalized quaternion with 3 16-bit fixed point fields
struct FQuatFixed48NoW
{
	word			X, Y, Z;				// unsigned short, corresponds to (float+1)*32767

	operator FQuat() const
	{
		FQuat r;
		r.X = (X - 32767) / 32767.0f;
		r.Y = (Y - 32767) / 32767.0f;
		r.Z = (Z - 32767) / 32767.0f;
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFixed48NoW &Q)
	{
		return Ar << Q.X << Q.Y << Q.Z;
	}
};

SIMPLE_TYPE(FQuatFixed48NoW, word)


struct FVectorFixed48
{
	word			X, Y, Z;

	operator FVector() const
	{
		FVector r;
		static const float scale = 128.0f / 32767.0f;
		r.X = (X - 32767) * scale;
		r.Y = (Y - 32767) * scale;
		r.Z = (Z - 32767) * scale;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorFixed48 &V)
	{
		return Ar << V.X << V.Y << V.Z;
	}
};

SIMPLE_TYPE(FVectorFixed48, word)


// normalized quaternion with 11/11/10-bit fixed point fields
struct FQuatFixed32NoW
{
	unsigned		Z:10, Y:11, X:11;

	operator FQuat() const
	{
		FQuat r;
		r.X = X / 1023.0f - 1.0f;
		r.Y = Y / 1023.0f - 1.0f;
		r.Z = Z / 511.0f  - 1.0f;
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFixed32NoW &Q)
	{
		return Ar << GET_DWORD(Q);
	}
};

SIMPLE_TYPE(FQuatFixed32NoW, unsigned)


struct FQuatIntervalFixed32NoW
{
	unsigned		Z:10, Y:11, X:11;

	FQuat ToQuat(const FVector &Mins, const FVector &Ranges) const
	{
		FQuat r;
		r.X = (X / 1023.0f - 1.0f) * Ranges.X + Mins.X;
		r.Y = (Y / 1023.0f - 1.0f) * Ranges.Y + Mins.Y;
		r.Z = (Z / 511.0f  - 1.0f) * Ranges.Z + Mins.Z;
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatIntervalFixed32NoW &Q)
	{
		return Ar << GET_DWORD(Q);
	}
};

SIMPLE_TYPE(FQuatIntervalFixed32NoW, unsigned)


struct FVectorIntervalFixed32
{
	unsigned		X:10, Y:11, Z:11;

	FVector ToVector(const FVector &Mins, const FVector &Ranges) const
	{
		FVector r;
		r.X = (X / 511.0f  - 1.0f) * Ranges.X + Mins.X;
		r.Y = (Y / 1023.0f - 1.0f) * Ranges.Y + Mins.Y;
		r.Z = (Z / 1023.0f - 1.0f) * Ranges.Z + Mins.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorIntervalFixed32 &V)
	{
		return Ar << GET_DWORD(V);
	}
};

SIMPLE_TYPE(FVectorIntervalFixed32, unsigned)


struct FVectorIntervalFixed32Bat4
{
	unsigned		X:10, Y:10, Z:10;

	FVector ToVector(const FVector &Mins, const FVector &Ranges) const
	{
		FVector r;
		r.X = X / 1023.0f * Ranges.X + Mins.X;
		r.Y = Y / 1023.0f * Ranges.Y + Mins.Y;
		r.Z = Z / 1023.0f * Ranges.Z + Mins.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorIntervalFixed32Bat4 &V)
	{
		return Ar << GET_DWORD(V);
	}
};

SIMPLE_TYPE(FVectorIntervalFixed32Bat4, unsigned)


// Similar to FVectorIntervalFixed32 used in animation, but has different X/Y/Z bit count.
// Used for GPU skin with compressed position. It looks like original UE3 has permitted use
// of this data type for XBox360 and PS3 builds only.
struct FVectorIntervalFixed32GPU
{
	int				X:11, Y:11, Z:10;

	FVector ToVector(const FVector &Mins, const FVector &Ranges) const
	{
		FVector r;
		r.X = (X / 1023.0f) * Ranges.X + Mins.X;
		r.Y = (Y / 1023.0f) * Ranges.Y + Mins.Y;
		r.Z = (Z / 511.0f)  * Ranges.Z + Mins.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorIntervalFixed32GPU &V)
	{
		return Ar << GET_DWORD(V);
	}
};

SIMPLE_TYPE(FVectorIntervalFixed32GPU, unsigned)


struct FVectorIntervalFixed48Bio
{
	word			X, Y, Z;

	FVector ToVector(const FVector &Mins, const FVector &Ranges) const
	{
		FVector r;
		r.X = (X / 65535.0f) * Ranges.X + Mins.X;
		r.Y = (Y / 65535.0f) * Ranges.Y + Mins.Y;
		r.Z = (Z / 65535.0f) * Ranges.Z + Mins.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorIntervalFixed48Bio &V)
	{
		return Ar << V.X << V.Y << V.Z;
	}
};

SIMPLE_TYPE(FVectorIntervalFixed48Bio, word)


struct FVectorIntervalFixed64
{
	short			X, Y, Z, W;

	FVector ToVector(const FVector &Mins, const FVector &Ranges) const
	{
		FVector r;
		r.X = (X / 32767.0f) * Ranges.X + Mins.X;
		r.Y = (Y / 32767.0f) * Ranges.Y + Mins.Y;
		r.Z = (Z / 32767.0f) * Ranges.Z + Mins.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorIntervalFixed64 &V)
	{
		return Ar << V.X << V.Y << V.Z << V.W;
	}
};

SIMPLE_TYPE(FVectorIntervalFixed64, word)


struct FQuatFloat32NoW
{
	unsigned		data;

	operator FQuat() const
	{
		FQuat r;

		int _X = data >> 21;			// 11 bits
		int _Y = (data >> 10) & 0x7FF;	// 11 bits
		int _Z = data & 0x3FF;			// 10 bits

		*(unsigned*)&r.X = ((((_X >> 7) & 7) + 123) << 23) | ((_X & 0x7F | 32 * (_X & 0xFFFFFC00)) << 16);
		*(unsigned*)&r.Y = ((((_Y >> 7) & 7) + 123) << 23) | ((_Y & 0x7F | 32 * (_Y & 0xFFFFFC00)) << 16);
		*(unsigned*)&r.Z = ((((_Z >> 6) & 7) + 123) << 23) | ((_Z & 0x3F | 32 * (_Z & 0xFFFFFE00)) << 17);

		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFloat32NoW &Q)
	{
		return Ar << Q.data;
	}
};

SIMPLE_TYPE(FQuatFloat32NoW, unsigned)


#if BATMAN

// This is a variant of FQuatFixed48NoW developed for Batman: Arkham Asylum. It's destination
// is to store quaternion with a better precision than FQuatFixed48NoW, but there was a logical
// error (?): FQuatFixed48NoW's precision is 1/32768, but FQuatFixed48Max's precision is only 1/23170.
// Found the name of this compression scheme in SCE Edge library overview: "smallest 3 compression".
struct FQuatFixed48Max
{
	word			data[3];				// layout: V2[15] : V1[15] : V0[15] : S[2]

	operator FQuat() const
	{
		unsigned tmp;
		tmp = (data[1] << 16) | data[0];
		int S = tmp & 3;								// bits [0..1]
		int L = (tmp >> 2) & 0x7FFF;					// bits [2..16]
		tmp = (data[2] << 16) | data[1];
		int M = (tmp >> 1) & 0x7FFF;					// bits [17..31]
		int H = (tmp >> 16) & 0x7FFF;					// bits [32..46]
		// bit 47 is unused ...

		static const float shift = 0.70710678118f;		// sqrt(0.5)
		static const float scale = 1.41421356237f;		// sqrt(0.5)*2
		float l = (L - 0.5f) / 32767 * scale - shift;
		float m = (M - 0.5f) / 32767 * scale - shift;
		float h = (H - 0.5f) / 32767 * scale - shift;
		float a = sqrt(1.0f - (l*l + m*m + h*h));
		// "l", "m", "h" are serialized values, in a range -shift .. +shift
		// if we will remove one of maximal values ("a"), other values will be smaller
		// that "a"; if "a" is 1, all other values are 0; when "a" becomes smaller,
		// other values may grow; maximal value of "other" equals to "a" when
		// 2 quaternion components equals to "a" and 2 other is 0; so, maximal
		// stored value can be computed from equation "a*a + a*a + 0 + 0 = 1", so
		// maximal value is sqrt(1/2) ...
		// "a" is computed value, up to 1

		FQuat r;
		switch (S)			// choose where to place "a"
		{
		case 0:
			r.Set(a, l, m, h);
			break;
		case 1:
			r.Set(l, a, m, h);
			break;
		case 2:
			r.Set(l, m, a, h);
			break;
		default: // 3
			r.Set(l, m, h, a);
		}
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFixed48Max &Q)
	{
		return Ar << Q.data[0] << Q.data[1] << Q.data[2];
	}
};

SIMPLE_TYPE(FQuatFixed48Max, word)

#endif // BATMAN


#if MASSEFF

// similar to FQuatFixed48Max
struct FQuatBioFixed48
{
	word			data[3];

	operator FQuat() const
	{
		FQuat r;
		static const float shift = 0.70710678118f;		// sqrt(0.5)
		static const float scale = 1.41421356237f;		// sqrt(0.5)*2
		float x = (data[0] & 0x7FFF) / 32767.0f * scale - shift;
		float y = (data[1] & 0x7FFF) / 32767.0f * scale - shift;
		float z = (data[2] & 0x7FFF) / 32767.0f * scale - shift;
		float w = 1.0f - (x*x + y*y + z*z);
		if (w >= 0.0f)
			w = sqrt(w);
		else
			w = 0.0f;
		int s = ((data[0] >> 14) & 2) | ((data[1] >> 15) & 1);
		switch (s)
		{
		case 0:
			r.Set(w, x, y, z);
			break;
		case 1:
			r.Set(x, w, y, z);
			break;
		case 2:
			r.Set(x, y, w, z);
			break;
		default: // 3
			r.Set(x, y, z, w);
		}
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatBioFixed48 &Q)
	{
		return Ar << Q.data[0] << Q.data[1] << Q.data[2];
	}
};

SIMPLE_TYPE(FQuatBioFixed48, word)

#endif // MASSEFF


#if TRANSFORMERS

// mix of FQuatFixed48NoW and FQuatIntervalFixed32NoW
struct FQuatIntervalFixed48NoW_Trans
{
	word			X, Y, Z;

	FQuat ToQuat(const FVector &Mins, const FVector &Ranges) const
	{
		FQuat r;
		r.X = (X - 32767) / 32767.0f * Ranges.X + Mins.X;
		r.Y = (Y - 32767) / 32767.0f * Ranges.Y + Mins.Y;
		r.Z = (Z - 32767) / 32767.0f * Ranges.Z + Mins.Z;
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatIntervalFixed48NoW_Trans &Q)
	{
		return Ar << Q.X << Q.Y << Q.Z;
	}
};

SIMPLE_TYPE(FQuatIntervalFixed48NoW_Trans, word)


struct FPackedVector_Trans
{
	unsigned		Z:11, Y:10, X:11;

	FVector ToVector(const FVector &Mins, const FVector &Ranges) const
	{
		FVector r;
		r.X = X / 2047.0f * Ranges.X + Mins.X;
		r.Y = Y / 1023.0f * Ranges.Y + Mins.Y;
		r.Z = Z / 2047.0f * Ranges.Z + Mins.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FPackedVector_Trans &Q)
	{
		return Ar << GET_DWORD(Q);
	}
};

SIMPLE_TYPE(FPackedVector_Trans, unsigned)

#endif // TRANSFORMERS


#if ARGONAUTS

// mix of FQuatFixed48NoW and FQuatIntervalFixed32NoW
struct FQuatIntervalFixed48NoW_Argo
{
	word			X, Y, Z;

	FQuat ToQuat(const FVector &Mins, const FVector &Ranges) const
	{
		FQuat r;
		r.X = X / 65535.0f * Ranges.X + Mins.X;
		r.Y = Y / 65535.0f * Ranges.Y + Mins.Y;
		r.Z = Z / 65535.0f * Ranges.Z + Mins.Z;
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatIntervalFixed48NoW_Argo &Q)
	{
		return Ar << Q.X << Q.Y << Q.Z;
	}
};

SIMPLE_TYPE(FQuatIntervalFixed48NoW_Argo, word)


struct FQuatFixed64NoW_Argo
{
	uint64			V;

	operator FQuat() const
	{
		FQuat r;
		r.X =  int(V >> 43)             * (1.0f / 0x0FFFFF) - 1.0f;	// upper 64-43=21 bits
		r.Y = (int(V >> 22) & 0x1FFFFF) * (1.0f / 0x0FFFFF) - 1.0f;	// mid 64-21-22=21 bits
		r.Z =  int(V        & 0x3FFFFF) * (1.0f / 0x1FFFFF) - 1.0f;	// lower 22 bits
		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFixed64NoW_Argo &Q)
	{
		return Ar << Q.V;
	}
};

SIMPLE_TYPE(FQuatFixed64NoW_Argo, int64)


struct FQuatFloat48NoW_Argo
{
	word			X, Y, Z;

	operator FQuat() const
	{
		FQuat r;

		*(unsigned*)&r.X = ((((X >> 10) & 0x1F) + 111) << 23) | ((X & 0x3FF | 8 * (X & 0x8000)) << 13);
		*(unsigned*)&r.Y = ((((Y >> 10) & 0x1F) + 111) << 23) | ((Y & 0x3FF | 8 * (Y & 0x8000)) << 13);
		*(unsigned*)&r.Z = ((((Z >> 10) & 0x1F) + 111) << 23) | ((Z & 0x3FF | 8 * (Z & 0x8000)) << 13);

		RESTORE_QUAT_W(r);
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatFloat48NoW_Argo &Q)
	{
		return Ar << Q.X << Q.Y << Q.Z;
	}
};

SIMPLE_TYPE(FQuatFloat48NoW_Argo, word)


#endif // ARGONAUTS


#if BORDERLANDS

struct FQuatDelta48NoW
{
	word			X, Y, Z;

	FQuat ToQuat(const FVector &Mins, const FVector &Ranges, const FQuat &Base) const
	{
		FQuat r;
		int Sign = Z >> 15;
		r.X = X / 65535.0f * Ranges.X + Mins.X + Base.X;
		r.Y = Y / 65535.0f * Ranges.Y + Mins.Y + Base.Y;
		r.Z = (Z & 0x7FFF) / 32767.0f * Ranges.Z + Mins.Z + Base.Z;
		RESTORE_QUAT_W(r);
		if (Sign) r.W = -r.W;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatDelta48NoW &Q)
	{
		return Ar << Q.X << Q.Y << Q.Z;
	}
};

struct FVectorDelta48NoW
{
	word			X, Y, Z;

	FVector ToVector(const FVector &Mins, const FVector &Ranges, const FVector &Base) const
	{
		FVector r;
		r.X = X / 65535.0f * Ranges.X + Mins.X + Base.X;
		r.Y = Y / 65535.0f * Ranges.Y + Mins.Y + Base.Y;
		r.Z = Z / 65535.0f * Ranges.Z + Mins.Z + Base.Z;
		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FVectorDelta48NoW &V)
	{
		return Ar << V.X << V.Y << V.Z;
	}
};


// Borderlands 2

struct FQuatPolarEncoded32
{
	unsigned		data;

	operator FQuat() const
	{
		FQuat r;
		float angle1, angle2;

		float D0 = (data & 0x7FF) / 2047.0f;
		r.W = 1.0f - (D0 * D0);

		int D1 = (data >> 11) & 0x3FFFF;
		int D2 = appFloor(sqrt((float)D1));

		if ( (D2+1)*(D2+1) <= D1 )
			D2++;

		static const float AngleScale = 1.5717963f; // not M_PI/2
		if ( (D2+1)*(D2+1) - D2*D2 == 1 )	//?? strange code: (a+b)^2 = a^2 + 2ab + b^2; (D2+1)^2 = D2^2+2*D2+1
			angle1 = 0;
		else
			angle1 = AngleScale * (float)(D1 - D2*D2) / (float)( (D2+1)*(D2+1) - D2*D2 - 1 ); //?? strange code, see above
		angle2 = AngleScale * D2 / 511.0f;

		float _X = cos(angle1);
		float _Y = sqrt(1.0f - _X*_X);
		float _Z = cos(angle2);

		float scale1 = sqrt(1.0f - _Z*_Z);
		_X *= scale1;
		_Y *= scale1;

		float scale = sqrt(1.0f - r.W*r.W) / sqrt(_X*_X + _Y*_Y + _Z*_Z);
		r.X = _X * scale;
		r.Y = _Y * scale;
		r.Z = _Z * scale;

		if (data & 0x80000000) r.X *= -1;
		if (data & 0x40000000) r.Y *= -1;
		if (data & 0x20000000) r.Z *= -1;

		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatPolarEncoded32 &Q)
	{
		return Ar << Q.data;
	}
};

struct FQuatPolarEncoded48
{
	byte			data[6];

	operator FQuat() const
	{
		FQuat r;
		float angle1, angle2;

		float D0 = ( *(word*)data ) / 65535.0f;
		r.W = 1.0f - (D0 * D0);

		int data2 = *(int*) (data+2);
		int D1 = data2 & 0x1FFFFFFF;
		int D2 = appFloor(sqrt((float)D1));

		if ( (D2+1)*(D2+1) <= D1 )
			D2++;

		static const float AngleScale = 1.5717963f; // not M_PI/2
		if ( (D2+1)*(D2+1) - D2*D2 == 1 )	//?? strange code: (a+b)^2 = a^2 + 2ab + b^2; (D2+1)^2 = D2^2+2*D2+1
			angle1 = 0;
		else
			angle1 = AngleScale * (float)(D1 - D2*D2) / (float)( (D2+1)*(D2+1) - D2*D2 - 1 ); //?? strange code, see above
		angle2 = AngleScale * D2 / 16383.0f;

		float _X = cos(angle1);
		float _Y = sqrt(1.0f - _X*_X);
		float _Z = cos(angle2);

		float scale1 = sqrt(1.0f - _Z*_Z);
		_X *= scale1;
		_Y *= scale1;

		float scale = sqrt(1.0f - r.W*r.W) / sqrt(_X*_X + _Y*_Y + _Z*_Z);
		r.X = _X * scale;
		r.Y = _Y * scale;
		r.Z = _Z * scale;

		if (data2 & 0x80000000) r.X *= -1;
		if (data2 & 0x40000000) r.Y *= -1;
		if (data2 & 0x20000000) r.Z *= -1;

		return r;
	}

	friend FArchive& operator<<(FArchive &Ar, FQuatPolarEncoded48 &Q)
	{
		Ar.ByteOrderSerialize(Q.data, 6);
		return Ar;
	}
};

#endif // BORDERLANDS


#endif // __UNMESHTYPES_H__
