#include "segment.h"
#include "math.h"
#include "qmath.h"
#include "utlilityfunctions.h"

#include <QVector2D>


Segment::Segment()
{
	p1.setX(0.0);
	p1.setY(0.0);

	p2.setX(0.0);
	p2.setY(0.0);

	normal.setX(0.0);
	normal.setY(0.0);

	trailingSeg = NULL;
	leadingSeg = NULL;

	pLoop = NULL;

	chucked = false;
}

Segment::Segment(QVector2D point1, QVector2D point2)
{
	p1.setX(point1.x());
	p1.setY(point1.y());

	p2.setX(point2.x());
	p2.setY(point2.y());

	FormNormal();

	trailingSeg = NULL;
	leadingSeg = NULL;

	pLoop = NULL;

	chucked = false;

}

void Segment::FormNormal()
{
	double dx = p2.x() - p1.x();
	double dy = p2.y() - p1.y();

	//double theta = atan2(dy,dx);
	normal.setX(-dy);
	normal.setY(dx);

	normal.normalize(); //unit vector
}



bool Segment::CorrectPointOrder()
{
	double p2x;
	double p2y;

	QVector2D center((p2.x() + p1.x())/2.0,(p2.y() + p1.y())/2.0);
	int side = PointLineCompare(center,normal, p1);//returns 1 if point is on right, -1 if point is on left
	
	if(side < 0)
	{
		return 0; //we want the point on the left
	}
	else
	{
		//swap point data!
		p2x = p2.x();
		p2y = p2.y();

		p2.setX(p1.x());
		p2.setY(p1.y());

		p1.setX(p2x);
		p1.setY(p2y);

		return 1;
	}


}