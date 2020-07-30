#include "Viewport.hpp"

static QWidget* loadUiFile(QWidget* parent, QString path) {
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    QUiLoader loader;
    return loader.load(&file, parent);
}

Viewport::Viewport(QWidget* parent) : QWidget(parent), gl_widget(this), renderer_3D(this) {
    // Propagate opengl signals
    connect(&gl_widget, &OpenGLWidget::opengl_initialized, this, &Viewport::opengl_initialized);

    // Load UI
    loadUiFile(parent, "src/Viewport.ui");

    ui.setupUi(this);
    setFocusPolicy(Qt::StrongFocus);

    // Setup viewport
    QGridLayout* layout = findChild<QGridLayout*>("gridLayout");
    layout->addWidget(&gl_widget, 0, 0);

    renderer_3D.set_camera(&camera_3D);
    cam_controller.set_camera_3D(&camera_3D);
    gl_widget.set_renderer(&renderer_3D);

    mouse_captured = false;

    // Setup 3D settings ui
    settings3D = new Settings3D(this);

}

void Viewport::main_loop(float dt) {
    cam_controller.main_loop(dt);
    gl_widget.main_loop();
}

void Viewport::set_scene(Scene* scene) {
    this->scene = scene;
    renderer_3D.set_scene(scene);
}

Renderer3DOptions* Viewport::get_renderer_3D_options() {
    return renderer_3D.get_options();
}

void Viewport::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Home:
            QApplication::quit();
            break;
        case Qt::Key_Escape:
            if (mouse_captured) release_mouse();
            else capture_mouse();
            break;
        case Qt::Key_F2:
            break;
        case Qt::Key_F3:
            break;
        default:
            cam_controller.key_event(event);
            break;
    }
    event->accept();
}

void Viewport::keyReleaseEvent(QKeyEvent* event) {
    cam_controller.key_event(event);
    event->accept();
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
    if (mouse_captured) {
        QPoint screen_center = mapToGlobal(QPoint(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2));
        QPoint mouse_movement = QCursor::pos()-screen_center;
        cam_controller.mouse_moved(mouse_movement.x(), mouse_movement.y());
        QCursor::setPos(screen_center);
    }
    event->accept();
}

void Viewport::wheelEvent(QWheelEvent* event) {
    QPoint angle_delta = event->angleDelta() / 120;

    if (!angle_delta.isNull()) {
        float fov_change = static_cast<float>(angle_delta.y()) / 5;
        cam_controller.update_fov(fov_change);
    }

}

void Viewport::capture_mouse() {
    mouse_captured = true;
    QPoint screen_center = mapToGlobal(QPoint(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2));
    QCursor::setPos(screen_center);
    grabMouse(Qt::BlankCursor);
    setMouseTracking(true);
}

void Viewport::release_mouse() {
    mouse_captured = false;
    releaseMouse();
    setMouseTracking(false);
}

void Viewport::on_iterativeRenderCheckBox_toggled(bool checked)
{
    settings3D->toggle_iterative_rendering(checked, get_renderer_3D_options());
}
