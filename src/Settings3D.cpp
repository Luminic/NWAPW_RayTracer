#include "Settings3D.hpp"

Settings3DWindow::Settings3DWindow(QWidget* parent) : QDockWidget(parent) {
	setWindowTitle("3D Render Settings");
	resize(800, 600);
	show();
}

Settings3DWindow::~Settings3DWindow() {

}
