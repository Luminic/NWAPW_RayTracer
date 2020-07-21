#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

class Renderer3D : public QObject, protected QOpenGLFunctions_4_5_Core{
    Q_OBJECT;

public:
    Renderer3D(QObject* parent=nullptr);
};

#endif