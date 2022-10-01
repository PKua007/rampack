/*
XenoCollide Collision Detection and Physics Library
Copyright (c) 2007-2014 Gary Snethen http://xenocollide.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Adapted by Michal Ciesla and Piotr Kubala
 */


#pragma once

#include "../Vector.h"
#include "CollideGeometry.h"


//////////////////////////////////////////////////////////////////////////////
// This file (and its associated *.cpp file) contain the implementation of
// the XenoCollide algorithm.

class Collide{

private:
	static inline void Swap(Vector<3>& a, Vector<3>& b);


public:
//////////////////////////////////////////////////////////////////////////////
// Intersect() is the simplest XenoCollide routine.  It returns true if two
// CollideGeometry objects overlap, or false if they do not.

	static bool Intersect(CollideGeometry& p1, const Matrix<3,3>& m1, const Vector<3>& t1, CollideGeometry& p2, const Matrix<3,3>& m2, const Vector<3>& t2, double boundaryTolerance);

//////////////////////////////////////////////////////////////////////////////
// TransformSupportVert() finds the support point for a rotated and/or
// translated CollideGeometry.

	static Vector<3> TransformSupportVert( CollideGeometry& p, const Matrix<3,3>& m, const Vector<3>& t, const Vector<3>& n );
};
