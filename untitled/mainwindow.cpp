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
    int nAlignment = Qt::AlignLeft | Qt::AlignBottom;

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
    if (false == InitScene())
    {
        return false;
    }

    splash.clearMessage();
    splash.update();

    showMaximized();
    QApplication::setOverrideCursor(Qt::BlankCursor);

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

    QApplication::setOverrideCursor(Qt::ArrowCursor);

    m_SkyBox.Release();
    for(int i = 0; i < m_modelGround.count(); i++)
    {
        Model *model = m_modelGround.at(i);
        model->Release();
        delete model;
    }
    for(int i = 0; i < m_modelDestructiveObjects.count(); i++)
    {
        Model *model = m_modelDestructiveObjects.at(i);
        model->Release();
        delete model;
    }
    m_modelGround.clear();
    m_modelDestructiveObjects.clear();
    //m_modelGround.Release();
    //m_modelDestructiveObjects.Release();

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
        24,                             // Z-Buffer (Depth Buffer)
        8,                              // Stencil Buffer
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

bool MainWindow::InitScene()
{
    wglMakeCurrent(hDC, hRC);

    // opengl
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    wglSwapIntervalEXT(0);

    m_programDraw = Shader::Load("Shaders/Draw.vs.txt", "Shaders/Draw.fs.txt");
    m_programDrawToDepthTexture = Shader::Load("Shaders/DrawToDepthTexture.vs.txt", "Shaders/DrawToDepthTexture.fs.txt");
    CreateShadowMapFramebuffer();
    m_SkyBox.Load("Scene/", "barren_bk.jpg", "barren_dn.jpg", "barren_ft.jpg", "barren_lf.jpg", "barren_rt.jpg", "barren_up.jpg");

    for(int i = 0; i < 1; i++)
    {
        glm::vec3 v3Translate(i * 77.5f,0,0);

        Model *modelGround = new Model();
        modelGround->LoadFromOBJFile("Scene", "Ground.obj", v3Translate);
        modelGround->CreateOpenGLBuffers();

        Model *modelDestructiveObjects = new Model();
        modelDestructiveObjects->LoadFromOBJFile("Scene", "DestructiveObjects.obj", v3Translate);
        modelDestructiveObjects->CreateOpenGLBuffers();

        CreateConcaveMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f, modelGround->GetVertices());
        CreateConcaveMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f, modelDestructiveObjects->GetVertices());

        m_modelGround.push_back(modelGround);
        m_modelDestructiveObjects.push_back(modelDestructiveObjects);
    }
    //m_modelGround.LoadFromOBJFile("Scene", "Ground.obj");
    //m_modelGround.CreateOpenGLBuffers();
    //m_modelDestructiveObjects.LoadFromOBJFile("Scene", "DestructiveObjects.obj");
    //m_modelDestructiveObjects.CreateOpenGLBuffers();
    //CreateConcaveMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f, m_modelGround.GetVertices());
    //CreateConcaveMesh(glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f, m_modelDestructiveObjects.GetVertices());

    Model avatar;
    avatar.LoadFromOBJFile("Scene", "Avatar.obj");
    m_rigidbodyAvatarId = CreateConvexMesh(glm::vec3(20,3,20), glm::vec3(0,0,0), 85.0f, avatar.GetVertices());

    m_rigidBodyPipeline->setGravity(b3MakeVector3(0, -9.81f, 0));

    m_rigidBodyPipeline->writeAllInstancesToGpu();
    m_np->writeAllBodiesToGpu();
    m_bp->writeAabbsToGpu();

    m_Camera.Init(glm::vec3(0,8,20), glm::vec3(0,0,0));

    // openal
    //ALuint buffer;
    //buffer = alutCreateBufferFromFile("Music.wav");
    //ALuint source;
    //alGenSources(1, &source);
    //alSourcei(source, AL_BUFFER, buffer);
    //alSourcei(source, AL_LOOPING, true);
    //alSourcePlay(source);

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

    m_config.m_maxConvexBodies = 10000;
    m_config.m_maxConvexShapes = m_config.m_maxConvexBodies;
    int maxPairsPerBody = 8;
    m_config.m_maxBroadphasePairs = maxPairsPerBody * m_config.m_maxConvexBodies;
    m_config.m_maxContactCapacity = m_config.m_maxBroadphasePairs;

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
    QPoint pointDiff = cursor().pos() - QPoint(width() / 2, height() / 2);
    cursor().setPos(QPoint(width() / 2, height() / 2));
    m_Camera.Rotate(pointDiff.x(), pointDiff.y());
    m_Camera.Update(dt);
    glm::vec3 v3CameraPos = m_Camera.GetPos();
    glm::vec3 v3CameraAt = m_Camera.GetAt();
    glm::vec3 v3CameraDir = glm::normalize(v3CameraAt - v3CameraPos);

    // get avatar
    b3RigidBodyData *pRigidBodies = (b3RigidBodyData*)m_np->getBodiesCpu();//getRigidBodyDataCpu();
    b3RigidBodyData *rigidbodyAvatar = &(pRigidBodies[m_rigidbodyAvatarId]);
    rigidbodyAvatar->m_quat = b3Quat(0,0,0);
    m_Camera.SetPos(glm::vec3(rigidbodyAvatar->m_pos.x, rigidbodyAvatar->m_pos.y + 0.7f, rigidbodyAvatar->m_pos.z));
    // move
    {
        glm::vec3 dir(v3CameraDir); dir.y = 0.0f; dir = glm::normalize(dir);
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

        glm::vec3 v3Velocity = newDir * fMoveVelocity;
        v3Velocity.y = rigidbodyAvatar->m_linVel.y;

        float fDamping = 0.9f;
        v3Velocity.x *= fDamping;
        v3Velocity.z *= fDamping;

        rigidbodyAvatar->m_linVel = b3MakeVector3(v3Velocity.x, v3Velocity.y, v3Velocity.z);
    }

    m_np->writeAllBodiesToGpu();

    glm::vec3 v3LightPos = glm::vec3(32.6785f, 85.7038f, -39.8369f);
    glm::vec3 v3LightAt = glm::vec3(0, 0, 0);
    glm::vec3 v3LightDir = glm::normalize(v3LightAt - v3LightPos);

    glm::mat4 mLightWorld = glm::mat4(1.0f);
    glm::mat4 mLightView = glm::lookAtRH(v3LightPos, v3LightAt, glm::vec3(0, 1, 0));
    glm::mat4 mLightProj = glm::orthoRH(-30.0f,30.0f, -30.0f,30.0f, 1.0f, 200.0f);

    // Draw
    // draw to shadow texture
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLenum arrDrawBuffers1[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, arrDrawBuffers1);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, nShadowWidth, nShadowWidth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_programDrawToDepthTexture);

    glUniformMatrix4fv(glGetUniformLocation(m_programDrawToDepthTexture, "matWorld"), 1, GL_FALSE, glm::value_ptr(mLightWorld));
    glUniformMatrix4fv(glGetUniformLocation(m_programDrawToDepthTexture, "matView"), 1, GL_FALSE, glm::value_ptr(mLightView));
    glUniformMatrix4fv(glGetUniformLocation(m_programDrawToDepthTexture, "matProj"), 1, GL_FALSE, glm::value_ptr(mLightProj));

    for(int i = 0; i < m_modelGround.count(); i++)
    {
        m_modelGround.at(i)->Draw(m_programDrawToDepthTexture);
    }
    for(int i = 0; i < m_modelDestructiveObjects.count(); i++)
    {
        m_modelDestructiveObjects.at(i)->Draw(m_programDrawToDepthTexture);
    }
    //m_modelGround.Draw(m_programDrawToDepthTexture);
    //m_modelDestructiveObjects.Draw(m_programDrawToDepthTexture);

    glUseProgram(0);

    // draw to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffers(1, arrDrawBuffers1);

    glm::mat4 mCameraWorld = glm::mat4(1.0f);
    glm::mat4 mCameraView = glm::lookAtRH(v3CameraPos, v3CameraAt, glm::vec3(0, 1, 0));
    glm::mat4 mCameraProj = glm::perspectiveRH(glm::radians(45.0f), (float)nWidth / (float)nHeight, 0.1f, 1000.0f);

    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glViewport(0, 0, nWidth, nHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_programDraw);

    glUniformMatrix4fv(glGetUniformLocation(m_programDraw, "matCameraWorld"), 1, GL_FALSE, glm::value_ptr(mCameraWorld));
    glUniformMatrix4fv(glGetUniformLocation(m_programDraw, "matCameraView"), 1, GL_FALSE, glm::value_ptr(mCameraView));
    glUniformMatrix4fv(glGetUniformLocation(m_programDraw, "matCameraProj"), 1, GL_FALSE, glm::value_ptr(mCameraProj));

    glUniformMatrix4fv(glGetUniformLocation(m_programDraw, "matLightWorld"), 1, GL_FALSE, glm::value_ptr(mLightWorld));
    glUniformMatrix4fv(glGetUniformLocation(m_programDraw, "matLightView"), 1, GL_FALSE, glm::value_ptr(mLightView));
    glUniformMatrix4fv(glGetUniformLocation(m_programDraw, "matLightProj"), 1, GL_FALSE, glm::value_ptr(mLightProj));

    glUniform3fv(glGetUniformLocation(m_programDraw, "lightDir"), 1, glm::value_ptr(v3LightDir));

    glUniform1i(glGetUniformLocation(m_programDraw, "g_DepthTexture"), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    for(int i = 0; i < m_modelGround.count(); i++)
    {
        m_modelGround.at(i)->Draw(m_programDrawToDepthTexture);
    }
    for(int i = 0; i < m_modelDestructiveObjects.count(); i++)
    {
        m_modelDestructiveObjects.at(i)->Draw(m_programDrawToDepthTexture);
    }
    //m_modelGround.Draw(m_programDraw);
    //m_modelDestructiveObjects.Draw(m_programDraw);


    glUseProgram(0);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(mCameraProj));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(mCameraView * mCameraWorld));

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
        close();
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
    m_bMouseButtonDown = true;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_bMouseButtonDown = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
}

void MainWindow::CreateShadowMapFramebuffer()
{
    nShadowWidth = 2048;

    if (0 != framebuffer)
    {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }

    if (0 != depthBuffer)
    {
        glDeleteTextures(1, &depthBuffer);
        depthBuffer = 0;
    }

    if (0 != depthTexture)
    {
        glDeleteTextures(1, &depthTexture);
        depthTexture = 0;
    }

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, nShadowWidth, nShadowWidth, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthTexture, 0);

    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, nShadowWidth, nShadowWidth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
}
