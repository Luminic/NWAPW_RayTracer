#include "Settings4D.hpp"

SETTINGS_4D::SETTINGS_4D(Qwidget* parent) : QDockWidget(parent)
{
	setWindowTitle("4D Render Settings");
	resize(800, 600);
	show();
}

SETTINGS_4D::~SETTINGS_4D()
{
}
