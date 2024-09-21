#include "qt3d_window.h"
#include "spdlog/spdlog.h"
#include "utils/model_loader_manager.h"
#include <QVBoxLayout>
#include <QInputAspect>
#include <QForwardrenderer>
#include <QFirstpersoncameracontroller>
#include <QPointlight>
#include <QCamera>
#include <QMouseEvent>
#include <Qt3DRender/private/qsceneimportfactory_p.h>
#include <Qt3DRender/private/qsceneimporter_p.h>


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
}

void Qt3DWindowContainer::init3D(const QColor &color)
{
	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 0, 0, 0);
	m_view = new Q3DWindowEx(this);
	m_view->defaultFrameGraph()->setClearColor(color);
	m_3dContainer = QWidget::createWindowContainer(m_view);
	vLayout->addWidget(m_3dContainer);

	Qt3DRender::QSceneImporter* sceneImporter = Qt3DRender::QSceneImportFactory::create("assimpEx", QStringList()); //todo memory leak?
	sceneImporter->setSource(QUrl::fromLocalFile(m_modelPath));
	Qt3DCore::QEntity *rootEntity = sceneImporter->scene();
	m_view->setRootEntity(rootEntity);

	m_cameraEntity = m_view->camera();
	m_cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	m_cameraEntity->setPosition(m_view->getSuitableCameraPos());
	m_cameraEntity->setUpVector(QVector3D(0, 0, 0));
	m_cameraEntity->setViewCenter(QVector3D(0, 0, 0));

	Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
	Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
	light->setColor("white");
	light->setIntensity(1);
	lightEntity->addComponent(light);
	Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
	lightTransform->setTranslation(m_view->getSuitableLightPos());
	lightEntity->addComponent(lightTransform);

	Qt3DExtras::QFirstPersonCameraController* camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
	camController->setCamera(m_cameraEntity);
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
	float maxPosition = ModelLoadManager::instance()->getModelMaxPos(m_q3DWindowContainer->getModelPath());
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
	float maxPosition = ModelLoadManager::instance()->getModelMaxPos(m_q3DWindowContainer->getModelPath());
	QVector3D lightTransPos;
	lightTransPos.setX(0);
	lightTransPos.setY(maxPosition * 15);
	lightTransPos.setZ(maxPosition * 15);
	return lightTransPos;
}
