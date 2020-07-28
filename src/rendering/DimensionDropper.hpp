#ifndef DIMENSION_DROPPER_HPP
#define DIMENSION_DROPPER_HPP

#include <QObject>
#include "objects/Node.hpp"

class DimensionDropper : public QObject {
    Q_OBJECT;
public:
    DimensionDropper(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~DimensionDropper() {}

    Node* drop(Node* node4d, float slice);
};

#endif
