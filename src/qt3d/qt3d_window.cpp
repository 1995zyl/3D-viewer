﻿#include "qt3d_window.h"
#include "spdlog/spdlog.h"
#include "opengl/opengl_helper.h"
#include <QVBoxLayout>
#include <Qt3DRender/private/qsceneimportfactory_p.h>
#include <QInputAspect>
#include <QForwardrenderer>
#include <QMouseEvent>


/// @brief 
/// @param modelPath 
/// @param color 
/// @param parent 
Qt3DWindowContainer::Qt3DWindowContainer(const QString &modelPath, const QColor &color, QWidget *parent)
	: QWidget(parent)
{
	m_modelPath = modelPath;
	if (!modelPath.isEmpty())
		init3D(color);
}

Qt3DWindowContainer::~Qt3DWindowContainer()
{
	if (!m_modelPath.isEmpty())
	{
		delete m_cameraEntity;
		delete m_camController;
		delete m_light;
		delete m_lightTransform;
		delete m_lightEntity;
		delete m_pSceneImporter;
	}
}

void Qt3DWindowContainer::init3D(const QColor &color)
{
	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 0, 0, 0);
	m_view = new Q3DWindowEx(this);
	m_view->defaultFrameGraph()->setClearColor(color);
	m_3dContainer = QWidget::createWindowContainer(m_view);
	vLayout->addWidget(m_3dContainer);

	m_pSceneImporter = Qt3DRender::QSceneImportFactory::create("assimpEx", QStringList());
	m_pSceneImporter->setSource(QUrl::fromLocalFile(m_modelPath));
	Qt3DCore::QEntity *rootEntity = m_pSceneImporter->scene();
	m_view->setRootEntity(rootEntity);

	m_cameraEntity = m_view->camera();
	m_cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	m_cameraEntity->setPosition(m_view->getSuitableCameraPos());
	m_cameraEntity->setUpVector(QVector3D(0, 0, 0));
	m_cameraEntity->setViewCenter(QVector3D(0, 0, 0));

	m_lightEntity = new Qt3DCore::QEntity(rootEntity);
	m_light = new Qt3DRender::QPointLight(m_lightEntity);
	m_light->setColor("white");
	m_light->setIntensity(1);
	m_lightEntity->addComponent(m_light);
	m_lightTransform = new Qt3DCore::QTransform(m_lightEntity);
	m_lightTransform->setTranslation(m_view->getSuitableLightPos());
	m_lightEntity->addComponent(m_lightTransform);

	m_camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
	m_camController->setCamera(m_cameraEntity);
}

void Qt3DWindowContainer::resizeEx(const QSize &size)
{
	resize(size);
}

void Qt3DWindowContainer::showEx()
{
	show();
}

void Qt3DWindowContainer::hideEx()
{
	hide();
}

void Qt3DWindowContainer::setBgColor(const QColor &color)
{
	m_view->defaultFrameGraph()->setClearColor(color);
}

void Qt3DWindowContainer::setCameraPos(const QVector3D& cameraPos)
{
	m_cameraEntity->setPosition(cameraPos);
}

/// @brief 
/// @param w 
Q3DWindowEx::Q3DWindowEx(Qt3DWindowContainer* w)
	: Qt3DExtras::Qt3DWindow(), m_q3DWindowContainer(w), m_cameraPos(getSuitableCameraPos())
{
}

Q3DWindowEx::~Q3DWindowEx()
{
}

void Q3DWindowEx::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::RightButton)
	{
		m_mousePress = true;
		m_mousePos = e->pos();
	}
}

void Q3DWindowEx::mouseReleaseEvent(QMouseEvent* e)
{
	Q_UNUSED(e);
	m_mousePress = false;
}

void Q3DWindowEx::mouseMoveEvent(QMouseEvent* e)
{
	if (!m_mousePress)
		return;

	QPoint diff = e->pos() - m_mousePos;
	m_cameraPos.setX(m_cameraPos.x() + diff.x());
	m_cameraPos.setX(m_cameraPos.z() + diff.y());
	m_q3DWindowContainer->setCameraPos(m_cameraPos);
}

QVector3D Q3DWindowEx::getSuitableCameraPos()
{
	float maxPosition = OpenGLHelper::getModelMaxPos(m_q3DWindowContainer->getModelPath());
	QVector3D cameraPos;
	if (maxPosition <= 1)
	{
		cameraPos.setX(0.1);
		cameraPos.setY(0.1);
		cameraPos.setZ(0.1);
	}
	else if (maxPosition < 5)
	{
		cameraPos.setX(maxPosition / 5);
		cameraPos.setY(maxPosition / 5);
		cameraPos.setZ(maxPosition / 5);
	}
	else if (maxPosition < 100)
	{
		cameraPos.setX(maxPosition / 40);
		cameraPos.setY(maxPosition / 40);
		cameraPos.setZ(maxPosition / 40);
	}
	else if (maxPosition < 1000)
	{
		cameraPos.setX(maxPosition / 40);
		cameraPos.setY(maxPosition / 40);
		cameraPos.setZ(maxPosition / 40);
	}
	else if (maxPosition < 2000)
	{
		cameraPos.setX(maxPosition / 40);
		cameraPos.setY(maxPosition / 40);
		cameraPos.setZ(maxPosition / 40);
	}
	else if (maxPosition < 3000)
	{
		cameraPos.setX(maxPosition / 45);
		cameraPos.setY(maxPosition / 45);
		cameraPos.setZ(maxPosition / 45);
	}
	else
	{
		cameraPos.setX(maxPosition / 50);
		cameraPos.setY(maxPosition / 50);
		cameraPos.setZ(maxPosition / 50);
	}
	return cameraPos;
}

QVector3D Q3DWindowEx::getSuitableLightPos()
{
	float maxPosition = OpenGLHelper::getModelMaxPos(m_q3DWindowContainer->getModelPath());
	QVector3D lightTransPos;
	lightTransPos.setX(0);
	lightTransPos.setY(maxPosition * 15);
	lightTransPos.setZ(maxPosition * 15);
	return lightTransPos;
}
