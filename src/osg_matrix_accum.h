#pragma once

// 顶点世界化：把 osg::MatrixTransform 累积出来，落到顶点上。
//
// osgb 里的变换有两种写法，转 3D Tiles 时必须一视同仁：
//   * 顶点已经是绝对坐标（ContextCapture 的叶子瓦片、被 osgconv 烘焙过的产物）
//   * 顶点是局部坐标，真实位置靠一个 osg::MatrixTransform 表达（ContextCapture 的
//     块根与中间层、osgconv 对含 PagedLOD 场景的包裹）
//
// 只读顶点、不累积变换，第二类会整体错位一个矩阵平移量。

#include <cmath>

#include <osg/Array>
#include <osg/Geometry>
#include <osg/Matrix>
#include <osg/Vec3>

namespace osgb {

inline bool matrix_is_identity(const osg::Matrixd& m)
{
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c) {
            const double expect = (r == c) ? 1.0 : 0.0;
            if (std::fabs(m(r, c) - expect) > 1e-12)
                return false;
        }
    return true;
}

// osg 是行向量约定：世界坐标 = 顶点 * 矩阵
inline void apply_matrix_to_geometry(osg::Geometry& geometry, const osg::Matrixd& m)
{
    if (matrix_is_identity(m))
        return;

    osg::Array* array = geometry.getVertexArray();
    if (array == nullptr)
        return;

    if (auto* v3f = dynamic_cast<osg::Vec3Array*>(array)) {
        for (auto& v : *v3f) {
            const osg::Vec3d p = osg::Vec3d(v.x(), v.y(), v.z()) * m;
            v.set(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
        }
        v3f->dirty();
    }
    else if (auto* v3d = dynamic_cast<osg::Vec3dArray*>(array)) {
        for (auto& v : *v3d)
            v = v * m;
        v3d->dirty();
    }
}

} // namespace osgb
