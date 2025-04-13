//***************************************************************************************
// GeometryGenerator.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "GeometryGenerator.h"
#include "DirectXMath.h"
#include <iostream>
#include <algorithm>
#include <string>

using namespace DirectX;

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32 numSubdivisions)
{
    MeshData meshData;

    //
	// Create the vertices.
	//

	Vertex v[24];

	float w2 = 0.5f*width;
	float h2 = 0.5f*height;
	float d2 = 0.5f*depth;
    
	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);
 
	//
	// Create the indices.
	//

	uint32 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7]  = 5; i[8]  = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] =  9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices32.assign(&i[0], &i[36]);

    // Put a cap on the number of subdivisions.
    numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

    for(uint32 i = 0; i < numSubdivisions; ++i)
        Subdivide(meshData);

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
{
    MeshData meshData;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back( topVertex );

	float phiStep   = XM_PI/stackCount;
	float thetaStep = 2.0f*XM_PI/sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for(uint32 i = 1; i <= stackCount-1; ++i)
	{
		float phi = i*phiStep;

		// Vertices of ring.
        for(uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j*thetaStep;

			Vertex v;

			// spherical to cartesian
			v.Position.x = radius*sinf(phi)*cosf(theta);
			v.Position.y = radius*cosf(phi);
			v.Position.z = radius*sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius*sinf(phi)*sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius*sinf(phi)*cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

			v.TexC.x = theta / XM_2PI;
			v.TexC.y = phi / XM_PI;

			meshData.Vertices.push_back( v );
		}
	}

	meshData.Vertices.push_back( bottomVertex );

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

    for(uint32 i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices32.push_back(0);
		meshData.Indices32.push_back(i+1);
		meshData.Indices32.push_back(i);
	}
	
	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
    uint32 baseIndex = 1;
    uint32 ringVertexCount = sliceCount + 1;
	for(uint32 i = 0; i < stackCount-2; ++i)
	{
		for(uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(baseIndex + i*ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i*ringVertexCount + j+1);
			meshData.Indices32.push_back(baseIndex + (i+1)*ringVertexCount + j);

			meshData.Indices32.push_back(baseIndex + (i+1)*ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i*ringVertexCount + j+1);
			meshData.Indices32.push_back(baseIndex + (i+1)*ringVertexCount + j+1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32 southPoleIndex = (uint32)meshData.Vertices.size()-1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;
	
	for(uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(southPoleIndex);
		meshData.Indices32.push_back(baseIndex+i);
		meshData.Indices32.push_back(baseIndex+i+1);
	}

    return meshData;
}
 
void GeometryGenerator::Subdivide(MeshData& meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;


	meshData.Vertices.resize(0);
	meshData.Indices32.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	uint32 numTris = (uint32)inputCopy.Indices32.size()/3;
	for(uint32 i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.Vertices[ inputCopy.Indices32[i*3+0] ];
		Vertex v1 = inputCopy.Vertices[ inputCopy.Indices32[i*3+1] ];
		Vertex v2 = inputCopy.Vertices[ inputCopy.Indices32[i*3+2] ];

		//
		// Generate the midpoints.
		//

        Vertex m0 = MidPoint(v0, v1);
        Vertex m1 = MidPoint(v1, v2);
        Vertex m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5
 
		meshData.Indices32.push_back(i*6+0);
		meshData.Indices32.push_back(i*6+3);
		meshData.Indices32.push_back(i*6+5);

		meshData.Indices32.push_back(i*6+3);
		meshData.Indices32.push_back(i*6+4);
		meshData.Indices32.push_back(i*6+5);

		meshData.Indices32.push_back(i*6+5);
		meshData.Indices32.push_back(i*6+4);
		meshData.Indices32.push_back(i*6+2);

		meshData.Indices32.push_back(i*6+3);
		meshData.Indices32.push_back(i*6+1);
		meshData.Indices32.push_back(i*6+4);
	}
}

GeometryGenerator::Vertex GeometryGenerator::MidPoint(const Vertex& v0, const Vertex& v1)
{
    XMVECTOR p0 = XMLoadFloat3(&v0.Position);
    XMVECTOR p1 = XMLoadFloat3(&v1.Position);

    XMVECTOR n0 = XMLoadFloat3(&v0.Normal);
    XMVECTOR n1 = XMLoadFloat3(&v1.Normal);

    XMVECTOR tan0 = XMLoadFloat3(&v0.TangentU);
    XMVECTOR tan1 = XMLoadFloat3(&v1.TangentU);

    XMVECTOR tex0 = XMLoadFloat2(&v0.TexC);
    XMVECTOR tex1 = XMLoadFloat2(&v1.TexC);

    // Compute the midpoints of all the attributes.  Vectors need to be normalized
    // since linear interpolating can make them not unit length.  
    XMVECTOR pos = 0.5f*(p0 + p1);
    XMVECTOR normal = XMVector3Normalize(0.5f*(n0 + n1));
    XMVECTOR tangent = XMVector3Normalize(0.5f*(tan0+tan1));
    XMVECTOR tex = 0.5f*(tex0 + tex1);

    Vertex v;
    XMStoreFloat3(&v.Position, pos);
    XMStoreFloat3(&v.Normal, normal);
    XMStoreFloat3(&v.TangentU, tangent);
    XMStoreFloat2(&v.TexC, tex);

    return v;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGeosphere(float radius, uint32 numSubdivisions)
{
    MeshData meshData;

	// Put a cap on the number of subdivisions.
    numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f; 
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] = 
	{
		XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),  
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),    
		XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X), 
		XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),    
		XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f), 
		XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
	};

    uint32 k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,    
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,    
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7 
	};

    meshData.Vertices.resize(12);
    meshData.Indices32.assign(&k[0], &k[60]);

	for(uint32 i = 0; i < 12; ++i)
		meshData.Vertices[i].Position = pos[i];

	for(uint32 i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	// Project vertices onto sphere and scale.
	for(uint32 i = 0; i < meshData.Vertices.size(); ++i)
	{
		// Project onto unit sphere.
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.Vertices[i].Position));

		// Project onto sphere.
		XMVECTOR p = radius*n;

		XMStoreFloat3(&meshData.Vertices[i].Position, p);
		XMStoreFloat3(&meshData.Vertices[i].Normal, n);

		// Derive texture coordinates from spherical coordinates.
        float theta = atan2f(meshData.Vertices[i].Position.z, meshData.Vertices[i].Position.x);

        // Put in [0, 2pi].
        if(theta < 0.0f)
            theta += XM_2PI;

		float phi = acosf(meshData.Vertices[i].Position.y / radius);

		meshData.Vertices[i].TexC.x = theta/XM_2PI;
		meshData.Vertices[i].TexC.y = phi/XM_PI;

		// Partial derivative of P with respect to theta
		meshData.Vertices[i].TangentU.x = -radius*sinf(phi)*sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +radius*sinf(phi)*cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
		XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));
	}

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
    MeshData meshData;

	//
	// Build Stacks.
	// 

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	uint32 ringCount = stackCount+1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for(uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f*height + i*stackHeight;
		float r = bottomRadius + i*radiusStep;

		// vertices of ring
		float dTheta = 2.0f*XM_PI/sliceCount;
		for(uint32 j = 0; j <= sliceCount; ++j)
		{
			Vertex vertex;

			float c = cosf(j*dTheta);
			float s = sinf(j*dTheta);

			vertex.Position = XMFLOAT3(r*c, y, r*s);

			vertex.TexC.x = (float)j/sliceCount;
			vertex.TexC.y = 1.0f - (float)i/stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius-topRadius;
			XMFLOAT3 bitangent(dr*c, -height, dr*s);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.Normal, N);

			meshData.Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	uint32 ringVertexCount = sliceCount+1;

	// Compute indices for each stack.
	for(uint32 i = 0; i < stackCount; ++i)
	{
		for(uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(i*ringVertexCount + j);
			meshData.Indices32.push_back((i+1)*ringVertexCount + j);
			meshData.Indices32.push_back((i+1)*ringVertexCount + j+1);

			meshData.Indices32.push_back(i*ringVertexCount + j);
			meshData.Indices32.push_back((i+1)*ringVertexCount + j+1);
			meshData.Indices32.push_back(i*ringVertexCount + j+1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

    return meshData;
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
											uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
	uint32 baseIndex = (uint32)meshData.Vertices.size();

	float y = 0.5f*height;
	float dTheta = 2.0f*XM_PI/sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for(uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius*cosf(i*dTheta);
		float z = topRadius*sinf(i*dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x/height + 0.5f;
		float v = z/height + 0.5f;

		meshData.Vertices.push_back( Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v) );
	}

	// Cap center vertex.
	meshData.Vertices.push_back( Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f) );

	// Index of center vertex.
	uint32 centerIndex = (uint32)meshData.Vertices.size()-1;

	for(uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i+1);
		meshData.Indices32.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
											   uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
	// 
	// Build bottom cap.
	//

	uint32 baseIndex = (uint32)meshData.Vertices.size();
	float y = -0.5f*height;

	// vertices of ring
	float dTheta = 2.0f * XM_PI / sliceCount;
	for(uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius*cosf(i * dTheta);
		float z = bottomRadius*sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x/height + 0.5f;
		float v = z/height + 0.5f;

		meshData.Vertices.push_back( Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v) );
	}

	// Cap center vertex.
	meshData.Vertices.push_back( Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f) );

	// Cache the index of center vertex.
	uint32 centerIndex = (uint32)meshData.Vertices.size()-1;

	for(uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i+1);
	}
}

XMFLOAT3 GeometryGenerator::NormalizeF3(const XMFLOAT3& f3)
{
	XMVECTOR v = XMVectorSet(f3.x, f3.y, f3.z, 0);
	v = XMVector3Normalize(v);

	return XMFLOAT3(v.m128_f32[0], v.m128_f32[1], v.m128_f32[2]);
}

DirectX::XMFLOAT3 GeometryGenerator::MeanF3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
	XMFLOAT3 mean = XMFLOAT3((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);

	return mean;
}

GeometryGenerator::MeshData GeometryGenerator::CreateCone(float radius, float height, int segments)
{
	// Generate vertices
	MeshData meshData;
	meshData.Vertices.resize(segments + 2);
	float radialIncrement = 2 * XM_PI / segments;

	// Add the top vertex
	meshData.Vertices[0].Position = XMFLOAT3(0, height, 0);
	meshData.Vertices[0].Normal = XMFLOAT3(0, 1, 0);
	meshData.Vertices[0].TexC = XMFLOAT2(0.5,0.5);

	// Add the base vertices
	for (int i = 0; i < segments; i++)
	{
		float x = radius * cos(radialIncrement * i);
		float z = radius * sin(radialIncrement * i);
		meshData.Vertices[i + 1].Position = XMFLOAT3(x, 0, z);
		XMFLOAT3 top = meshData.Vertices[0].Position;
		XMFLOAT3 pos = meshData.Vertices[i + 1].Position;
		XMFLOAT3 a = NormalizeF3(XMFLOAT3(top.x - pos.x , top.y - pos.y, top.z - pos.z));
		XMFLOAT3 b = NormalizeF3(XMFLOAT3(-pos.x, -pos.y, -pos.z));
		XMFLOAT3 invNormal = NormalizeF3(MeanF3(a, b));
		meshData.Vertices[i + 1].Normal = XMFLOAT3(-invNormal.x, -invNormal.y, -invNormal.z);
		meshData.Vertices[i + 1].TexC = XMFLOAT2((x + 1) / 2, (z + 1) / 2);
	}

	// Add the bottom vertex
	meshData.Vertices.back().Position = XMFLOAT3(0, 0, 0);
	meshData.Vertices.back().Normal = XMFLOAT3(0, -1, 0);
	meshData.Vertices.back().TexC = XMFLOAT2(0.5, 0.5);
	// Generate indices for the top cap
	for (int i = 0; i < segments ; i++)
	{
	   
	   meshData.Indices32.push_back(i + 1);
		meshData.Indices32.push_back(0);
	   meshData.Indices32.push_back((i + 1) % segments + 1);
	}

	// Generate indices for the bottom cap
	for (int i = 0; i < segments ; i++)
	{
		
		meshData.Indices32.push_back(meshData.Vertices.size() - 1);
		meshData.Indices32.push_back(i + 1);
		meshData.Indices32.push_back((i + 1) % segments + 1);
	}
	


   return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreatePyramid(float depth, float width, float height)
{
	// Generate vertices
	MeshData meshData;
	meshData.Vertices.resize(6);

	float halfWidth = width / 2;
	float halfDepth = depth / 2;

	// Add the top vertex
	meshData.Vertices[0].Position = XMFLOAT3(0, height, 0);
	meshData.Vertices[0].Normal = XMFLOAT3(0, 1, 0);
	meshData.Vertices[0].TexC = XMFLOAT2(0.5,0.5);

	// Add the base vertices
	for (int y = 0; y < 2; y++)
	{
		for(int x = 0; x < 2; x++)
		{
			int current = (y * 2) + x + 1;
			meshData.Vertices[current].Position = XMFLOAT3(x ? -halfWidth : halfWidth, 0, y ? -halfDepth : halfDepth);
			XMFLOAT3 top = meshData.Vertices[0].Position;
			XMFLOAT3 pos = meshData.Vertices[current].Position;
			XMFLOAT3 a = NormalizeF3(XMFLOAT3(top.x - pos.x , top.y - pos.y, top.z - pos.z));
			XMFLOAT3 b = NormalizeF3(XMFLOAT3(-pos.x, -pos.y, -pos.z));
			XMFLOAT3 invNormal = NormalizeF3(MeanF3(a, b));
			meshData.Vertices[current].Normal = XMFLOAT3(-invNormal.x, -invNormal.y, -invNormal.z);
			meshData.Vertices[current].TexC = XMFLOAT2(x,y);
		}
	}

	// Add the bottom vertex
	meshData.Vertices.back().Position = XMFLOAT3(0, 0, 0);
	meshData.Vertices.back().Normal = XMFLOAT3(0, -1, 0);
	meshData.Vertices.back().TexC = XMFLOAT2(0.5,0.5);
	
	// Generate indices for the top cap
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(2);

	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(1);

	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(3);

	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(4);

	// Generate indices for the bottom cap
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(5);
	
	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(5);
	
	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(5);
	
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(5);
	
	return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateOctahedron(float depth, float width, float high, float low)
{
	// Generate vertices
	MeshData meshData;
	meshData.Vertices.resize(6);

	float halfWidth = width / 2;
	float halfDepth = depth / 2;

	// Add the top vertex
	meshData.Vertices[0].Position = XMFLOAT3(0, high, 0);
	meshData.Vertices[0].Normal = XMFLOAT3(0, 1, 0);
	meshData.Vertices[0].TexC = XMFLOAT2(0.5,0.5);
	XMFLOAT3 bot = XMFLOAT3(0, low, 0);
	
	// Add the base vertices
	for (int y = 0; y < 2; y++)
	{
		for(int x = 0; x < 2; x++)
		{
			int current = (y * 2) + x + 1;
			meshData.Vertices[current].Position = XMFLOAT3(x ? -halfWidth : halfWidth, 0, y ? -halfDepth : halfDepth);
			XMFLOAT3 top = meshData.Vertices[0].Position;
			XMFLOAT3 pos = meshData.Vertices[current].Position;
			XMFLOAT3 a = NormalizeF3(XMFLOAT3(top.x - pos.x , top.y - pos.y, top.z - pos.z));
			XMFLOAT3 b = NormalizeF3(XMFLOAT3(-pos.x, -pos.y, -pos.z));
			XMFLOAT3 invNormal = NormalizeF3(MeanF3(a, b));
			meshData.Vertices[current].Normal = XMFLOAT3(-invNormal.x, -invNormal.y, -invNormal.z);
			meshData.Vertices[current].TexC = XMFLOAT2(x,y);
		}
	}

	// Add the bottom vertex
	meshData.Vertices.back().Position = XMFLOAT3(0, low, 0);
	meshData.Vertices.back().Normal = XMFLOAT3(0, -1, 0);
	meshData.Vertices.back().TexC = XMFLOAT2(0.5,0.5);

	// Generate indices for the top cap
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(2);

	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(1);

	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(3);

	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(4);

	// Generate indices for the bottom cap
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(5);
	
	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(5);
	
	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(5);
	
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(5);

	return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateWedge(float width, float height, float depth)
{
	MeshData meshData;
	meshData.Vertices.resize(18);
	float halfDepth = depth / 2;
	float halfHeight = height / 2;
	float halfWidth = width / 2;

	// Generate verts for quads
	meshData.Vertices[0].Position = XMFLOAT3(-halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[1].Position = XMFLOAT3(-halfWidth, halfHeight, -halfDepth);
	meshData.Vertices[2].Position = XMFLOAT3(-halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[3].Position = XMFLOAT3(-halfWidth, halfHeight, halfDepth);
	
	meshData.Vertices[4].Position = XMFLOAT3(halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[5].Position = XMFLOAT3(-halfWidth, halfHeight, halfDepth);
	meshData.Vertices[6].Position = XMFLOAT3(halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[7].Position = XMFLOAT3(-halfWidth, halfHeight, -halfDepth);

	meshData.Vertices[8].Position = XMFLOAT3(-halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[9].Position = XMFLOAT3(-halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[10].Position = XMFLOAT3(halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[11].Position = XMFLOAT3(halfWidth, -halfHeight, halfDepth);
	
	meshData.Vertices[0].TexC = XMFLOAT2(0,0);
	meshData.Vertices[1].TexC = XMFLOAT2(0,1);
	meshData.Vertices[2].TexC = XMFLOAT2(1,0);
	meshData.Vertices[3].TexC = XMFLOAT2(1,1);
	
	meshData.Vertices[4].TexC = XMFLOAT2(1,0);
	meshData.Vertices[5].TexC = XMFLOAT2(0,1);
	meshData.Vertices[6].TexC = XMFLOAT2(0,0);
	meshData.Vertices[7].TexC = XMFLOAT2(1,1);

	meshData.Vertices[8].TexC = XMFLOAT2(0,0);
	meshData.Vertices[9].TexC = XMFLOAT2(0,1);
	meshData.Vertices[10].TexC = XMFLOAT2(1,0);
	meshData.Vertices[11].TexC = XMFLOAT2(1,1);

	XMVECTOR aCross = XMVector3Normalize(
		XMVector3Cross(
			XMLoadFloat3(&meshData.Vertices[2].Position) - XMLoadFloat3(&meshData.Vertices[0].Position),
			XMLoadFloat3(&meshData.Vertices[1].Position) - XMLoadFloat3(&meshData.Vertices[0].Position)
		)
	);
	
	XMVECTOR bCross = XMVector3Normalize(
		XMVector3Cross(
			XMLoadFloat3(&meshData.Vertices[6].Position) - XMLoadFloat3(&meshData.Vertices[4].Position),
			XMLoadFloat3(&meshData.Vertices[5].Position) - XMLoadFloat3(&meshData.Vertices[4].Position)
		)
	);

	meshData.Vertices[0].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	meshData.Vertices[1].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	meshData.Vertices[2].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	meshData.Vertices[3].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	
	meshData.Vertices[4].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);
	meshData.Vertices[5].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);
	meshData.Vertices[6].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);
	meshData.Vertices[7].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);

	meshData.Vertices[8].Normal = XMFLOAT3(0,-1,0);
	meshData.Vertices[9].Normal = XMFLOAT3(0,-1,0);
	meshData.Vertices[10].Normal = XMFLOAT3(0,-1,0);
	meshData.Vertices[11].Normal = XMFLOAT3(0,-1,0);

	// Generate verts for tris
	meshData.Vertices[12].Position = XMFLOAT3(-halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[13].Position = XMFLOAT3(-halfWidth, halfHeight, -halfDepth);
	meshData.Vertices[14].Position = XMFLOAT3(halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[15].Position = XMFLOAT3(-halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[16].Position = XMFLOAT3(-halfWidth, halfHeight, halfDepth);
	meshData.Vertices[17].Position = XMFLOAT3(halfWidth, -halfHeight, halfDepth);

	meshData.Vertices[12].TexC = XMFLOAT2(0,0);
	meshData.Vertices[13].TexC = XMFLOAT2(0,1);
	meshData.Vertices[14].TexC = XMFLOAT2(1,0);
	meshData.Vertices[15].TexC = XMFLOAT2(0,0);
	meshData.Vertices[16].TexC = XMFLOAT2(0,1);
	meshData.Vertices[17].TexC = XMFLOAT2(1,0);

	meshData.Vertices[12].Normal = XMFLOAT3(0,0, -1);
	meshData.Vertices[13].Normal = XMFLOAT3(0,0, -1);
	meshData.Vertices[14].Normal = XMFLOAT3(0,0, -1);
	meshData.Vertices[15].Normal = XMFLOAT3(0,0, 1);
	meshData.Vertices[16].Normal = XMFLOAT3(0,0, 1);
	meshData.Vertices[17].Normal = XMFLOAT3(0,0, 1);

	// Generate indices Quads
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(1);

	meshData.Indices32.push_back(7);
	meshData.Indices32.push_back(6);
	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(7);
	meshData.Indices32.push_back(5);
	meshData.Indices32.push_back(6);

	meshData.Indices32.push_back(9);
	meshData.Indices32.push_back(8);
	meshData.Indices32.push_back(10);
	meshData.Indices32.push_back(10);
	meshData.Indices32.push_back(11);
	meshData.Indices32.push_back(9);
	
	// Generate indices Tris
	meshData.Indices32.push_back(12);
	meshData.Indices32.push_back(13);
	meshData.Indices32.push_back(14);
	meshData.Indices32.push_back(15);
	meshData.Indices32.push_back(17);
	meshData.Indices32.push_back(16);
	
	return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateTriangularPrism(float width, float height, float depth)
{
	MeshData meshData;
	meshData.Vertices.resize(18);
	float halfDepth = depth / 2;
	float halfHeight = height / 2;
	float halfWidth = width / 2;

	// Generate verts for quads
	meshData.Vertices[0].Position = XMFLOAT3(-halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[1].Position = XMFLOAT3(0, halfHeight, -halfDepth);
	meshData.Vertices[2].Position = XMFLOAT3(-halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[3].Position = XMFLOAT3(0, halfHeight, halfDepth);
	
	meshData.Vertices[4].Position = XMFLOAT3(halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[5].Position = XMFLOAT3(0, halfHeight, halfDepth);
	meshData.Vertices[6].Position = XMFLOAT3(halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[7].Position = XMFLOAT3(0, halfHeight, -halfDepth);

	meshData.Vertices[8].Position = XMFLOAT3(-halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[9].Position = XMFLOAT3(-halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[10].Position = XMFLOAT3(halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[11].Position = XMFLOAT3(halfWidth, -halfHeight, halfDepth);
	
	meshData.Vertices[0].TexC = XMFLOAT2(0,0);
	meshData.Vertices[1].TexC = XMFLOAT2(0,1);
	meshData.Vertices[2].TexC = XMFLOAT2(1,0);
	meshData.Vertices[3].TexC = XMFLOAT2(1,1);
	
	meshData.Vertices[4].TexC = XMFLOAT2(1,0);
	meshData.Vertices[5].TexC = XMFLOAT2(0,1);
	meshData.Vertices[6].TexC = XMFLOAT2(0,0);
	meshData.Vertices[7].TexC = XMFLOAT2(1,1);

	meshData.Vertices[8].TexC = XMFLOAT2(0,0);
	meshData.Vertices[9].TexC = XMFLOAT2(0,1);
	meshData.Vertices[10].TexC = XMFLOAT2(1,0);
	meshData.Vertices[11].TexC = XMFLOAT2(1,1);

	XMVECTOR aCross = XMVector3Normalize(
		XMVector3Cross(
			XMLoadFloat3(&meshData.Vertices[2].Position) - XMLoadFloat3(&meshData.Vertices[0].Position),
			XMLoadFloat3(&meshData.Vertices[1].Position) - XMLoadFloat3(&meshData.Vertices[0].Position)
		)
	);
	
	XMVECTOR bCross = XMVector3Normalize(
		XMVector3Cross(
			XMLoadFloat3(&meshData.Vertices[6].Position) - XMLoadFloat3(&meshData.Vertices[4].Position),
			XMLoadFloat3(&meshData.Vertices[5].Position) - XMLoadFloat3(&meshData.Vertices[4].Position)
		)
	);

	meshData.Vertices[0].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	meshData.Vertices[1].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	meshData.Vertices[2].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	meshData.Vertices[3].Normal = XMFLOAT3(aCross.m128_f32[0], aCross.m128_f32[1], aCross.m128_f32[2]);
	
	meshData.Vertices[4].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);
	meshData.Vertices[5].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);
	meshData.Vertices[6].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);
	meshData.Vertices[7].Normal = XMFLOAT3(bCross.m128_f32[0], bCross.m128_f32[1], bCross.m128_f32[2]);

	meshData.Vertices[8].Normal = XMFLOAT3(0,-1,0);
	meshData.Vertices[9].Normal = XMFLOAT3(0,-1,0);
	meshData.Vertices[10].Normal = XMFLOAT3(0,-1,0);
	meshData.Vertices[11].Normal = XMFLOAT3(0,-1,0);

	// Generate verts for tris
	meshData.Vertices[12].Position = XMFLOAT3(-halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[13].Position = XMFLOAT3(0, halfHeight, -halfDepth);
	meshData.Vertices[14].Position = XMFLOAT3(halfWidth, -halfHeight, -halfDepth);
	meshData.Vertices[15].Position = XMFLOAT3(-halfWidth, -halfHeight, halfDepth);
	meshData.Vertices[16].Position = XMFLOAT3(0, halfHeight, halfDepth);
	meshData.Vertices[17].Position = XMFLOAT3(halfWidth, -halfHeight, halfDepth);

	meshData.Vertices[12].TexC = XMFLOAT2(0,0);
	meshData.Vertices[13].TexC = XMFLOAT2(0.5,1);
	meshData.Vertices[14].TexC = XMFLOAT2(1,0);
	meshData.Vertices[15].TexC = XMFLOAT2(0,0);
	meshData.Vertices[16].TexC = XMFLOAT2(0.5,1);
	meshData.Vertices[17].TexC = XMFLOAT2(1,0);

	meshData.Vertices[12].Normal = XMFLOAT3(0,0, -1);
	meshData.Vertices[13].Normal = XMFLOAT3(0,0, -1);
	meshData.Vertices[14].Normal = XMFLOAT3(0,0, -1);
	meshData.Vertices[15].Normal = XMFLOAT3(0,0, 1);
	meshData.Vertices[16].Normal = XMFLOAT3(0,0, 1);
	meshData.Vertices[17].Normal = XMFLOAT3(0,0, 1);

	// Generate indices Quads
	meshData.Indices32.push_back(1);
	meshData.Indices32.push_back(0);
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(2);
	meshData.Indices32.push_back(3);
	meshData.Indices32.push_back(1);

	meshData.Indices32.push_back(7);
	meshData.Indices32.push_back(6);
	meshData.Indices32.push_back(4);
	meshData.Indices32.push_back(7);
	meshData.Indices32.push_back(5);
	meshData.Indices32.push_back(6);

	meshData.Indices32.push_back(9);
	meshData.Indices32.push_back(8);
	meshData.Indices32.push_back(10);
	meshData.Indices32.push_back(10);
	meshData.Indices32.push_back(11);
	meshData.Indices32.push_back(9);
	
	// Generate indices Tris
	meshData.Indices32.push_back(12);
	meshData.Indices32.push_back(13);
	meshData.Indices32.push_back(14);
	meshData.Indices32.push_back(15);
	meshData.Indices32.push_back(17);
	meshData.Indices32.push_back(16);
	
	return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateTorus(float radius, float thicknessH, float thicknessW, int segmentsDiam, int segmentsTang,  bool offAxis, float uvXScale, float uvYScale, bool tcWrap)
{
	MeshData meshData;
	int ringVertCount = segmentsTang + 1;
	int ringCount = segmentsDiam + 1;
	int vertsNum = ringCount * ringVertCount;
	if(!tcWrap) meshData.Vertices.resize(vertsNum * 4);
	else meshData.Vertices.resize(vertsNum);

	float diamIncrement = 2 * XM_PI / segmentsDiam;
	float tangIncrement = 2 * XM_PI / segmentsTang;

	
	for(int d = 0; d < ringCount; d++)
	{
		XMMATRIX diamRot;
		if(offAxis)
			diamRot = XMMatrixRotationY(diamIncrement * d + diamIncrement / 2);
		else
			diamRot = XMMatrixRotationY(diamIncrement * d);
		
		XMMATRIX ringScale = XMMatrixScaling(thicknessW, thicknessH, 1);
		
		if(!tcWrap)
		{
			// TODO FIX THIS
			for(int t = 0; t < ringVertCount; t++)
			{
				{
					int current = d * ringVertCount * 4 + t * 4;
                    float x1, x2, y1, y2;
                    // Generate verts
                    if(offAxis)
                    {
                    	x1 = cos(tangIncrement * t + tangIncrement / 2);
                    	x2 = cos(tangIncrement * t + 1 + tangIncrement / 2);
                    	y1 = sin(tangIncrement * t + tangIncrement / 2);
                    	y2 = sin(tangIncrement * t + 1 + tangIncrement / 2);
                    }
                    else
                    {
                    	x1 = cos(tangIncrement * t);
                    	x2 = cos(tangIncrement * t + 1);
                    	y1 = sin(tangIncrement * t);
                    	y2 = sin(tangIncrement * t + 1);
                    }
					
                    XMVECTOR defaultRingNormal = XMVectorSet((x1 + x2) / 2,(y1 + y2) / 2,0,0);
                    XMVECTOR normal = XMVector3Transform(defaultRingNormal, diamRot);
                    XMStoreFloat3(&meshData.Vertices[current].Normal, normal);
					XMStoreFloat3(&meshData.Vertices[current + 1].Normal, normal);
					XMStoreFloat3(&meshData.Vertices[current + 2].Normal, normal);
					XMStoreFloat3(&meshData.Vertices[current + 3].Normal, normal);
					
					XMVECTOR vertexRingNormalPos1 = XMVectorSet(x1, y1,0,0);
					XMVECTOR vertexRingPos1 = XMVector3Transform(vertexRingNormalPos1, ringScale) + XMVectorSet(radius,0,0,0);
                    XMVECTOR vertexPos1 = XMVector3Transform(vertexRingPos1, diamRot);
                    meshData.Vertices[current].Position = XMFLOAT3(vertexPos1.m128_f32[0], vertexPos1.m128_f32[1], vertexPos1.m128_f32[2]);
                    meshData.Vertices[current].TexC = XMFLOAT2(0, 0);
					
					XMVECTOR vertexRingNormalPos2 = XMVectorSet(x2, y2,0,0);
					XMVECTOR vertexRingPos2 = XMVector3Transform(vertexRingNormalPos2, ringScale) + XMVectorSet(radius,0,0,0);
					XMVECTOR vertexPos2 = XMVector3Transform(vertexRingPos2, diamRot);
					meshData.Vertices[current + 1].Position = XMFLOAT3(vertexPos2.m128_f32[0], vertexPos2.m128_f32[1], vertexPos2.m128_f32[2]);
					meshData.Vertices[current + 1].TexC = XMFLOAT2(1 * uvXScale, 0);
					
					XMVECTOR vertexRingNormalPos3 = XMVectorSet(x1, y1,0,0);
					XMVECTOR vertexRingPos3 = XMVector3Transform(vertexRingNormalPos3, ringScale) + XMVectorSet(radius,0,0,0);
					XMVECTOR vertexPos3 = XMVector3Transform(vertexRingPos3, XMMatrixRotationY(diamIncrement * (d + 1)));
					meshData.Vertices[current + 2].Position = XMFLOAT3(vertexPos3.m128_f32[0], vertexPos3.m128_f32[1], vertexPos3.m128_f32[2]);
					meshData.Vertices[current + 2].TexC = XMFLOAT2(0, 1 * uvYScale);
					
					XMVECTOR vertexRingNormalPos4 = XMVectorSet(x2, y2,0,0);
					XMVECTOR vertexRingPos4 = XMVector3Transform(vertexRingNormalPos4, ringScale) + XMVectorSet(radius,0,0,0);
					XMVECTOR vertexPos4 = XMVector3Transform(vertexRingPos4, XMMatrixRotationY(diamIncrement * (d + 1)));
					meshData.Vertices[current + 3].Position = XMFLOAT3(vertexPos4.m128_f32[0], vertexPos4.m128_f32[1], vertexPos4.m128_f32[2]);
					meshData.Vertices[current + 3].TexC = XMFLOAT2(1 * uvXScale, 1 * uvYScale);

					meshData.Indices32.push_back(current);
					meshData.Indices32.push_back(current + 1);
					meshData.Indices32.push_back(current + 2);
					meshData.Indices32.push_back(current + 3);
					meshData.Indices32.push_back(current + 4);
					meshData.Indices32.push_back(current + 5);
                    
				}

			}
		}
		else
		{
	        for(int t = 0; t < ringVertCount; t++)
            {
                int current = d * ringVertCount + t;
                float x, y;
                // Generate verts
                if(offAxis)
                {
                    x = cos(tangIncrement * t + tangIncrement / 2);
                    y = sin(tangIncrement * t + tangIncrement / 2);
                }
                else
                {
                    x = cos(tangIncrement * t);
                    y = sin(tangIncrement * t);
                }
                XMVECTOR defaultRingNormal = XMVectorSet(x,y,0,0);
                XMVECTOR normal = XMVector3Transform(defaultRingNormal, diamRot);
                meshData.Vertices[current].Normal = XMFLOAT3(normal.m128_f32[0], normal.m128_f32[1], normal.m128_f32[2]);
                
                XMVECTOR defaultRingPos = XMVector3Transform(defaultRingNormal, ringScale) + XMVectorSet(radius,0,0,0);
                XMVECTOR position = XMVector3Transform(defaultRingPos, diamRot);
                meshData.Vertices[current].Position = XMFLOAT3(position.m128_f32[0], position.m128_f32[1], position.m128_f32[2]);
                meshData.Vertices[current].TexC = XMFLOAT2((float)d / (float)(segmentsDiam) * uvXScale, (float)t / (float)(segmentsTang) * uvYScale);
                
    
                // Generate indices
                meshData.Indices32.push_back(current);
                meshData.Indices32.push_back((current + ringVertCount) % vertsNum);
                meshData.Indices32.push_back((current + 1) % vertsNum);
                meshData.Indices32.push_back((current + 1) % vertsNum);
                meshData.Indices32.push_back((current + ringVertCount) % vertsNum);
                meshData.Indices32.push_back((current + 1 + ringVertCount) % vertsNum);
            }
		}

	}
	
	return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGrid(float width, float depth, uint32 m, uint32 n)
{
    MeshData meshData;

	uint32 vertexCount = m*n;
	uint32 faceCount   = (m-1)*(n-1)*2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*depth;

	float dx = width / (n-1);
	float dz = depth / (m-1);

	float du = 1.0f / (n-1);
	float dv = 1.0f / (m-1);

	meshData.Vertices.resize(vertexCount);
	for(uint32 i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for(uint32 j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i*n+j].Position = XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[i*n+j].Normal   = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i*n+j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i*n+j].TexC.x = j*du;
			meshData.Vertices[i*n+j].TexC.y = i*dv;
		}
	}
 
    //
	// Create the indices.
	//

	meshData.Indices32.resize(faceCount*3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32 k = 0;
	for(uint32 i = 0; i < m-1; ++i)
	{
		for(uint32 j = 0; j < n-1; ++j)
		{
			meshData.Indices32[k]   = i*n+j;
			meshData.Indices32[k+1] = i*n+j+1;
			meshData.Indices32[k+2] = (i+1)*n+j;

			meshData.Indices32[k+3] = (i+1)*n+j;
			meshData.Indices32[k+4] = i*n+j+1;
			meshData.Indices32[k+5] = (i+1)*n+j+1;

			k += 6; // next quad
		}
	}

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
    MeshData meshData;

	meshData.Vertices.resize(4);
	meshData.Indices32.resize(6);

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = Vertex(
        x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = Vertex(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = Vertex(
		x+w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = Vertex(
		x+w, y-h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices32[0] = 0;
	meshData.Indices32[1] = 1;
	meshData.Indices32[2] = 2;

	meshData.Indices32[3] = 0;
	meshData.Indices32[4] = 2;
	meshData.Indices32[5] = 3;

    return meshData;
}
