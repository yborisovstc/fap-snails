#ifndef SNAILS_H
#define SNAILS_H

#include <QObject>
#include <QWidget>

#include <fapext.h>

class SMainWidget: public QWidget
{
	Q_OBJECT
public:
	SMainWidget(QWidget* parent);
	~SMainWidget();
	void paintEvent(QPaintEvent* event);
public slots:
	void timetrigger();
	void start();
private:
	void drawfield(QPainter* painter);
	void drawsnail(QPainter* painter, int position, CAE_Object* aSnail);
	int snailsCount();
private:
	CAE_Env* iEnv;
	int iRoadWidth;
	QPixmap iSnailPixmap;
};

#endif // SNAILS_H

