#pragma once

#include <QMap>
#include <QString>

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>


// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. Each loaded texture
// and/or shader is also stored for future reference by string
// handles. All functions and resources are static and no
// public constructor is defined. From https://learnopengl.com

class ResourceManager: protected QOpenGLFunctions_3_3_Core
{
public:
    // resource storage
    static QMap<QString, QOpenGLShaderProgram*> Shaders;
    static QMap<QString, QOpenGLTexture*>       Textures;

    static QOpenGLShaderProgram* LoadShader(QString vShaderFile, QString fShaderFile, QString gShaderFile, QString name);
    static QOpenGLShaderProgram* GetShader(QString name);
    static QOpenGLTexture* LoadTexture(QString file, QString name);
    static QOpenGLTexture* GetTexture(QString name);
    static void Clear();

private:
    ResourceManager();
    static QOpenGLShaderProgram* loadShaderFromFile(QString vShaderFile,
                                                    QString fShaderFile,
                                                    QString gShaderFile=QString());
    static QOpenGLTexture*       loadTextureFromFile(QString file);
};

