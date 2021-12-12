#ifndef TERRAIN_H
#define TERRAIN_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"            // glm::vec*, mat*, and basic glm functions
#include "glm/gtx/transform.hpp"  // glm::translate, scale, rotate
#include "glm/gtc/type_ptr.hpp"   // glm::value_ptr
#include <vector>

#include "openglshape.h"

class Terrain {
public:
    Terrain();

    std::vector<std::vector<glm::vec3>> genControlPoints(int startRow, int startCol);
    std::vector<std::vector<glm::vec3>> genControlPoints1(int startRow, int startCol, std::vector<glm::vec3> top, std::vector<glm::vec3> secondTop);
    glm::vec3 getNormal(int row, int col, std::vector<glm::vec3> &points);
    float bezierBlend(int k, float mu, float n);
    std::vector<glm::vec3> bezierSurface(float rowOffset, float colOffset);
    std::vector<glm::vec3> initPatch(float rowOffset, float colOffset);
    std::vector<glm::vec3> initSurface();
    void addPatches(std::vector<std::vector<glm::vec3>> &controlPoints, int id);
    void draw();

    std::unique_ptr<OpenGLShape> openGLShape;
    bool isFilledIn();

    struct Patch {
        int pid = 0;
        // edge[0] = top, edge[1] = right, edge[2] = bottom, edge[3] = left
        std::vector<std::vector<glm::vec3>> edges;
        std::vector<std::vector<glm::vec3>> edges2;
        std::vector<std::vector<glm::vec3>> controlPoints;
    };

    std::vector<Patch> m_patches;

private:
    float randValue(int row, int col);
    const float m_numRows, m_numCols;
    const bool m_isFilledIn;
};

#endif // TERRAIN_H
