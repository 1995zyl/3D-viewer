#include "opengl_window.h"
#include "spdlog/spdlog.h"
#include <QFile>

#define WHEEL_MIN (0.1 * 0.1)
#define WHEEL_MAX (10 * 10 * 10 * 10)
#define TIMER_ROTATE_ANGLE 45
#define TIMER_ROTATE_NUM 50
#define TIMER_HOVER_HEIGHT 3

static const std::array<float, 3> sLightPos{ 1.2f, 1.0f, 2.0f };
static const std::array<float, 3>  sLightColorLoc{ 1.0f, 1.0f, 1.0f };

OpenGLWindow::OpenGLWindow(const QString &modelPath, const QColor &color, QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    initializeFpsLabel();

    QRgb rgba = color.rgba();
    m_bgColor = {(float)qRed(rgba) / 255, (float)qGreen(rgba) / 255, (float)qBlue(rgba) / 255, (float)qAlpha(rgba) / 255};
    
    if (!modelPath.isEmpty())
    {
        OpenGLHelper::import3DModel(modelPath, m_modelMeshs);
        m_fpsLabel->show();
        m_fpsTimer.setInterval(1000);
        m_fpsTimer.start();
    }

    initializeZoom();
    connect(&m_fpsTimer, &QTimer::timeout, this, &OpenGLWindow::onFpsTimeOut);
    connect(&m_animationTimer, &QTimer::timeout, this, &OpenGLWindow::onAnimationTimeOut);
}

OpenGLWindow::~OpenGLWindow()
{
    for (auto& modelMesh : m_modelMeshs)
    {
        glDeleteVertexArrays(1, &modelMesh.m_VAO);
        glDeleteBuffers(1, &modelMesh.m_VBO);
        glDeleteBuffers(1, &modelMesh.m_EBO);
    }
}

void OpenGLWindow::initializeFpsLabel()
{
    m_fpsLabel = new QLabel(this);
    m_fpsLabel->setFixedSize(150, 30);
    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPointSize(10);
    m_fpsLabel->setFont(font);
    m_fpsLabel->move(10, 10);
    m_fpsLabel->hide();
}

void OpenGLWindow::initializeZoom()
{
    if (m_modelMeshs.isEmpty())
        return;

    float maxPosition = 1.0;
    for (const auto &modelMesh : m_modelMeshs)
    {
        for (const auto &vertex : modelMesh.m_vertices)
        {
            maxPosition = qMax(qMax(qMax(qAbs(vertex.m_positions[0]), qAbs(vertex.m_positions[1])), qAbs(vertex.m_positions[2])), maxPosition);
        }
    }

    spdlog::info("model max position is {}.", maxPosition);
    if (maxPosition < 5)
    {
        m_cameraDistance = maxPosition * 5;
        m_camera.m_zNear = 0.01;
        m_camera.m_zFar = maxPosition * 100;
    }
    else if (maxPosition < 100)
    {
        m_cameraDistance = maxPosition * 3;
        m_camera.m_zNear = 0.05;
        m_camera.m_zFar = maxPosition * 50;
    }
    else if (maxPosition < 1000)
    {
        m_cameraDistance = maxPosition * 3;
        m_camera.m_zNear = 0.05;
        m_camera.m_zFar = maxPosition * 30;
    }
    else if (maxPosition < 2000)
    {
        m_cameraDistance = maxPosition * 4;
        m_camera.m_zNear = 0.07;
        m_camera.m_zFar = maxPosition * 10;
    }
    else if (maxPosition < 3000)
    {
        m_cameraDistance = maxPosition * 4.5;
        m_camera.m_zNear = 0.4;
        m_camera.m_zFar = maxPosition * 10;
    }
    else
    {
        m_cameraDistance = maxPosition * 4.5;
        m_camera.m_zNear = 0.5;
        m_camera.m_zFar = maxPosition * 10;
    }

    m_camera.m_eye.setZ(m_camera.m_zoom * m_cameraDistance);
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    if (compileGLSL())
    {
        initializeMesh();
    }
}

void OpenGLWindow::paintGL()
{
    glClearColor(m_bgColor[0], m_bgColor[1], m_bgColor[2], m_bgColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ++m_frameCount;

    glUseProgram(m_glslProgramId);

    // 先将物体进行旋转，再进行平移缩放等其他变换
    QMatrix4x4 rotation;
    rotation.rotate(qreal(m_camera.m_zRot) / 16.0f, 0.0f, 0.0f, 1.0f);
    rotation.rotate(qreal(m_camera.m_yRot) / 16.0f, 0.0f, 1.0f, 0.0f);
    rotation.rotate(qreal(m_camera.m_xRot) / 16.0f, 1.0f, 0.0f, 0.0f);
    rotation = rotation * m_camera.m_rotation;

    QMatrix4x4 m1, m2;
    m1.lookAt(m_camera.m_eye, m_camera.m_center, m_camera.m_up);
    m1 *= rotation;
    m2.translate(m_camera.m_xTrans, -1.0 * m_camera.m_yTrans, 0);
    m2 *= m_camera.m_translation;

    glUniformMatrix4fv(glGetUniformLocation(m_glslProgramId, "projection"), 1, GL_FALSE, m_camera.m_projection.data());
    glUniformMatrix4fv(glGetUniformLocation(m_glslProgramId, "view"), 1, GL_FALSE, m2.data());
    glUniformMatrix4fv(glGetUniformLocation(m_glslProgramId, "model"), 1, GL_FALSE, m1.data());

    paintMesh();
}

void OpenGLWindow::resizeGL(int w, int h)
{
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    m_camera.m_projection.setToIdentity();
    m_camera.m_projection.perspective(m_camera.m_fovy, aspect, m_camera.m_zNear, m_camera.m_zFar);
}

void OpenGLWindow::mousePressEvent(QMouseEvent *e)
{
    m_mousePress = true;
    m_mousePos = e->pos();
    switch (e->button())
    {
    case Qt::LeftButton:
        m_mouseFlag = Qt::LeftButton;
        break;
    case Qt::RightButton:
        m_mouseFlag = Qt::RightButton;
        break;
    case Qt::MiddleButton:
        m_mouseFlag = Qt::MiddleButton;
        break;
    }
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *e)
{
    m_mousePress = false;
    releasePos(e->button());
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_mousePress)
        return;

    QPoint diff = e->pos() - m_mousePos;
    switch (m_mouseFlag)
    {
    case Qt::LeftButton:
    case Qt::MiddleButton:
        m_camera.m_yRot = setRotation(4 * diff.x());
        m_camera.m_xRot = setRotation(4 * diff.y());
        break;
    case Qt::RightButton:
    {
        qreal w_h_ratio = (qreal)(this->width()) / (qreal)(this->height());
        qreal cube_view_height = 2 * m_camera.m_zoom * m_cameraDistance * qTan(qDegreesToRadians(m_camera.m_fovy / 2));
        qreal cube_view_width = w_h_ratio * cube_view_height;
        m_camera.m_xTrans = cube_view_width / qreal(width()) * qreal(diff.x());
        m_camera.m_yTrans = cube_view_height / qreal(height()) * qreal(diff.y());
        break;
    }
    default:
        return;
    }
    repaint();
}

void OpenGLWindow::wheelEvent(QWheelEvent *e)
{
    if (m_mousePress)
        return;

    if (e->angleDelta().y() > 0)
        m_camera.m_zoom -= m_wheelScale;
    else
        m_camera.m_zoom += m_wheelScale;

    if (m_camera.m_zoom >= WHEEL_MAX)
        m_camera.m_zoom = WHEEL_MAX;
    else if (m_camera.m_zoom <= WHEEL_MIN)
        m_camera.m_zoom = WHEEL_MIN;

    m_camera.m_eye.setZ(m_camera.m_zoom * m_cameraDistance * 0.25);

    repaint();
}

int OpenGLWindow::setRotation(int angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
    return angle;
}

void OpenGLWindow::releasePos(Qt::MouseButton mbType)
{
    QMatrix4x4 mat;
    switch (mbType)
    {
    case Qt::LeftButton:
    case Qt::MiddleButton:
        mat.rotate(qreal(m_camera.m_zRot) / 16.0f, 0.0f, 0.0f, 1.0f);
        mat.rotate(qreal(m_camera.m_yRot) / 16.0f, 0.0f, 1.0f, 0.0f);
        mat.rotate(qreal(m_camera.m_xRot) / 16.0f, 1.0f, 0.0f, 0.0f);
        m_camera.m_rotation = mat * m_camera.m_rotation;
        m_camera.m_xRot = 0;
        m_camera.m_yRot = 0;
        m_camera.m_zRot = 0;
        break;
    case Qt::RightButton:
        mat.translate(m_camera.m_xTrans, -1.0 * m_camera.m_yTrans, 0);
        m_camera.m_translation = mat * m_camera.m_translation;
        m_camera.m_xTrans = 0;
        m_camera.m_yTrans = 0;
        break;
    }
}

void OpenGLWindow::initializeMesh()
{
    for (auto &modelMesh : m_modelMeshs)
    {
        for (auto &texture : modelMesh.m_textures)
        {
            unsigned int textureID;
            glGenTextures(1, &textureID);
            if (!texture.m_data)
            {
                spdlog::error("image data is null.");
                continue;
            }

            GLenum format = GL_RGBA;
            if (texture.m_channel == 1)
                format = GL_RED;
            else if (texture.m_channel == 3)
                format = GL_RGB;
            else if (texture.m_channel == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, texture.m_width, texture.m_height, 0, format, GL_UNSIGNED_BYTE, texture.m_data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            OpenGLHelper::cleanImageData(texture.m_data);

            texture.m_id = textureID;
        }

        glGenVertexArrays(1, &modelMesh.m_VAO);
        glGenBuffers(1, &modelMesh.m_VBO);
        glGenBuffers(1, &modelMesh.m_EBO);

        glBindVertexArray(modelMesh.m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, modelMesh.m_VBO);
        glBufferData(GL_ARRAY_BUFFER, modelMesh.m_vertices.size() * sizeof(OpenGLHelper::Vertex), &modelMesh.m_vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelMesh.m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelMesh.m_indices.size() * sizeof(unsigned int), &modelMesh.m_indices[0], GL_STATIC_DRAW);

        // vertex Positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLHelper::Vertex), (void *)0);
        glEnableVertexAttribArray(0);

        // vertex normals
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLHelper::Vertex), (void *)offsetof(OpenGLHelper::Vertex, m_normals));
        glEnableVertexAttribArray(1);
        
        // vertex texture coords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(OpenGLHelper::Vertex), (void *)offsetof(OpenGLHelper::Vertex, m_texCoords));
        glEnableVertexAttribArray(2);

        // vertex tangent
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLHelper::Vertex), (void *)offsetof(OpenGLHelper::Vertex, m_tangents));
        glEnableVertexAttribArray(3);

        // vertex bitangent
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLHelper::Vertex), (void *)offsetof(OpenGLHelper::Vertex, m_bitangents));
        glEnableVertexAttribArray(4);

        // ids
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(OpenGLHelper::Vertex), (void *)offsetof(OpenGLHelper::Vertex, m_boneIDs));
        glEnableVertexAttribArray(5);

        // weights
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(OpenGLHelper::Vertex), (void *)offsetof(OpenGLHelper::Vertex, m_weights));
        glEnableVertexAttribArray(6);

        glBindVertexArray(0);
    }
}

void OpenGLWindow::paintMesh()
{
    for (auto &modelMesh : m_modelMeshs)
    {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < modelMesh.m_textures.size(); i++)
        {
            unsigned int number = 1;
            glActiveTexture(GL_TEXTURE0 + i);
            if ("texture_diffuse" == modelMesh.m_textures[i].m_type)
                number = diffuseNr++;
            else if ("texture_specular" == modelMesh.m_textures[i].m_type)
                number = specularNr++;
            else if ("texture_normal" == modelMesh.m_textures[i].m_type)
                number = normalNr++;
            else if ("texture_height" == modelMesh.m_textures[i].m_type)
                number = heightNr++;

            // set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(m_glslProgramId, 
                (modelMesh.m_textures[i].m_type + std::to_string(number)).c_str()), i);
            
            // bind the texture
            glBindTexture(GL_TEXTURE_2D, modelMesh.m_textures[i].m_id);
        }

        // light
        glUniform3f(glGetUniformLocation(m_glslProgramId, "lightPos"), 
            sLightPos[0], sLightPos[1], sLightPos[2]);
        glUniform3f(glGetUniformLocation(m_glslProgramId, "lightColor"), 
            sLightColorLoc[0], sLightColorLoc[1], sLightColorLoc[2]);
        glUniform3f(glGetUniformLocation(m_glslProgramId, "viewPos"), 
            m_camera.m_eye.x(), m_camera.m_eye.y(), m_camera.m_eye.z());

        // draw mesh
        glBindVertexArray(modelMesh.m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(modelMesh.m_indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }
}

void OpenGLWindow::setBgColor(const QColor &color)
{
    QRgb rgba = color.rgba();
    m_bgColor = {(float)qRed(rgba) / 255, (float)qGreen(rgba) / 255, (float)qBlue(rgba) / 255, (float)qAlpha(rgba) / 255};
    repaint();
}

void OpenGLWindow::setWheelScale(float wheelScale)
{
    m_wheelScale = wheelScale;
}

void OpenGLWindow::startAnimation(AnimationType animationType, int millisecond)
{
    m_animationPos = 0;
    m_animationLoopNum = 0;
    m_animationType = animationType;
    m_animationTimer.setInterval(millisecond);
    m_animationTimer.start();
}

void OpenGLWindow::stopAnimation()
{
    if (m_animationTimer.isActive())
    {
        m_animationTimer.stop();

        Qt::MouseButton mbType;
        switch (m_animationType)
        {
        case OpenGLWindow::Turntable:
        case OpenGLWindow::Sway:
            mbType = Qt::LeftButton;
            break;
        case OpenGLWindow::Hover:
            mbType = Qt::RightButton;
            break;
        default:
            return;
        }
        releasePos(mbType);
    }
}

void OpenGLWindow::onFpsTimeOut()
{
    QPalette pe;
    pe.setColor(QPalette::WindowText, QColor(255 - m_bgColor[0] * 255, 255 - m_bgColor[1] * 255, 255 - m_bgColor[2] * 255));
    m_fpsLabel->setPalette(pe);
    m_fpsLabel->setText(QString("%1(FPS): %2").arg(tr("frame rate")).arg(m_frameCount));

    m_frameCount = 0;
}

void OpenGLWindow::onAnimationTimeOut()
{
    switch (m_animationType)
    {
    case Turntable:
        m_animationPos += TIMER_ROTATE_ANGLE;
        m_camera.m_yRot = setRotation(m_animationPos);
        break;
    case Sway:
        m_animationLoopNum = m_animationLoopNum % TIMER_ROTATE_NUM;
        if (m_animationLoopNum < TIMER_ROTATE_NUM / 2)
            m_animationPos += TIMER_ROTATE_ANGLE;
        else
            m_animationPos -= TIMER_ROTATE_ANGLE;
        ++m_animationLoopNum;
        m_camera.m_yRot = setRotation(m_animationPos);
        break;
    case Hover:
    {
        m_animationLoopNum = m_animationLoopNum % TIMER_ROTATE_NUM;
        if (m_animationLoopNum < TIMER_ROTATE_NUM / 2)
            m_animationPos -= TIMER_HOVER_HEIGHT;
        else
            m_animationPos += TIMER_HOVER_HEIGHT;
        ++m_animationLoopNum;
        qreal w_h_ratio = (qreal)(this->width()) / (qreal)(this->height());
        qreal cube_view_height = 2 * m_camera.m_zoom * m_cameraDistance * qTan(qDegreesToRadians(m_camera.m_fovy / 2));
        qreal cube_view_width = w_h_ratio * cube_view_height;
        m_camera.m_yTrans = cube_view_height / qreal(height()) * qreal(m_animationPos);
        break;
    }
    default:
        return;
    }
    repaint();
}

bool OpenGLWindow::compileGLSL()
{
    QFile vertFile(":/shader.vert");
    QFile fragFile(":/shader.frag");
    if (!vertFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        spdlog::error("open vert file failed. path: {}", vertFile.fileName().toStdString());
        return false;
    }
    if (!fragFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        spdlog::error("open frag file failed. path: {}", fragFile.fileName().toStdString());
        return false;
    }
    const char *vShaderCode = vertFile.readAll().constData();
    const char *fShaderCode = fragFile.readAll().constData();

    // compile shaders
    GLint success = 0;
    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &(vShaderCode), NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        spdlog::error("compile vertex shader failed. id: {}.", vertex);
        return false;
    }
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        spdlog::error("compile fragment shader failed. id: {}.", fragment);
        return false;
    }

    // shader Program
    m_glslProgramId = glCreateProgram();
    glAttachShader(m_glslProgramId, vertex);
    glAttachShader(m_glslProgramId, fragment);
    glLinkProgram(m_glslProgramId);
    glGetProgramiv(m_glslProgramId, GL_LINK_STATUS, &success);
    if (!success)
    {
        spdlog::error("compile program failed. id: {}, error: {}", m_glslProgramId, success);
        return false;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return true;
}
