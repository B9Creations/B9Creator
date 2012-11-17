#ifndef SEGMENT_H
#define SEGMENT_H
#include <QVector2D>


class Loop;


//for use with slices!
class Segment
{
public:
	QVector2D p1;
	QVector2D p2;

	QVector2D normal;

	//neighbors
	Segment* trailingSeg; //defined as the segment touching point 1
	Segment* leadingSeg; //defined as the segment touching point 2

	//loop
	Loop* pLoop;

	bool CorrectPointOrder();//swaps endpoints to match the normal vector convention



	Segment();
	Segment(QVector2D point1, QVector2D point2);

	void FormNormal();
	bool chucked;
private:


	



	
};

#endif