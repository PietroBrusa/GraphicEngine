/**
 * @file    list.cpp
 * @brief   List class implementation
 *
 * @author  Giona Valsecchi (C) SUPSI [giona.valsecchi@supsi.ch] , Pietro Brusadelli (C) SUPSI [pietro.brusadelli@supsi.ch], Filippo De Simoni (C) SUPSI [filippo.desimoni@supsi.ch]
 */

#include "engine.h"

// Freeglut:
#include <GL/freeglut.h>

// GLM:
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace Eng
{

    ////////////////
    // List CLASS //
    ////////////////

    ENG_API List::List(std::string name)
        : Object(name),
          camera(nullptr)
    {
    }

    List::~List()
    {
    }

    glm::mat4 List::createShadowMatrix(const glm::vec4 &lightPos, const glm::vec4 &planeEquation)
    {
        glm::mat4 shadowMat;
        float dot = planeEquation.x * lightPos.x +
                    planeEquation.y * lightPos.y +
                    planeEquation.z * lightPos.z +
                    planeEquation.w * lightPos.w;

        shadowMat[0][0] = dot - lightPos.x * planeEquation.x;
        shadowMat[1][0] = 0.0f - lightPos.x * planeEquation.y;
        shadowMat[2][0] = 0.0f - lightPos.x * planeEquation.z;
        shadowMat[3][0] = 0.0f - lightPos.x * planeEquation.w;

        shadowMat[0][1] = 0.0f - lightPos.y * planeEquation.x;
        shadowMat[1][1] = dot - lightPos.y * planeEquation.y;
        shadowMat[2][1] = 0.0f - lightPos.y * planeEquation.z;
        shadowMat[3][1] = 0.0f - lightPos.y * planeEquation.w;

        shadowMat[0][2] = 0.0f - lightPos.z * planeEquation.x;
        shadowMat[1][2] = 0.0f - lightPos.z * planeEquation.y;
        shadowMat[2][2] = dot - lightPos.z * planeEquation.z;
        shadowMat[3][2] = 0.0f - lightPos.z * planeEquation.w;

        shadowMat[0][3] = 0.0f - lightPos.w * planeEquation.x;
        shadowMat[1][3] = 0.0f - lightPos.w * planeEquation.y;
        shadowMat[2][3] = 0.0f - lightPos.w * planeEquation.z;
        shadowMat[3][3] = dot - lightPos.w * planeEquation.w;

        return shadowMat;
    }

    void List::renderShadows(const glm::vec4 &planeEquation)
    {
        if (lightList.empty() || meshList.empty())
            return;

        // Usa la prima luce trovata
        Instance lightInst = lightList.front();
        glm::vec4 lightPos = glm::vec4(lightInst.nodeWorldMatrix[3]);

        // Crea matrice ombra
        glm::mat4 shadowMatrix = createShadowMatrix(lightPos, planeEquation);

        // Setup rendering ombre
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);
        // glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.65f, 0.65f, 0.65f, 0.1f);

        glm::mat4 viewMatrix = camera->getViewMatrix();

        // Renderizza ombre per ogni mesh
        for (auto &inst : meshList)
        {

            glm::mat4 modelViewShadow = viewMatrix * shadowMatrix * inst.nodeWorldMatrix;


            glLoadMatrixf(glm::value_ptr(modelViewShadow));
            glTranslatef(0.0f, 0.01f, 0.0f); // Offset per evitare z-fighting

            Mesh *mesh = dynamic_cast<Mesh *>(inst.node);
            if (mesh)
            {
                // set global bool to remove gltexture on material rendering
                Eng::Base::getInstance().setShadowRender(true);
                mesh->render(modelViewShadow);
                Eng::Base::getInstance().setShadowRender(false);
            }

            glPopMatrix();
        }

        // Ripristina stato
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    void List::render(glm::mat4 modelview)
    {
        if (camera == nullptr)
        {
            std::cerr << "No camera found" << std::endl;
            return;
        }

        glMatrixMode(GL_MODELVIEW);
        glm::mat4 viewMatrix = camera->getViewMatrix();

        // Renderizza prima le ombre
        glm::vec4 groundPlane(0.0f, 1.0f, 0.0f, 0.0f); // Piano y=0
        renderShadows(groundPlane);

        // Poi renderizza normalmente
        for (auto inst : list)
        {
            glm::mat4 modelViewModel = viewMatrix * inst.nodeWorldMatrix;
            inst.node->render(modelViewModel);
        }
    }

    void List::pass(Node *root, glm::mat4 matrix)
    {
        matrix = matrix * root->getMatrix();

        Instance inst;
        inst.node = root;
        inst.node->calculateMove();
        inst.nodeWorldMatrix = root->getWorldCoordinateMatrix();

        if (dynamic_cast<Light *>(inst.node) != nullptr)
        {
            this->list.push_front(inst);
            this->lightList.push_back(inst);
        }
        else if (dynamic_cast<Mesh *>(inst.node) != nullptr)
        {
            this->list.push_back(inst);
            this->meshList.push_back(inst);
        }

        for (Node *child : root->getChildren())
        {
            this->pass(child, matrix);
        }
    }

    void List::setCamera(Eng::Camera *camera)
    {
        if (camera == nullptr)
        {
            std::cerr << "Cannot set a null camera" << std::endl;
            return;
        }

        this->camera = camera;

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(this->camera->getProjectionMatrix()));
    }

    void List::clear()
    {
        list.clear();
        meshList.clear();
        lightList.clear();
    }

}; // end of namespace Eng::
