#include "resourcemanager.h"


// Instantiate static variables
QMap<QString, QOpenGLTexture*>       ResourceManager::Textures;
QMap<QString, QOpenGLShaderProgram*> ResourceManager::Shaders;


ResourceManager::ResourceManager() {
    initializeOpenGLFunctions();
}


QOpenGLShaderProgram*
ResourceManager::LoadShader(QString vShaderFile,
                            QString fShaderFile,
                            QString gShaderFile,
                            QString name)
{
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}


QOpenGLShaderProgram*
ResourceManager::GetShader(QString name) {
    return Shaders[name];
}


QOpenGLTexture*
ResourceManager::LoadTexture(QString file, QString name) {
    Textures[name] = loadTextureFromFile(file);
    return Textures[name];
}


QOpenGLTexture*
ResourceManager::GetTexture(QString name) {
    return Textures[name];
}


void
ResourceManager::Clear() {
    while(!Shaders.isEmpty())
        delete Shaders.last();
    while(!Textures.isEmpty())
        delete Textures.last();
}


QOpenGLShaderProgram*
ResourceManager::loadShaderFromFile(QString vShaderFile,
                                    QString fShaderFile,
                                    QString gShaderFile)
{
    QOpenGLShaderProgram* pShader = new QOpenGLShaderProgram();
    pShader->addShaderFromSourceFile(QOpenGLShader::Vertex,   vShaderFile);
    pShader->addShaderFromSourceFile(QOpenGLShader::Fragment, fShaderFile);
    if(gShaderFile != QString())
        pShader->addShaderFromSourceFile(QOpenGLShader::Geometry, gShaderFile);
    pShader->link();
    return pShader;
}


QOpenGLTexture*
ResourceManager::loadTextureFromFile(QString file) {
    QOpenGLTexture* pTexture = new QOpenGLTexture(QImage(file).mirrored());
    return pTexture;
}
