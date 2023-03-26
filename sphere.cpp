#include "sphere.h"


Sphere::Sphere(float radius, int slices, int stacks)
    : m_vbo(nullptr)
    , m_radius(radius)
    , m_slices(slices)
    , m_stacks(stacks)
{
    initializeOpenGLFunctions();
    m_firstDraw = true;
    m_stripsize = (slices + 1) * 2;
    int nverts  = m_stripsize * (m_stacks - 2) + 2 * (slices + 2);
    QVector3D* vertices  = new QVector3D[nverts];
    QVector3D* normals   = new QVector3D[nverts];
    QVector3D* tangents  = new QVector3D[nverts];
    QVector2D* texCoords = new QVector2D[nverts];

    double latstep  = M_PI / m_stacks;
    double longstep = 2.0 * M_PI / m_slices;
    double texXstep = 1.0 / m_slices;
    double texYstep = 1.0 / m_stacks;

    double lat0  = -M_PI / 2.0 + latstep;
    double z0    = m_radius * sin(lat0);
    double zcos0 = m_radius * cos(lat0);
    double texY  = texYstep;

    double lng  = M_PI;
    double texX = 0;
    double coslong;// = cos(lng);
    double sinlong;// = sin(lng);
    int idx     = 0;

    // mid lat strips
    for(int i=1; i<m_stacks-1; i++) {

        double lat1  = lat0 + latstep;
        double z1    = m_radius * sin(lat1);
        double zcos1 = m_radius * cos(lat1);

        // cout << 180*lat0/M_PI << " " << 180*lat1/M_PI << endl;
        lng = -M_PI;
        texX = 0.0;

        for(int j=0; j<=m_slices; j++) {
            coslong = cos(lng);
            sinlong = sin(lng);
            vertices[idx]    = QVector3D(zcos1 * coslong, zcos1 * sinlong, z1);
            normals[idx]     = QVector3D(cos(lat1) * coslong, cos(lat1) * sinlong, sin(lat1));
            tangents[idx]    = QVector3D(-sinlong, coslong, 0.0);
            texCoords[idx++] = QVector2D(texX, texY + texYstep);
            vertices[idx]    = QVector3D(zcos0 * coslong, zcos0 * sinlong, z0);
            normals[idx]     = QVector3D(cos(lat0) * coslong, cos(lat0) * sinlong, sin(lat0));
            tangents[idx]    = QVector3D(-sinlong, coslong, 0.0);
            texCoords[idx++] = QVector2D(texX, texY);
            lng += longstep;
            texX += texXstep;
        }
        // swap lat1, z1, zcos1 up
        lat0 = lat1;
        z0 = z1;
        zcos0 = zcos1;
        texY += texYstep;
    }

    // draw polar caps as fans

    // north pole
    vertices[idx]    = QVector3D(0.0f, 0.0f, m_radius);
    normals[idx]     = QVector3D(0.0f, 0.0f, 1.0f);
    tangents[idx]    = QVector3D(1.0f, 0.0f, 0.0f);
    texCoords[idx++] = QVector2D(0.5f, 1.0f);
    lat0 = M_PI_2 - latstep;
    lng = -M_PI;
    z0 = m_radius * sin(lat0);
    texX = 0;
    texY = 1 - texYstep;
    zcos0 = m_radius * cos(lat0);
    for(int i=0; i<=m_slices; i++) {
        coslong = cos(lng);
        sinlong = sin(lng);
        vertices[idx]    = QVector3D(zcos0 * coslong, zcos0 * sinlong, z0);
        normals[idx]     = QVector3D(cos(lat0) * coslong, cos(lat0) * sinlong, sin(lat0));
        tangents[idx]    = QVector3D(-sinlong, coslong, 0.);
        texCoords[idx++] = QVector2D(texX, texY);
        lng += longstep;
        texX += texXstep;
    }

    // south pole
    vertices[idx]    = QVector3D(0, 0, -m_radius);
    normals[idx]     = QVector3D(0, 0., -1.);
    tangents[idx]    = QVector3D(-1, 0., 0.);
    texCoords[idx++] = QVector2D(0.5, 0);
    lat0 = -M_PI_2 + latstep;
    lng  = M_PI; // Q: why M_PI and not -M_PI
    texX = 1;
    texY = texYstep;
    z0 = m_radius * sin(lat0);
    zcos0 = m_radius * cos(lat0);
    for(int i=0; i<=m_slices; i++) {
        coslong = cos(lng);
        sinlong = sin(lng);
        vertices[idx]    = QVector3D(zcos0 * coslong, zcos0 * sinlong, z0);
        normals[idx]     = QVector3D(cos(lat0) * coslong, cos(lat0) * sinlong, sin(lat0));
        tangents[idx]    = QVector3D(-sinlong, coslong, 0);
        texCoords[idx++] = QVector2D(texX, texY);
        lng -= longstep; // Q: why -= ?
        texX -= texXstep;
    }

    if(initVBO()) {
        int DataSize =
                m_stripsize * (m_stacks - 2) * sizeof(QVector3D); // all mid lat strips
        DataSize += 2 * (m_slices + 2) * sizeof(QVector3D);       // two polar fans
        // size of Texture
        int TexSize =
                m_stripsize * (m_stacks - 2) * sizeof(QVector2D); // all mid lat strips
        TexSize += 2 * (m_slices + 2) * sizeof(QVector2D);        // two polar fans
        // size of Normals, Tangents
        int NormalSize = DataSize;
        m_vbo->bind();
        m_vbo->allocate(DataSize + TexSize + 2 * NormalSize);
        m_vbo->write(0, vertices, DataSize);
        m_vbo->write(DataSize, texCoords, TexSize);
        m_vbo->write(DataSize + TexSize, normals, NormalSize);
        m_vbo->write(DataSize + TexSize + NormalSize, tangents, NormalSize);
        m_vbo->release();
    }

    delete[] vertices;
    vertices = nullptr;
    delete[] texCoords;
    texCoords = nullptr;
    delete[] normals;
    normals = nullptr;
    delete[] tangents;
    tangents = nullptr;
}


bool
Sphere::initVBO() {
    m_vao = new QOpenGLVertexArrayObject();
    bool ok = m_vao->create();
    if(!ok) return false;
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    ok = m_vbo->create();
    if(!ok) return false;
    m_vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    return ok;
}


Sphere::~Sphere() {
    if(m_vbo) {
        m_vbo->release();
        delete m_vbo;
        m_vbo = NULL;
    }
    if(m_vao) {
        m_vao->release();
        delete m_vao;
        m_vao = NULL;
    }
}


void
Sphere::setupVAO(QOpenGLShaderProgram* pProgram) {
    m_vao->bind();
    m_vbo->bind();
    pProgram->enableAttributeArray("vPosition");
    pProgram->setAttributeBuffer("vPosition", GL_FLOAT, 0, 3, 0);
    pProgram->enableAttributeArray("vTexture");
    int nverts = (m_stacks - 2) * m_stripsize + 2 * (m_slices + 2);
    pProgram->setAttributeBuffer("vTexture", GL_FLOAT, nverts * sizeof(QVector3D), 2, 0);
    pProgram->enableAttributeArray("vNormal");
    pProgram->setAttributeBuffer("vNormal", GL_FLOAT,
                             nverts * (sizeof(QVector3D) + sizeof(QVector2D)), 3, 0);
    pProgram->enableAttributeArray("vTangent");
    pProgram->setAttributeBuffer("vTangent", GL_FLOAT,
                             nverts * (2 * sizeof(QVector3D) + sizeof(QVector2D)), 3, 0);
    m_vao->release();
    m_vbo->release();
}


void
Sphere::draw(QOpenGLShaderProgram* pProgram) {
    if (!pProgram) {
        return;
    }
    if (m_firstDraw) {
        setupVAO(pProgram);
        m_firstDraw = false;
    }
    m_vao->bind();

    for(int i=0; i<m_stacks-2; i++) {
        glDrawArrays(GL_TRIANGLE_STRIP, i*m_stripsize, m_stripsize);
    }
    int offset = (m_stacks - 2) * m_stripsize;
    int fansize = m_slices + 2;
    glDrawArrays(GL_TRIANGLE_FAN, offset, fansize);
    glDrawArrays(GL_TRIANGLE_FAN, offset + fansize, fansize);
    m_vao->release();
}
