#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>
#include <QDebug>
#include <QMessageBox>
#include <QException>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->glWidget->installEventFilter(this);
}

bool MainWindow::Init(QSplashScreen &splash)
{
    // openal
    splash.showMessage("Initialize OpenAL...", nAlignment);
    splash.update();
    alutInit(&__argc, __argv);

    // play wav
    ALuint buffer;
    buffer = alutCreateBufferFromFile("Music.wav");
    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, true);
    alSourcePlay(source);

    splash.showMessage("Initialize OpenGL...", nAlignment);
    splash.update();
    if (false == InitGL(ui->glWidget))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("ERROR!");
        msgBox.setText("OpenGL 3.3 not supported!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        return false;
    }

    splash.showMessage("Initilaize BulletPhysics... (this may take a few minutes)", nAlignment);
    splash.update();
    if (false == InitPhysics())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("ERROR!");
        msgBox.setText("BulletPhysics: OpenCL device not Found!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        return false;
    }

    splash.showMessage("Loading Scene...", nAlignment);
    splash.update();
    if (false == InitScene(splash))
    {
        return false;
    }

    splash.clearMessage();
    splash.update();

    showMaximized();
    m_bMouseButtonDown = true;
    ui->glWidget->setCursor(Qt::BlankCursor);

    m_elapsedTimer.start();
    m_nElapsedTime = m_nCurrentTime = m_elapsedTimer.nsecsElapsed();

    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(TimerTick()));
    m_Timer.start(0);

    return true;
}

MainWindow::~MainWindow()
{
    m_Timer.stop();
    disconnect(&m_Timer, SIGNAL(timeout()), this, SLOT(TimerTick()));

    ui->glWidget->setCursor(Qt::ArrowCursor);

    ExitPhysics();
    delete ui;
}

bool MainWindow::InitGL(QWidget *pWidget)
{
    hWnd = (HWND)pWidget->winId();
    hDC = GetDC(hWnd);

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
        PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                       // Must Support Double Buffering
        PFD_TYPE_RGBA,                          // Request An RGBA Format
        32,                               // Select Our Color Depth
        0, 0, 0, 0, 0, 0,                       // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                         // Accumulation Bits Ignored
        32,                             // Z-Buffer (Depth Buffer)
        0,                              // Stencil Buffer
        0,                              // No Auxiliary Buffer
        PFD_MAIN_PLANE,                         // Main Drawing Layer
        0,                              // Reserved
        0, 0, 0                             // Layer Masks Ignored
    };

    int pf = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pf, &pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    glewInit();

    if (false == glewIsSupported("GL_VERSION_3_3"))
    {
        return false;
    }

    return true;
}

bool MainWindow::InitScene(QSplashScreen &splash)
{
    wglMakeCurrent(hDC, hRC);

    // opengl
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wglSwapIntervalEXT(0);

    m_shaderDraw.Load("Shaders/Draw.vs.txt", "Shaders/Draw.fs.txt");
    m_shaderShadowMap.Load("Shaders/DrawToDepthTexture.vs.txt", "Shaders/DrawToDepthTexture.fs.txt");
    std::vector<int> listTypes;
    listTypes.push_back(GL_RGBA32F);
    m_RenderToShadowTexture.Load(listTypes, nShadowWidth, nShadowWidth);
    m_SkyBox.Load("Scene/", "barren_bk.jpg", "barren_dn.jpg", "barren_ft.jpg", "barren_lf.jpg", "barren_rt.jpg", "barren_up.jpg");

    // Models
    // 1/2 - draw
    m_modelDraw.Load("Scene", "Scene.obj");
    m_modelDraw.CreateOpenGLBuffers();
    // 2/2 - physics
    m_modelPhysics.Load("Scene", "Physics.obj", glm::mat4(1.0f), false);
    CreateConcaveMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f, m_modelPhysics.GetVertices());

    //Model avatar;
    //avatar.Load("Scene", "Avatar.obj");
    //m_rigidbodyAvatarId = CreateConvexMesh(glm::vec3(20,3,20), glm::vec3(0,0,0), 85.0f, avatar.GetVertices());

    m_dynamicmodel.Load("Scene", "barrel.obj", glm::scale(glm::vec3(0.015f, 0.015f, 0.015f)), false);
    m_dynamicmodel.CreateOpenGLBuffers();
    for (int i = 0; i < numTextures; i++)
    {
        QString strFilename = "Scene/" + strFilenames[i];

        Texture texture;
        texture.Load(strFilename.toStdString());
        textures[i] = texture.ID();
    }

    int max = 5000;
    int curr = 0;
    for(int x = -5; x < 5; x++)
    {
        for(int z = -5; z < 5; z++)
        {
            for(int y = 0; y < (1/*100db*/ * 50/*5000db*/); y++)
            {
                float fScale = 1.5f;
                int dynamic_id = CreateConvexMesh(glm::vec3(x * fScale, 20 + (y * fScale), z * fScale), glm::vec3(0,0,0), 10.0f, m_dynamicmodel.GetVertices());
                m_listDynamicIds.push_back(dynamic_id);

                int id = rand() % numTextures;
                m_listRigidBodiesTextureId.push_back( textures[id] );

                curr++;
                std::string strMessage = "Loading Scene... (" + std::to_string(curr) + "/" + std::to_string(max) + ")";
                splash.showMessage(strMessage.c_str(), nAlignment);
                splash.update();
            }
        }
    }

    m_rigidBodyPipeline->setGravity(b3MakeVector3(0, -9.81f, 0));

    m_rigidBodyPipeline->writeAllInstancesToGpu();
    m_np->writeAllBodiesToGpu();
    m_bp->writeAabbsToGpu();

    m_Camera.Init(glm::vec3(20,3,20), glm::vec3(0,0,0));

    return true;
}

int MainWindow::CreateConvexMesh(glm::vec3 v3Position, glm::vec3 v3Rotate, float fMass, std::vector< Vertex > *pListVertices)
{
    std::vector<b3Vector3> vertices;

    for(int i = 0; i < (int)pListVertices->size(); i++)
    {
        Vertex vertex = pListVertices->at(i);

        glm::vec3 pos = vertex.v3Position;
        vertices.push_back(b3MakeVector3(pos.x, pos.y, pos.z));
    }

    b3Vector3 scaling = b3MakeVector3(1.0f, 1.0f, 1.0f);
    int colIndex = m_np->registerConvexHullShape( (float*)vertices.data() , 3 * sizeof(float), vertices.size(), scaling);

    b3Vector3 position = b3MakeVector3(v3Position.x, v3Position.y, v3Position.z);
    b3Quaternion orn(v3Rotate.x, v3Rotate.y, v3Rotate.z);

    int nRigidBodyIndex = m_rigidBodyPipeline->registerPhysicsInstance(fMass, position, orn, colIndex, 0, false);

    return nRigidBodyIndex;
}

int MainWindow::CreateConcaveMesh(glm::vec3 v3Position, glm::vec3 v3Rotate, float fMass, std::vector< Vertex > *pListVertices)
{
    b3AlignedObjectArray<b3Vector3> vertices;
    b3AlignedObjectArray<int> indices;

    for(int i = 0; i < (int)pListVertices->size(); i++)
    {
        Vertex vertex = pListVertices->at(i);

        glm::vec3 pos = vertex.v3Position + v3Position;
        vertices.push_back(b3MakeVector3(pos.x, pos.y, pos.z));

        indices.push_back(i);
    }

    b3Vector3 scaling = b3MakeVector3(1.0f, 1.0f, 1.0f);
    int colIndex = m_np->registerConcaveMesh(&vertices, &indices, scaling);

    b3Vector3 position = b3MakeVector3(0, 0, 0);
    b3Quaternion orn(v3Rotate.x, v3Rotate.y, v3Rotate.z);

    int nRigidBodyIndex = m_rigidBodyPipeline->registerPhysicsInstance(fMass, position, orn, colIndex, 0, false);

    return nRigidBodyIndex;
}

bool MainWindow::InitPhysics()
{
    if (false == initCL(-1, -1))
    {
        return false;
    }

    //m_config.m_maxConvexBodies = 100;
    //m_config.m_maxConvexShapes = m_config.m_maxConvexBodies;
    //int maxPairsPerBody = 8;
    //m_config.m_maxBroadphasePairs = maxPairsPerBody * m_config.m_maxConvexBodies;
    //m_config.m_maxContactCapacity = m_config.m_maxBroadphasePairs;

    m_np = new b3GpuNarrowPhase(m_clContext, m_clDevice, m_clQueue, m_config);
    m_bp = new b3GpuSapBroadphase(m_clContext, m_clDevice, m_clQueue);
    m_broadphaseDbvt = new b3DynamicBvhBroadphase(m_config.m_maxConvexBodies);

    m_rigidBodyPipeline = new b3GpuRigidBodyPipeline(m_clContext, m_clDevice, m_clQueue, m_np, m_bp, m_broadphaseDbvt, m_config);

    return true;
}

bool MainWindow::initCL(int preferredDeviceIndex, int preferredPlatformIndex)
{
    int ciErrNum = 0;

    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

    m_clContext = b3OpenCLUtils::createContextFromType(deviceType, &ciErrNum, 0, 0, preferredDeviceIndex, preferredPlatformIndex, &m_platformId);

    oclCHECKERROR(ciErrNum, CL_SUCCESS);

    int numDev = b3OpenCLUtils::getNumDevices(m_clContext);

    if (numDev > 0)
    {
        m_clDevice = b3OpenCLUtils::getDevice(m_clContext, 0);
        m_clQueue = clCreateCommandQueue(m_clContext, m_clDevice, 0, &ciErrNum);
        oclCHECKERROR(ciErrNum, CL_SUCCESS);

        b3OpenCLDeviceInfo info;
        b3OpenCLUtils::getDeviceInfo(m_clDevice, &info);
        m_clDeviceName = info.m_deviceName;

        return true;
    }

    return false;
}

void MainWindow::ExitPhysics()
{
    delete m_np;
    delete m_bp;
    delete m_broadphaseDbvt;
    delete m_rigidBodyPipeline;

    clReleaseCommandQueue(m_clQueue);
    clReleaseContext(m_clContext);
}

void MainWindow::TimerTick()
{
    // Update
    wglMakeCurrent(hDC, hRC);

    // getDT
    m_nElapsedTime = m_nCurrentTime;
    m_nCurrentTime = m_elapsedTimer.nsecsElapsed();
    dt = (float)(m_nCurrentTime - m_nElapsedTime) / 1000000000.0f;

    if (dt <= 0.0) { return; }
    if (dt > (1.0f / 30.0f)){ dt = 1.0f / 30.0f; }

    // print fps
    nFPS++;
    fSec += dt;
    if (fSec >= 1.0f)
    {
        this->setWindowTitle("FPS: " + QString::number(nFPS));

        nFPS = 0;
        fSec = 0.0f;
    }

    int nWidth = ui->glWidget->width();
    int nHeight = ui->glWidget->height();
    if (nWidth < 1) { nWidth = 1; }
    if (nHeight < 1) { nHeight = 1; }

    // physics
    m_rigidBodyPipeline->stepSimulation(dt);

    m_np->readbackAllBodiesToCpu();

    // mouse rotate
    if (true == m_bMouseButtonDown)
    {
        QPoint pointDiff = cursor().pos() - QPoint(width() / 2, height() / 2);
        cursor().setPos(QPoint(width() / 2, height() / 2));
        m_Camera.Rotate(pointDiff.x(), pointDiff.y());
    }
    m_Camera.Update(dt);
    glm::vec3 v3CameraPos = m_Camera.GetPos();
    glm::vec3 v3CameraAt = m_Camera.GetAt();
    glm::vec3 v3CameraDir = glm::normalize(v3CameraAt - v3CameraPos);


    // get avatar
    //b3RigidBodyData *pRigidBodies = (b3RigidBodyData*)m_np->getBodiesCpu();
    //b3RigidBodyData *rigidbodyAvatar = &(pRigidBodies[m_rigidbodyAvatarId]);
    //rigidbodyAvatar->m_quat = b3Quat(0,0,0);

    //m_Camera.SetPos(glm::vec3(rigidbodyAvatar->m_pos.x, rigidbodyAvatar->m_pos.y + 0.7f, rigidbodyAvatar->m_pos.z));
    // move
    {
        glm::vec3 dir(v3CameraDir); //dir.y = 0.0f; dir = glm::normalize(dir);
        glm::vec3 newDir(0, 0, 0);
        float fMoveVelocity = 6.0f;

        if (true == m_bKeys[Qt::Key_W])
        {
            newDir += dir;
            newDir = normalize(newDir);
        }
        else if (true == m_bKeys[Qt::Key_S])
        {
            newDir += (-dir);
            newDir = normalize(newDir);
        }
        if (true == m_bKeys[Qt::Key_D])
        {
            newDir += glm::cross(dir, glm::vec3(0,1,0));
            newDir = normalize(newDir);
        }
        else if (true == m_bKeys[Qt::Key_A])
        {
            newDir += glm::cross(-dir, glm::vec3(0,1,0));
            newDir = normalize(newDir);
        }

        glm::vec3 v3Step = newDir * fMoveVelocity * dt;
        m_Camera.SetPos(m_Camera.GetPos() + v3Step);

        //glm::vec3 v3Velocity = newDir * fMoveVelocity;
        //v3Velocity.y = rigidbodyAvatar->m_linVel.y;

        //rigidbodyAvatar->m_linVel = b3MakeVector3(v3Velocity.x, v3Velocity.y, v3Velocity.z);
    }

    //m_np->writeAllBodiesToGpu();

    glm::vec3 v3LightPos = glm::vec3(32.6785f, 85.7038f, -39.8369f);
    glm::vec3 v3LightAt = glm::vec3(0, 0, 0);
    glm::vec3 v3LightDir = glm::normalize(v3LightAt - v3LightPos);

    glm::mat4 mLightWorld = glm::mat4(1.0f);
    glm::mat4 mLightView = glm::lookAtRH(v3LightPos, v3LightAt, glm::vec3(0, 1, 0));
    glm::mat4 mLightProj = glm::orthoRH(-30.0f,30.0f, -30.0f,30.0f, 1.0f, 200.0f);

    // draw
    // 1/2 - draw to shadow texture
    m_RenderToShadowTexture.Bind();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, nShadowWidth, nShadowWidth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderShadowMap.Begin();
    glm::mat4 mWorld = glm::mat4(1.0f);
    m_shaderShadowMap.SetMatrix("matWorld", &mWorld);
    m_shaderShadowMap.SetMatrix("matView", &mLightView);
    m_shaderShadowMap.SetMatrix("matProj", &mLightProj);

    m_modelDraw.Begin(&m_shaderShadowMap);
    m_modelDraw.Draw(&m_shaderShadowMap);
    m_modelDraw.End(&m_shaderShadowMap);

    {
        b3RigidBodyData *pRigidBodies = (b3RigidBodyData*)m_np->getBodiesCpu();

        m_dynamicmodel.Begin(&m_shaderShadowMap);
        for (int j = 0; j < numTextures; j++)
        {
            m_shaderShadowMap.SetTexture("g_Texture", textures[j], 0);

            for (int i = 0; i < m_listDynamicIds.size(); i++)
            {
                if (m_listRigidBodiesTextureId.at(i) != textures[j])
                {
                    continue;
                }

                int nRigidBodyId = m_listDynamicIds.at(i);
                //b3RigidBodyData *pRigidBody = &(pRigidBodies[nRigidBodyId]);

                //b3Vector3 tr = pRigidBody->m_pos;
                //b3Quat quat = pRigidBody->m_quat;
                b3Vector3 tr;
                b3Quat quat;
                m_np->getObjectTransformFromCpu(&tr.x, &quat.x, nRigidBodyId);

                glm::mat4 mWorld = glm::translate(glm::vec3(tr.x, tr.y, tr.z) ) * glm::rotate(quat.getAngle(), glm::vec3(quat.getAxis().x, quat.getAxis().y, quat.getAxis().z));

                m_shaderShadowMap.SetMatrix("matWorld", &mWorld);
                m_dynamicmodel.Draw(&m_shaderShadowMap);
            }
        }
        m_dynamicmodel.End(&m_shaderShadowMap);
    }

    m_shaderShadowMap.End();
    m_RenderToShadowTexture.Unbind();

    // 2/2 - draw to screen with shadow
    glm::mat4 mCameraView = glm::lookAtRH(v3CameraPos, v3CameraAt, glm::vec3(0, 1, 0));
    glm::mat4 mCameraProj = glm::perspectiveRH(glm::radians(45.0f), (float)nWidth / (float)nHeight, 0.1f, 1000.0f);

    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glViewport(0, 0, nWidth, nHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderDraw.Begin();
    mWorld = glm::mat4(1.0f);
    m_shaderDraw.SetMatrix("matWorld", &mWorld);
    m_shaderDraw.SetMatrix("matView", &mCameraView);
    m_shaderDraw.SetMatrix("matProj", &mCameraProj);
    m_shaderDraw.SetMatrix("matLightView", &mLightView);
    m_shaderDraw.SetMatrix("matLightProj", &mLightProj);
    m_shaderDraw.SetVector3("lightDir", &v3LightDir);
    m_shaderDraw.SetTexture("g_DepthTexture", m_RenderToShadowTexture.GetTextureID(0), 1);

    m_modelDraw.Begin(&m_shaderDraw);
    m_modelDraw.Draw(&m_shaderDraw);
    m_modelDraw.End(&m_shaderDraw);

    {
        b3RigidBodyData *pRigidBodies = (b3RigidBodyData*)m_np->getBodiesCpu();

        m_dynamicmodel.Begin(&m_shaderDraw);
        for (int j = 0; j < numTextures; j++)
        {
            m_shaderDraw.SetTexture("g_Texture", textures[j], 0);

            for (int i = 0; i < m_listDynamicIds.size(); i++)
            {
                if (m_listRigidBodiesTextureId.at(i) != textures[j])
                {
                    continue;
                }

                int nRigidBodyId = m_listDynamicIds.at(i);
                b3RigidBodyData *pRigidBody = &(pRigidBodies[nRigidBodyId]);

                //b3Vector3 tr = pRigidBody->m_pos;
                //b3Quat quat = pRigidBody->m_quat;
                b3Vector3 tr;
                b3Quat quat;
                m_np->getObjectTransformFromCpu(&tr.x, &quat.x, nRigidBodyId);

                glm::mat4 mWorld = glm::translate(glm::vec3(tr.x, tr.y, tr.z) ) * glm::rotate(quat.getAngle(), glm::vec3(quat.getAxis().x, quat.getAxis().y, quat.getAxis().z));

                m_shaderDraw.SetMatrix("matWorld", &mWorld);
                m_dynamicmodel.Draw(&m_shaderDraw);
            }
        }
        m_dynamicmodel.End(&m_shaderDraw);
    }

    m_shaderDraw.DisableTexture(1);
    m_shaderDraw.End();

    // sky
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(mCameraProj));
    glMatrixMode(GL_MODELVIEW);
    mWorld = glm::mat4(1.0f);
    glLoadMatrixf(glm::value_ptr(mCameraView * mWorld));

    m_SkyBox.Draw(v3CameraPos, 300.0f);

    SwapBuffers(hDC);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    bool ret = QObject::eventFilter(obj, event);

    if (event->type() == QEvent::Resize)
    {
        //int nWidth = ui->glWidget->width();
        //int nHeight = ui->glWidget->height();
    }

    return ret;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int id = event->key();
    if (id >= 0 && id <= 255)
    {
        m_bKeys[id] = true;
    }

    if (Qt::Key_Escape == event->key())
    {
        m_bMouseButtonDown = false;
        ui->glWidget->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    int id = event->key();
    if (id >= 0 && id <= 255)
    {
        m_bKeys[id] = false;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    ui->glWidget->setCursor(Qt::BlankCursor);

    m_bMouseButtonDown = true;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //m_bMouseButtonDown = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
}
