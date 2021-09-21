#ifndef __H__STL_WRITER
#define __H__STL_WRITER

#include <vector>
#include <fstream>

namespace stl_writer
{

    void STLExport(std::ofstream &,
                   const std::vector<float> &,
                   const std::vector<float> &,
                   const std::vector<unsigned int> &);
    void STLFacetOut(std::ofstream &,
                     const float, const float, const float,
                     const float, const float, const float,
                     const float, const float, const float,
                     const float, const float, const float);

    // Save as ASCII STL format
    void WriteStlFile(const char *filename,
                      const std::vector<float> &coords,
                      const std::vector<float> &normals,
                      const std::vector<unsigned int> &tris)
    {
        std::ofstream stlout(filename, std::ofstream::out);
        STLExport(stlout, coords, normals, tris);
        stlout.close();
    }

    void STLExport(std::ofstream &stlout,
                   const std::vector<float> &coords,
                   const std::vector<float> &normals,
                   const std::vector<unsigned int> &tris)
    {
        stlout << "solid STLExport" << std::endl;

        const size_t numTris = tris.size() / 3;
        for (size_t itri = 0; itri < numTris; ++itri)
        {
            float C[3][3] = {};
            for (size_t icorner = 0; icorner < 3; ++icorner)
            {
                const float *c = &coords[3 * tris[3 * itri + icorner]];
                C[icorner][0] = c[0]; // x
                C[icorner][1] = c[1]; // y
                C[icorner][2] = c[2]; // z
            }

            const float *n = &normals[3 * itri];

            STLFacetOut(stlout,
                        n[0], n[1], n[2],
                        C[0][0], C[0][1], C[0][2],
                        C[1][0], C[1][1], C[1][2],
                        C[2][0], C[2][1], C[2][2]);
        }

        stlout << "endsolid STLExport" << std::endl;
    }

    void STLFacetOut(std::ofstream &stlout,
                     const float nx, const float ny, const float nz,
                     const float c0x, const float c0y, const float c0z,
                     const float c1x, const float c1y, const float c1z,
                     const float c2x, const float c2y, const float c2z)
    {

        // Export an STL facet to an output stream

        stlout << "facet normal " << nx << " " << ny << " " << nz << std::endl;
        stlout << "   outer loop" << std::endl;
        stlout << "      vertex " << c0x << " " << c0y << " " << c0z << std::endl;
        stlout << "      vertex " << c1x << " " << c1y << " " << c1z << std::endl;
        stlout << "      vertex " << c2x << " " << c2y << " " << c2z << std::endl;
        stlout << "   endloop" << std::endl;
        stlout << "endfacet" << std::endl;
    }

} // end of namespace stl_writer

#endif //__H__STL_WRITER
