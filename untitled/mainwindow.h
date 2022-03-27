#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "GL/glew.h"
#include "GL/wglew.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "AL/alut.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "SkyBox.h"
#include "RenderTarget.h"

#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"
#include "Bullet3OpenCL/RigidBody/b3GpuRigidBodyPipeline.h"
#include "Bullet3OpenCL/RigidBody/b3GpuNarrowPhase.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuSapBroadphase.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3Config.h"
#include "Bullet3Collision/BroadPhaseCollision/b3DynamicBvhBroadphase.h"
#include "Bullet3Collision/NarrowPhaseCollision/shared/b3RigidBodyData.h"

#include <Windows.h>

#include <QSplashScreen>
#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool Init(QSplashScreen &splash);

private slots:
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void TimerTick();

private:
    Ui::MainWindow *ui;

    // opengl
    bool InitGL(QWidget *pWidget);

    // physics
    bool InitPhysics();
    void ExitPhysics();
    bool initCL(int preferredDeviceIndex, int preferredPlatformIndex);
    int CreateConvexMesh(glm::vec3 v3Position, glm::vec3 v3Rotate, float fMass, std::vector< Vertex > *pListVertices);
    int CreateConcaveMesh(glm::vec3 v3Position, glm::vec3 v3Rotate, float fMass, std::vector< Vertex > *pListVertices);

    // scene
    bool InitScene();

    // OpenGL
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;

    // SkyBox
    SkyBox m_SkyBox;

    // Model
    Model m_modelDraw;
    Model m_modelPhysics;

    // shader
    Shader m_shaderDraw;
    Shader m_shaderShadowMap;

    // RenderTarget
    RenderTarget m_RenderToShadowTexture;

    // avatar
    int m_rigidbodyGroundId;
    int m_rigidbodyAvatarId;

    // SYSTEM
    QTimer m_Timer;
    Camera m_Camera;
    bool   m_bKeys[256];
    bool m_bMouseButtonDown = false;

    // FPS
    QElapsedTimer m_elapsedTimer;
    uint64_t m_nElapsedTime;
    uint64_t m_nCurrentTime;
    float dt;
    int nFPS = 0;
    float fSec = 0;

    int nShadowWidth = 2048;


    // bullet physics
    cl_platform_id m_platformId;
    cl_context m_clContext;
    cl_device_id m_clDevice;
    cl_command_queue m_clQueue;
    char* m_clDeviceName;

    b3Config m_config;
    b3GpuNarrowPhase* m_np;
    b3GpuBroadphaseInterface* m_bp;
    b3DynamicBvhBroadphase* m_broadphaseDbvt;
    b3GpuRigidBodyPipeline* m_rigidBodyPipeline;
};
#endif // MAINWINDOW_H
