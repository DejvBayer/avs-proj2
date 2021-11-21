/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  David Bayer <xbayer09@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    2021/11/16
 **/

#include <iostream>
#include <math.h>
#include <limits>
#include <bitset>
#include <omp.h>

#include "tree_mesh_builder.h"

//  assert(depth_limit == 2^n)
const uint TreeMeshBuilder::depth_limit = 8U;

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{
    for (uint i = mGridSize; i > depth_limit; i >>= 1) {
        double r = mIsoLevel + (sqrt(3.0) / 2.0) * static_cast<double>(i) * mGridResolution;
        sphere_radius[i] = static_cast<float>(r);
    }
}
/*
uint TreeMeshBuilder::decomposeOctree(Vec3_t<uint> pos, uint size, const ParametricScalarField &field)
{
    auto sphere_radius = [this](uint size) -> float
    {
        constexpr float half_sqrt_3 = static_cast<float>(sqrt(3.0) / 2.0);
        return mIsoLevel + half_sqrt_3 * static_cast<float>(size) * mGridResolution;
    };

    auto decompose = [](Vec3_t<uint> p, uint size) -> std::array<Vec3_t<uint>, 8UL>
    {
        uint sh = size >> 1;   //  (size / 2)
        return {{
            { p.x, p.y, p.z },              { p.x + sh, p.y, p.z },
            { p.x, p.y + sh, p.z },         { p.x, p.y, p.z + sh },
            { p.x + sh, p.y + sh, p.z },    { p.x, p.y + sh, p.z + sh },
            { p.x + sh, p.y, p.z + sh },    { p.x + sh, p.y + sh, p.z + sh }
        }};
    };

    auto cube_center_denorm = [this](Vec3_t<uint> p, uint size) -> Vec3_t<float>
    {
        uint sh = size >> 1;  //  size / 2
        return { (p.x + sh) * mGridResolution, (p.y + sh) * mGridResolution, (p.z + sh) * mGridResolution };
    };

    uint totalTriangles = 0;
    
    if (size > depth_limit) {
        float r = sphere_radius(size);
        uint subcube_size = size >> 1;

        for (auto sc : decompose(pos, size)) {
            Vec3_t<float> S = cube_center_denorm(sc, subcube_size);
            
            if (!(evaluateFieldAt(S, field) > r)) {
#               pragma omp task shared(totalTriangles) firstprivate(sc, subcube_size, field)
                {
                    totalTriangles += decomposeOctree(sc, subcube_size, field);
                }
            }
            
        }

#       pragma omp taskwait

    } else {
#       pragma omp parallel for reduction(+: totalTriangles) firstprivate(pos, field)
        for (uint i = 0U; i < depth_limit; i++) {
            for (uint j = 0U; j < depth_limit; j++) {
                for (uint k = 0U; k < depth_limit; k++) {
                    totalTriangles += buildCube(Vec3_t<float>(pos.x + i, pos.y + j, pos.z + k), field);
                }
            }
        }
    }

    return totalTriangles;
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.

    uint totalTriangles;

#   pragma omp parallel
#   pragma omp master
    {
        totalTriangles = decomposeOctree(Vec3_t<uint>(0U, 0U, 0U), mGridSize, field);
    }

    return totalTriangles;
}
*/

/*
uint TreeMeshBuilder::decomposeOctree(Vec3_t<float> pos, uint size, const ParametricScalarField &field)
{
    auto sphere_radius = [this](uint size) -> float
    {
        constexpr float half_sqrt_3 = static_cast<float>(sqrt(3.0) / 2.0);
        return mIsoLevel + half_sqrt_3 * static_cast<float>(size) * mGridResolution;
    };

    auto decompose = [](Vec3_t<float> p, uint size) -> std::array<Vec3_t<float>, 8UL>
    {
        float sh = static_cast<float>(size >> 1);   //  (size / 2)
        return {{
            { p.x, p.y, p.z },              { p.x + sh, p.y, p.z },
            { p.x, p.y + sh, p.z },         { p.x, p.y, p.z + sh },
            { p.x + sh, p.y + sh, p.z },    { p.x, p.y + sh, p.z + sh },
            { p.x + sh, p.y, p.z + sh },    { p.x + sh, p.y + sh, p.z + sh }
        }};
    };

    auto cube_center_denormalized = [this](Vec3_t<float> p, uint size) -> Vec3_t<float>
    {
        float sh = static_cast<float>(size >> 1);  //  size / 2
        return { (p.x + sh) * mGridResolution, (p.y + sh) * mGridResolution, (p.z + sh) * mGridResolution };
    };

    uint totalTriangles = 0;
    
    if (size > depth_limit) {
        float r = sphere_radius(size);
        uint subcube_size = size >> 1;

        for (auto sc : decompose(pos, size)) {
            Vec3_t<float> S = cube_center_denormalized(sc, subcube_size);
            
            if (!(evaluateFieldAt(S, field) > r)) {
#               pragma omp task shared(totalTriangles) firstprivate(sc, subcube_size, field)
                {
                    totalTriangles += decomposeOctree(sc, subcube_size, field);
                }
            }
            
        }

#       pragma omp taskwait

    } else {
#       pragma omp parallel for reduction(+: totalTriangles) firstprivate(pos, field)
        for (uint i = 0U; i < depth_limit; i++) {
            float z = pos.z + static_cast<float>(i);

            for (uint j = 0U; j < depth_limit; j++) {
                float y = pos.y + static_cast<float>(j);

                for (uint k = 0U; k < depth_limit; k++) {
                    float x = pos.x + static_cast<float>(k);
                    totalTriangles += buildCube(Vec3_t<float>(x, y, z), field);
                }
            }
        }
    }

    return totalTriangles;
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.

    uint totalTriangles;

#   pragma omp parallel
#   pragma omp master
    {
        totalTriangles = decomposeOctree(Vec3_t<float>(0.f, 0.f, 0.f), mGridSize, field);
    }

    return totalTriangles;
}
*/

uint TreeMeshBuilder::decomposeOctree(uint index, uint size, const ParametricScalarField &field)
{
    auto decompose = [this](uint index, uint size) -> std::array<uint, 8UL>
    {
        uint x_shift = size >> 1;   //  size / 2
        uint y_shift = x_shift * mGridSize;
        uint z_shift = y_shift * mGridSize;

        return std::array<uint, 8UL> {{
            index,                        index + x_shift,
            index + y_shift,              index + y_shift + x_shift,
            index + z_shift,              index + z_shift + x_shift,
            index + z_shift + y_shift,    index + z_shift + y_shift + x_shift
        }};
    };

    auto cube_center = [this](uint i, uint s) -> Vec3_t<float>
    {
        return {
            ((i % mGridSize) + s >> 1) * mGridResolution,
            (((i / mGridSize) % mGridSize) + s >> 1) * mGridResolution,
            ((i / (mGridSize * mGridSize)) + s >> 1) * mGridResolution
        };
    };

    auto cube_index_to_offset = [this](uint i) -> Vec3_t<float>
    {
        return Vec3_t<float>(i % mGridSize, (i / mGridSize) % mGridSize, i / (mGridSize * mGridSize));
    };

    uint totalTriangles = 0;
    
    if (size > depth_limit) {
        uint subcube_size = size >> 1;  //  size / 2

        for (auto subcube_index : decompose(index, size)) {
            Vec3_t<float> S = cube_center(subcube_index, subcube_size);

            if (!(evaluateFieldAt(S, field) > sphere_radius.at(size))) {
#               pragma omp task shared(totalTriangles) firstprivate(subcube_index, subcube_size, field)
                {
                    totalTriangles += decomposeOctree(subcube_index, subcube_size, field);
                }
            }
        }

#       pragma omp taskwait

    } else {
#       pragma omp parallel for reduction(+: totalTriangles) firstprivate(index, mGridSize, field)
        for (uint i = 0U; i < depth_limit; i++) {
            for (uint j = 0U; j < depth_limit; j++) {
                for (uint k = 0U; k < depth_limit; k++) {
                    uint idx = index + i * mGridSize * mGridSize + j * mGridSize + k;
                    totalTriangles += buildCube(cube_index_to_offset(idx), field);
                }
            }
        }

    }

    return totalTriangles;
}

uint TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    uint totalTriangles;

#   pragma omp parallel
#   pragma omp master
    {
        totalTriangles = decomposeOctree(0U, mGridSize, field);
    }

    return totalTriangles;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    // NOTE: This method is called from "buildCube(...)"!

    // 1. Store pointer to and number of 3D points in the field
    //    (to avoid "data()" and "size()" call in the loop).
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    // 2. Find minimum square distance from points "pos" to any point in the
    //    field.
#   pragma omp simd reduction(min: value) simdlen(64)
    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        // Comparing squares instead of real distance to avoid unnecessary
        // "sqrt"s in the loop.
        value = value > distanceSquared ? distanceSquared : value;
    }

    // 3. Finally take square root of the minimal square distance to get the real distance
    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    // NOTE: This method is called from "buildCube(...)"!

    // Store generated triangle into vector (array) of generated triangles.
    // The pointer to data in this array is return by "getTrianglesArray(...)" call
    // after "marchCubes(...)" call ends.

#   pragma omp critical
    {
        mTriangles.push_back(triangle);
    }
}
