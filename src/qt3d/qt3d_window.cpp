#include "qt3d_window.h"
#include "spdlog/spdlog.h"
#include <QVBoxLayout>
#include <Qt3DRender/private/qsceneimporter_p.h>
#include <Qt3DRender/private/qsceneimportfactory_p.h>
#include <QInputAspect>
#include <QForwardrenderer>

Qt3dWindow::Qt3dWindow(const QString &modelPath, const QColor &color, QWidget *parent)
	: QWidget(parent)
{
	m_modelPath = modelPath;
	if (!modelPath.isEmpty())
		init3D(color);
}

Qt3dWindow::~Qt3dWindow()
{
	if (!m_modelPath.isEmpty())
	{
		delete m_cameraEntity;
		delete m_camController;
		delete m_light;
		delete m_lightTransform;
		delete m_lightEntity;
		delete m_3dContainer;
		delete m_view;
	}
}

void Qt3dWindow::init3D(const QColor &color)
{
	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 0, 0, 0);
	m_view = new Qt3DExtras::Qt3DWindow();
	m_view->defaultFrameGraph()->setClearColor(color);
	m_3dContainer = QWidget::createWindowContainer(m_view);
	vLayout->addWidget(m_3dContainer);

	Qt3DRender::QSceneImporter *pSceneImporter = Qt3DRender::QSceneImportFactory::create("assimpEx", QStringList());
	pSceneImporter->setSource(QUrl::fromLocalFile(m_modelPath));
	Qt3DCore::QEntity *rootEntity = pSceneImporter->scene();
	m_view->setRootEntity(rootEntity);

	m_cameraEntity = m_view->camera();
	m_cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	m_cameraEntity->setPosition(QVector3D(0.0f, 0.0f, 30.0f));
	m_cameraEntity->setUpVector(QVector3D(0, 1, 0));
	m_cameraEntity->setViewCenter(QVector3D(0, 5, 0));

	m_lightEntity = new Qt3DCore::QEntity(rootEntity);
	m_light = new Qt3DRender::QPointLight(m_lightEntity);
	m_light->setColor("white");
	m_light->setIntensity(1);
	m_lightEntity->addComponent(m_light);
	m_lightTransform = new Qt3DCore::QTransform(m_lightEntity);
	m_lightTransform->setTranslation(QVector3D(0, 100000, 100000));
	m_lightEntity->addComponent(m_lightTransform);

	m_camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
	m_camController->setCamera(m_cameraEntity);
}

void Qt3dWindow::resizeEx(const QSize &size)
{
	resize(size);
}

void Qt3dWindow::showEx()
{
	show();
}

void Qt3dWindow::hideEx()
{
	hide();
}

void Qt3dWindow::setBgColor(const QColor &color)
{
	m_view->defaultFrameGraph()->setClearColor(color);
	repaint();
}
