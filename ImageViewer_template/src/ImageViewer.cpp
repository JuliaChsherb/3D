#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked())
	{
		if (w->getDrawLineActivated()) {
			w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	vW->clear();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}

// spôsob uchovania povrchovej reprezentácie: Ukazovatele do zoznamu vrcholov
bool ImageViewer::saveCubeToVTK(double s, const QString& path)
{
	//   3 ──── 2       7 ──── 6
	//   |      |       |      |
	//   0 ──── 1       4 ──── 5

	struct Vertex { double x, y, z; };

	QVector<Vertex> verts = {
		{0, 0, 0}, {s, 0, 0}, {s, s, 0}, {0, s, 0},  // V0-V3
		{0, 0, s}, {s, 0, s}, {s, s, s}, {0, s, s}   // V4-V7
	};

	struct Triangle { int v[3]; };

	QVector<Triangle> tris = {
		// (z = 0)
		{{0, 2, 1}},  {{0, 3, 2}},
		// (z = s)
		{{4, 5, 6}},  {{4, 6, 7}},
		// (y = 0)
		{{0, 1, 5}},  {{0, 5, 4}},
		// (y = s)
		{{3, 6, 2}},  {{3, 7, 6}},
		// (x = 0)
		{{0, 4, 7}},  {{0, 7, 3}},
		// (x = s)
		{{1, 2, 6}},  {{1, 6, 5}}
	};

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}

	QTextStream out(&file);

	out << "# vtk DataFile Version 3.0\n";
	out << "cube\n";
	out << "ASCII\n";
	out << "DATASET POLYDATA\n\n";

	out << "POINTS " << verts.size() << " float\n";
	for (auto& v : verts) {
		out << v.x << " " << v.y << " " << v.z << "\n";
	}

	out << "\nPOLYGONS " << tris.size() << " " << tris.size() * 4 << "\n";
	for (auto& t : tris) {
		out << "3 " << t.v[0] << " " << t.v[1] << " " << t.v[2] << "\n";
	}

	file.close();
	return true;
}

void ImageViewer::on_pushButtonGenerateCube_clicked()
{
	double size = ui->spinBoxDiceLenght->value();

	QString folderPath = "D:/JuliaEv/PG2026/3D/ImageViewer_template/VTK files";

	QDir dir;
	if (!dir.exists(folderPath)) {
		dir.mkpath(folderPath);
	}

	QString fileName = folderPath + "/cube_" + QString::number(size) + ".vtk";

	if (saveCubeToVTK(size, fileName)) {
		msgBox.setText(QString("Cube saved successfully to:\n%1").arg(fileName));
		msgBox.setIcon(QMessageBox::Information);
	}
	else {
		msgBox.setText(QString("Failed to save cube to:\n%1").arg(fileName));
		msgBox.setIcon(QMessageBox::Warning);
	}
	msgBox.exec();
}

bool ImageViewer::saveOctasphereToVTK(int subdivisions, double radius, const QString& path)
{
	struct Vertex { double x, y, z; };
	struct Triangle { int v[3]; };

	QVector<Vertex>   verts;
	QVector<Triangle> tris;

	verts.push_back({ 1,  0,  0 });  // V0
	verts.push_back({ -1,  0,  0 }); // V1
	verts.push_back({ 0,  1,  0 });  // V2
	verts.push_back({ 0, -1,  0 });  // V3
	verts.push_back({ 0,  0,  1 });  // V4
	verts.push_back({ 0,  0, -1 });  // V5

	tris.push_back({ 0, 2, 4 });
	tris.push_back({ 4, 2, 1 });
	tris.push_back({ 1, 2, 5 });
	tris.push_back({ 5, 2, 0 });
	tris.push_back({ 0, 4, 3 });
	tris.push_back({ 4, 1, 3 });
	tris.push_back({ 1, 5, 3 });
	tris.push_back({ 5, 0, 3 });

	QMap<QString, int> globalVertexIndex;

	auto addUniqueVertex = [&](const Vertex& v) -> int {
		QString key = QString("%1_%2_%3")
			.arg(v.x, 0, 'f', 12)
			.arg(v.y, 0, 'f', 12)
			.arg(v.z, 0, 'f', 12);

		if (globalVertexIndex.contains(key)) {
			return globalVertexIndex[key];
		}

		int newIndex = verts.size();
		verts.push_back(v);
		globalVertexIndex[key] = newIndex;
		return newIndex;
		};

	for (int level = 0; level < subdivisions; level++)
	{
		QVector<Triangle> newTris;

		for (int i = 0; i < tris.size(); i++)
		{
			int i0 = tris[i].v[0];
			int i1 = tris[i].v[1];
			int i2 = tris[i].v[2];

			Vertex v0 = verts[i0];
			Vertex v1 = verts[i1];
			Vertex v2 = verts[i2];

			Vertex m01 = { (v0.x + v1.x) * 0.5, (v0.y + v1.y) * 0.5, (v0.z + v1.z) * 0.5 };
			Vertex m12 = { (v1.x + v2.x) * 0.5, (v1.y + v2.y) * 0.5, (v1.z + v2.z) * 0.5 };
			Vertex m02 = { (v0.x + v2.x) * 0.5, (v0.y + v2.y) * 0.5, (v0.z + v2.z) * 0.5 };

			double len01 = sqrt(m01.x * m01.x + m01.y * m01.y + m01.z * m01.z);
			double len12 = sqrt(m12.x * m12.x + m12.y * m12.y + m12.z * m12.z);
			double len02 = sqrt(m02.x * m02.x + m02.y * m02.y + m02.z * m02.z);

			if (len01 > 0) { m01.x /= len01; m01.y /= len01; m01.z /= len01; }
			if (len12 > 0) { m12.x /= len12; m12.y /= len12; m12.z /= len12; }
			if (len02 > 0) { m02.x /= len02; m02.y /= len02; m02.z /= len02; }

			int im01 = addUniqueVertex(m01);
			int im12 = addUniqueVertex(m12);
			int im02 = addUniqueVertex(m02);

			Triangle t1 = { i0, im01, im02 };
			Triangle t2 = { im01, i1, im12 };
			Triangle t3 = { im02, im12, i2 };
			Triangle t4 = { im12, im01, im02 };

			newTris.push_back(t1);
			newTris.push_back(t2);
			newTris.push_back(t3);
			newTris.push_back(t4);
		}

		tris = newTris;
	}

	for (int i = 0; i < verts.size(); i++)
	{
		verts[i].x *= radius;
		verts[i].y *= radius;
		verts[i].z *= radius;
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&file);
	out << "# vtk DataFile Version 3.0\n";
	out << "octasphere\n";
	out << "ASCII\n";
	out << "DATASET POLYDATA\n\n";

	out << "POINTS " << verts.size() << " float\n";
	for (int i = 0; i < verts.size(); i++)
		out << verts[i].x << " " << verts[i].y << " " << verts[i].z << "\n";

	out << "\nPOLYGONS " << tris.size() << " " << tris.size() * 4 << "\n";
	for (int i = 0; i < tris.size(); i++)
		out << "3 " << tris[i].v[0] << " " << tris[i].v[1] << " " << tris[i].v[2] << "\n";

	file.close();
	return true;
}

void ImageViewer::on_pushButtonGenerateOctasphere_clicked()
{
	int    divisions = ui->spinBoxSphereDivision->value();
	double radius = ui->spinBoxSphereRadius->value();

	QString folderPath = "D:/JuliaEv/PG2026/3D/ImageViewer_template/VTK files";

	QDir dir;
	if (!dir.exists(folderPath)) {
		dir.mkpath(folderPath);
	}

	QString fileName = folderPath + QString("/octasphere_div%1_rad%2.vtk")
		.arg(divisions)
		.arg(radius);

	if (saveOctasphereToVTK(divisions, radius, fileName)) {
		msgBox.setText(QString("Octasphere saved: %1 triangles\nFile: %2")
			.arg(8 * (int)pow(4, divisions))
			.arg(fileName));
		msgBox.setIcon(QMessageBox::Information);
	}
	else {
		msgBox.setText(QString("Failed to save octasphere to:\n%1").arg(fileName));
		msgBox.setIcon(QMessageBox::Warning);
	}
	msgBox.exec();
}