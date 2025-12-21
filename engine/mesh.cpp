/**
 * @file    mesh.cpp
 * @brief   Mesh class implementation
 *
 * @author  Giona Valsecchi (C) SUPSI [giona.valsecchi@supsi.ch] , Pietro Brusadelli (C) SUPSI [pietro.brusadelli@supsi.ch], Filippo De Simoni (C) SUPSI [filippo.desimoni@supsi.ch]
 */

#include "engine.h"

// Freeglut:
#include <GL/freeglut.h>

namespace Eng
{

    /////////////////
    // MESH CLASS //
    /////////////////

    Mesh::Mesh(const std::string &name, const glm::mat4 &matrix,
               std::vector<glm::vec3> vertexes,
               std::vector<glm::uvec3> faces,
               std::vector<glm::vec4> normals,
               std::vector<glm::vec2> textureCoordinates)
        : Node(name, matrix),
          vertexes{vertexes},
          faces{faces},
          normals{normals},
          textureCoordinates{textureCoordinates},
          material{nullptr}
    {
    }

    Mesh::~Mesh()
    {
        // todo
    }

    void Mesh::render(glm::mat4 modelview)
    {

        glLoadMatrixf(glm::value_ptr(modelview));

        if (vertexes.empty())
            return;

        if (material)
        {

            material->render();
        }

        glBegin(GL_TRIANGLES);
        for (auto f : faces)
        {
            glm::vec3 normal = normals[f.x];
            glm::vec2 texture = textureCoordinates[f.x];
            glm::vec3 vertex = vertexes[f.x];

            glNormal3f(normal.x, normal.y, normal.z);
            glTexCoord2f(texture.s, texture.t);
            glVertex3f(vertex.x, vertex.y, vertex.z);

            normal = normals[f.y];
            texture = textureCoordinates[f.y];
            vertex = vertexes[f.y];

            glNormal3f(normal.x, normal.y, normal.z);
            glTexCoord2f(texture.s, texture.t);
            glVertex3f(vertex.x, vertex.y, vertex.z);

            normal = normals[f.z];
            texture = textureCoordinates[f.z];
            vertex = vertexes[f.z];

            glNormal3f(normal.x, normal.y, normal.z);
            glTexCoord2f(texture.s, texture.t);
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        glEnd();
    }

    void Mesh::setMaterial(Material *materialPtr)
    {
        this->material = materialPtr;
    }

    Material *Mesh::getMaterial()
    {
        return this->material;
    }
}; // end of namespace Eng::