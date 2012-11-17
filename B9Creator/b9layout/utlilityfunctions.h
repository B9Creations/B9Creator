#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H
#define TO_RAD 0.01745329251994329
#define SIMPLIFY_THRESH 0.001
class QVector2D;
class QVector3D;
class Segment;

//Utility Prototypes
bool IsZero(double number, double tolerance);
bool PointsShare(QVector2D point1, QVector2D point2, double tolerance);
int PointLineCompare(QVector2D pointm, QVector2D dir, QVector2D quarrypoint);//return -1,0,1 for one side of line, one line, and other side of line.
double Distance2D(QVector2D point1, QVector2D point2);
double Distance3D(QVector3D point1, QVector3D point2);
void RotateVector(QVector3D &vec, double angledeg, QVector3D axis);
bool SegmentIntersection(QVector2D &result, QVector2D seg11, QVector2D seg12, QVector2D seg21, QVector2D seg22) ;
bool SegmentsAffiliated(Segment* seg1, Segment* seg2, double epsilon);
#endif