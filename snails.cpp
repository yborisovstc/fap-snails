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

static const char* KLogFileName = "ut_ext_mut.txt";
static const char* KSpecFileName = "ut_spec_ext_mut.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;
// For how many snails is the feed prepared on theirs way
static const TUint32 KMaxFeed = 3;

void update_mass(CAE_Object* aObject, CAE_State* aState);
void update_coord(CAE_Object* aObject, CAE_State* aState);

const TTransInfo KTinfo_Update_mass = TTransInfo(update_mass, "trans_mass");
const TTransInfo KTinfo_Update_coord = TTransInfo(update_coord, "trans_coord");
static const TTransInfo* tinfos[] = {&KTinfo_Update_mass, &KTinfo_Update_coord, NULL};

static const char* KSnailImage = "images/snail.png";

void update_mass(CAE_Object* /*aObject*/, CAE_State* aState)
{
    CAE_TState<TUint32> *pself = (CAE_TState<TUint32>*)aState;
    CAE_TState<TUint32> *pcoord_s = (CAE_TState<TUint32>*) aState->Input("coord_self");

    TUint32 vself = ~*pself;
    TUint32 vcoord_self = ~*pcoord_s;
    // Getting the num of others snails being forvard
    TInt feed = KMaxFeed;
    for (TInt i = 1; ; i++) {
	CAE_TState<TUint32> *pcoord_o = (CAE_TState<TUint32>*) aState->Input("coord_others", i);
	if (pcoord_o == NULL) break;
	TUint32 vcoord_o = ~*pcoord_o;
	if (vcoord_o > vcoord_self && feed > 0)
	    feed--;
    }
    TUint32 newmass = vself + feed - 1;
    if (newmass > KMass_Max) newmass = KMass_Max;
    if (newmass < KMass_Min) newmass = KMass_Min;
    *pself = newmass;
}

void update_coord(CAE_Object* /*aObject*/, CAE_State* aState)
{
    CAE_TState<TUint32> *pself = CAE_TState<TUint32>::Interpret(aState);
    CAE_TState<TUint32> *pmass_s = (CAE_TState<TUint32>*) aState->Input("mass");
    TUint32 vself = ~*pself;
    TUint32 vmass_s = ~*pmass_s;
    if (vmass_s > 0) {
    *pself = vself + KMass_Max/vmass_s;
    }
}


SMainWidget::SMainWidget(QWidget* parent) : QWidget(parent),
	iRoadWidth(3), iSnailPixmap(KSnailImage)
{
	iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
	setMinimumSize(parentWidget()->size().width(), parentWidget()->size().height());

	for (int i = 1;; i++) {
		char buf[10];
		sprintf(buf, "snail_%d", i);
		CAE_Object *snail = iEnv->Root()->GetComp(buf);
		if (snail == NULL) break;
		iSnails.append(snail);
	}
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
	for (int i = 0; i<snailsCount() ; i++) {
		drawsnail(&painter, i);
	}
}

int SMainWidget::snailsCount()
{
	return iSnails.size();
}

void SMainWidget::drawfield(QPainter* painter)
{
	QRect wnd = painter->window();

	for (int i = 0; i<snailsCount() ; i++) {
		float heightmult = 1.0*(i+1)/(snailsCount()+1);
		painter->drawLine(wnd.x(), wnd.height()*heightmult,
				wnd.width(), wnd.height()*heightmult);
	}
}

void SMainWidget::drawsnail(QPainter* painter, int position)
{
	int mass = ~*((CAE_TState<TUint32>*) iSnails.at(position)->GetInput("mass"));
	int coord = ~*((CAE_TState<TUint32>*) iSnails.at(position)->GetOutput("coord"));
	float massmult = 0.3 + 0.7*mass/KMass_Max;
	float imageheightmult = 1.0*(snailsCount()-1)/(snailsCount())/(snailsCount());
	float posmult = 1.0*(position+1)/(snailsCount()+1);
	QRect wnd = painter->window();

	QSize nsize(
		wnd.height() * iSnailPixmap.width() / iSnailPixmap.height() * imageheightmult * massmult,
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
