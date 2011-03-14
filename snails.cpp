#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QPicture>
#include <QTimer>
#include <stdlib.h>
#include <stdio.h>

#include <fapbase.h>
#include "snails.h"

//static const char* KLogFileName = "ut_ext_mut.txt";
static const char* KLogFileName = "fap_snails_log.txt";
//static const char* KSpecFileName = "ut_spec_ext_mut.xml";
static const char* KSpecFileName = "ut_emtran_spec_2.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;
// For how many snails is the feed prepared on theirs way
static const TUint32 KMaxFeed = 2;

static const TTransInfo* tinfos[] = {NULL};

static const char* KSnailImage = "images/snail.png";


SMainWidget::SMainWidget(QWidget* parent) : QWidget(parent), iRoadWidth(3), iSnailPixmap(KSnailImage)
{
	iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
	setMinimumSize(parentWidget()->size().width(), parentWidget()->size().height());
}

SMainWidget::~SMainWidget()
{
	delete iEnv;
}

void SMainWidget::paintEvent(QPaintEvent* /*event*/)
{
	QPainter painter(this);
	painter.setPen(QPen(Qt::black, iRoadWidth, Qt::SolidLine, Qt::RoundCap));

	drawfield(&painter);
	int ctx = 0, pos = 0;
	CAE_Object* snail = (CAE_Object*) iEnv->Root()->GetNextCompByType("snail", &ctx);
	while (snail != NULL) {
	    drawsnail(&painter, pos++, snail);
	    snail = (CAE_Object*) iEnv->Root()->GetNextCompByType("snail", &ctx);
	}
}

int SMainWidget::snailsCount()
{
	return iEnv->Root()->CountCompWithType("snail");
}

void SMainWidget::drawfield(QPainter* painter)
{
	QRect wnd = painter->window();

	for (int i = 0; i<snailsCount() ; i++) {
	    float heightmult = 1.0*(i+1)/(snailsCount()+1);
	    painter->drawLine(wnd.x(), wnd.height()*heightmult, wnd.width(), wnd.height()*heightmult);
	}
}

void SMainWidget::drawsnail(QPainter* painter, int position, CAE_Object* aSnail)
{
    CAE_ConnPointBase* c_mass = aSnail->GetOutpN("mass");
    CAE_Base* b_mass = c_mass->GetSrcPin("_1");
    CAE_StateBase* sb_mass = b_mass->GetFbObj(sb_mass);
    CAE_TState<TInt>& s_mass = *sb_mass;
    TInt mass = ~s_mass;

    CAE_ConnPointBase* c_coord = aSnail->GetOutpN("coord");
    CAE_Base* b_coord = c_coord->GetSrcPin("_1");
    CAE_StateBase* sb_coord = b_coord->GetFbObj(sb_coord);
    CAE_TState<TInt>& s_coord = *sb_coord;
    TInt coord = ~s_coord;

//	int mass = ~*((CAE_TState<TUint32>*) aSnail->GetInput("mass"));
//	int coord = ~*((CAE_TState<TUint32>*) aSnail->GetOutput("coord"));
	float massmult = 0.3 + 0.7*mass/KMass_Max;
	float imageheightmult = 1.0*(snailsCount()-1)/(snailsCount())/(snailsCount());
	float posmult = 1.0*(position+1)/(snailsCount()+1);
	QRect wnd = painter->window();

	QSize nsize( wnd.height() * iSnailPixmap.width() / iSnailPixmap.height() * imageheightmult * massmult,
		wnd.height()* imageheightmult * massmult);
	QPixmap scaled = iSnailPixmap.scaled(nsize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPoint npos(coord, wnd.height()*posmult - scaled.height() - iRoadWidth/2);

	painter->drawPixmap(npos, scaled);
}

void SMainWidget::start()
{
	QTimer *timer = new QTimer(this);
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timetrigger()));
//	timer->setInterval(10);
	timer->start(500);
}

void SMainWidget::timetrigger()
{
	iEnv->Step();
	update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	QWidget* window = new QWidget;
	window->setWindowTitle("Snails");
	
	QVBoxLayout* layout = new QVBoxLayout();

	QWidget* snails = new SMainWidget(window);
	layout->addWidget(snails);

	QPushButton* startbut = new QPushButton("Start");
	QObject::connect(startbut, SIGNAL(clicked()), snails, SLOT(start()));
	layout->addWidget(startbut);

	window->setLayout(layout);
	window->show();

	return app.exec();
}
