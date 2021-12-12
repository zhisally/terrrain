#include "terrain.h"

#include <math.h>
#include <iostream>
#include "gl/shaders/ShaderAttribLocations.h"

Terrain::Terrain() : m_numRows(96), m_numCols(m_numRows), m_isFilledIn(true)
{
}


/**
 * Returns a pseudo-random value between -1.0 and 1.0 for the given row and
 * column.
 */
float Terrain::randValue(int row, int col)
{
    return -4.0 + 8.0 * glm::fract(sin(row * 127.1f + col * 311.7f) * 43758.5453123f);
}

/**
 * Returns the normal vector for the terrain vertex at the given row and
 * column.
 */
glm::vec3 Terrain::getNormal(int row, int col, std::vector<glm::vec3> &points)
{
    // TODO: Compute the normal at the given row and column using the positions
    //       of the neighboring vertices.
    glm::vec3 vertex = points[row * m_numCols + col];
    glm::vec3 n0 = points[row * m_numCols + col + 1];
    glm::vec3 n1 = points[(row - 1) * m_numCols + col + 1];
    glm::vec3 n2 = points[(row - 1) * m_numCols + col];
    glm::vec3 n3 = points[(row - 1) * m_numCols + col - 1];
    glm::vec3 n4 = points[row * m_numCols + col + 1];
    glm::vec3 n5 = points[(row + 1) * m_numCols + col - 1];
    glm::vec3 n6 = points[(row + 1) * m_numCols + col];
    glm::vec3 n7 = points[(row + 1) * m_numCols + col + 1];

    glm::vec3 norm1 = glm::normalize(glm::cross(vertex-n1, vertex-n0));
    glm::vec3 norm2 = glm::normalize(glm::cross(vertex-n2, vertex-n1));
    glm::vec3 norm3 = glm::normalize(glm::cross(vertex-n3, vertex-n2));
    glm::vec3 norm4 = glm::normalize(glm::cross(vertex-n4, vertex-n3));
    glm::vec3 norm5 = glm::normalize(glm::cross(vertex-n5, vertex-n4));
    glm::vec3 norm6 = glm::normalize(glm::cross(vertex-n6, vertex-n5));
    glm::vec3 norm7 = glm::normalize(glm::cross(vertex-n7, vertex-n6));
    glm::vec3 norm8 = glm::normalize(glm::cross(vertex-n0, vertex-n7));
    glm::vec3 norm9 = {8,8,8};
    return glm::normalize((norm1 + norm2 + norm3 + norm4 + norm5 + norm6 + norm7 + norm8) / norm9);
}

bool Terrain::isFilledIn() {
    return m_isFilledIn;
}

float Terrain::bezierBlend(int k, float mu, float n) {
    double blend = 1.0;
    int nn = n;
    int kn = k;
    int nkn = n-k;
    while (nn >= 1.0) {
        blend *= nn;
        nn--;
        if (kn > 1.0) {
            blend /= (double)kn;
            kn--;
        }
        if (nkn > 1.0) {
            blend /= (double)nkn;
            nkn--;
        }

    }
    if (k >0.0) {
        blend *= pow(mu, (double)k);
    }
    if (n-k > 0.0) {
        blend *= pow(1.0-mu, (double)(n-k));
    }
    return blend;
}
/**
 * Initializes the terrain by storing positions and normals in a vertex buffer.
 */
std::vector<std::vector<glm::vec3>> Terrain::genControlPoints(int startRow, int startCol) {
    // Initializes a grid of vertices using triangle strips.
    std::vector<std::vector<glm::vec3>> controlPoints;
    for (int row = 0; row <= (m_numRows )/3; row++) {
        std::vector<glm::vec3> rowPoints((m_numCols)/3+1);
        for (int col = 0; col <= (m_numCols )/3; col++) {
            rowPoints[col] = glm::vec3(row, randValue(row, col), col);
        }
        controlPoints.push_back(rowPoints);
    }
    return controlPoints;

}

/**
 * Initializes the terrain by storing positions and normals in a vertex buffer.
 */
std::vector<std::vector<glm::vec3>> Terrain::genControlPoints1(int startRow, int startCol, std::vector<glm::vec3> top, std::vector<glm::vec3> secondTop) {
    // Initializes a grid of vertices using triangle strips.
    std::vector<std::vector<glm::vec3>> controlPoints;
    for (int row = 0; row < (m_numRows)/3-1; row++) {
        std::vector<glm::vec3> rowPoints((m_numCols)/3+1);
        for (int col = 0; col <= (m_numCols)/3; col++) {
            rowPoints[col] = glm::vec3(row, randValue(row + startRow, col), col);
        }
        controlPoints.push_back(rowPoints);
    }

    std::vector<glm::vec3> secondRow((m_numCols)/3+1);
    for (int col = 0; col <= (m_numCols)/3; col++) {
        glm::vec3 point1 = secondTop[col];
        glm::vec3 point2 = top[col];
        glm::vec3 slope = point2 - point1;
        glm::vec3 point3((m_numRows)/3-1,point2.y + slope.y,col);
        secondRow[col] = point3;
    }

    controlPoints.push_back(secondRow);
    std::vector<glm::vec3> topRow((m_numCols)/3+1);
    for (int col = 0; col <= (m_numCols)/3; col++) {
        glm::vec3 point((m_numRows)/3, top[col].y ,col);
        topRow[col] = point;
    }
    controlPoints.push_back(topRow);
    return controlPoints;
}

void Terrain::addPatches(std::vector<std::vector<glm::vec3>> &controlPoints, int id) {
    std::vector<glm::vec3> top;
    std::vector<glm::vec3> secondTop;
    for (int i = 0; i < controlPoints[0].size(); i++) {
        top.push_back(controlPoints[0][i]);
        secondTop.push_back(controlPoints[1][i]);
    }
    std::vector<glm::vec3> right;
    std::vector<glm::vec3> secondRight;
    for (int i = 0; i < controlPoints.size(); i++) {
        right.push_back(controlPoints[i][controlPoints[0].size()-1]);
        secondRight.push_back(controlPoints[i][controlPoints[0].size()-2]);
    }
    std::vector<glm::vec3> bottom;
    std::vector<glm::vec3> secondBottom;
    for (int i = 0; i < controlPoints[0].size(); i++) {
        bottom.push_back(controlPoints[controlPoints.size()-1][i]);
        secondBottom.push_back(controlPoints[controlPoints.size()-2][i]);
    }
    std::vector<glm::vec3> left;
    std::vector<glm::vec3> secondLeft;
    for (int i = 0; i < controlPoints.size(); i++) {
        left.push_back(controlPoints[i][0]);
        secondLeft.push_back(controlPoints[i][1]);
    }
    std::vector<std::vector<glm::vec3>> edges = {top, right, bottom, left};
    std::vector<std::vector<glm::vec3>> edges2 = {secondTop, secondRight, secondBottom, secondLeft};
    struct Patch patch = {id, edges, edges2, controlPoints};
    m_patches.push_back(patch);
}

std::vector<glm::vec3> Terrain::bezierSurface(float rowOffset, float colOffset) {
    std::vector<std::vector<glm::vec3>> controlPoints;
    if (m_patches.size() == 0) {
        controlPoints = genControlPoints(0,0);
    }
    else {
       controlPoints = genControlPoints1(-32,0, m_patches[0].edges[0], m_patches[0].edges2[0]);
    }
    addPatches(controlPoints, 0);

    int rowCP = (m_numRows)/3+1;
    int colCP = (m_numCols)/3+1;

    int numVertices = (m_numRows) * (m_numCols);
    std::vector<glm::vec3> output(numVertices);
    int index = 0;

    for (int i = 0; i < m_numRows; i++) {
        double mui = i / (double)(m_numRows);
        for (int j = 0; j < m_numCols; j++) {
            double muj = j / (double)(m_numCols);
            output[index].x = 0.0;
            output[index].y = 0.0;
            output[index].z = 0.0;
            for (int ki = 0; ki < rowCP; ki++) {
                float bi = bezierBlend(ki, mui, rowCP-1);
                for (int kj = 0; kj < colCP; kj++) {
                    float bj = bezierBlend(kj, muj, colCP-1);
                    output[index].x += (controlPoints[ki][kj].x * bi * bj);
                    output[index].y += (controlPoints[ki][kj].y * bi * bj);
                    output[index].z += (controlPoints[ki][kj].z * bi * bj);
                }
            }

            output[index].x = 40 * i / m_numRows - 20 - 40 * rowOffset;
            output[index].z = 40 * j / m_numCols - 20 - 40 * colOffset;
            index += 1;
        }
    }
    return output;
}

//std::vector<glm::vec3> Terrain::initPatch(float rowOffset, float colOffset) {
//    std::vector<glm::vec3> points = bezierSurface(rowOffset, colOffset);
//    int numVertices = (m_numRows - 1) * (2 * m_numCols + 2);
//    std::vector<glm::vec3> data(2 * numVertices);
//    int index = 0;
//    int pIndex = 0;
//    for (int row = 0; row < m_numRows - 1; row++) {
//        for (int col = m_numCols - 1; col >= 0; col--) {
//            data[index++] = points[row * m_numCols + col];
//            data[index++] = getNormal  (row, col, points);
//            data[index++] = points[(row+1) * (m_numCols) + col];
//            data[index++] = getNormal  (row + 1, col, points);
//        }
//        data[index++] = points[(row + 1) * m_numRows];
//        data[index++] = getNormal  (row + 1, 0, points);
//        data[index++] = points[(row+1) * m_numCols + m_numCols - 1];
//        data[index++] = getNormal  (row + 1, m_numCols - 1, points);
//    }

//    return data;
//}

std::vector<glm::vec3> Terrain::initSurface() {
    std::vector<glm::vec3> points1 = bezierSurface(0.0, 0.0);
    std::vector<glm::vec3> points2 = bezierSurface(1.0, 0.0);

    for (int i = 1; i >= 0; i--) {
        for (int row = 0; row  <= (m_numRows )/3; row ++) {
            std::cout << "x: " + std::to_string(m_patches[i].controlPoints[row][0].x) + " y: " + std::to_string(m_patches[i].controlPoints[row][0].y)+ " z: " + std::to_string(m_patches[i].controlPoints[row][0].z) << std::endl;
        }
    }
    points2.insert(points2.end(), points1.begin(), points1.end());

//    for (int row = 0; row < m_numRows * 2; row ++) {
//        std::cout << "x: " + std::to_string(points2[row * m_numCols].x) + " y: " + std::to_string(points2[row * m_numCols].y)+ " z: " + std::to_string(points2[row * m_numCols].z) << std::endl;
//    }
    int numVertices = (2 * m_numRows - 1) * (2 * m_numCols + 2);
    std::vector<glm::vec3> data(2 * numVertices);
    int index = 0;
    int pIndex = 0;
    for (int row = 0; row < 2 * m_numRows - 1; row++) {
        for (int col = m_numCols - 1; col >= 0; col--) {
            data[index++] = points2[row * m_numCols + col];
            data[index++] = getNormal  (row, col, points2);
            data[index++] = points2[(row+1) * (m_numCols) + col];
            data[index++] = getNormal  (row + 1, col, points2);
        }
        data[index++] = points2[(row + 1) * m_numRows];
        data[index++] = getNormal  (row + 1, 0, points2);
        data[index++] = points2[(row+1) * m_numCols + m_numCols - 1];
        data[index++] = getNormal  (row + 1, m_numCols - 1, points2);
    }

    return data;
}

/**
 * Draws the terrain.
 */
void Terrain::draw()
{
    openGLShape->draw();
}
