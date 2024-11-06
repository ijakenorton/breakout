#ifndef TYPES_H
#include "types.h"
#endif // !TYPES_H

bool pointPoint(Vector2 point1, Vector2 point2);
float dist(Vector2 point1, Vector2 point2);
bool point_circle(Vector2 point, Ball c);
bool circle_circle(Ball circle1, Ball circle2);
bool point_rect(Vector2 point, Rectangle rect);
bool rect_rect(Rectangle r1, Rectangle r2);
bool circle_rect(Ball c, Rectangle r);
bool line_point(Vector2 l_start, Vector2 l_end, Vector2 p);
bool line_circle(Vector2 l_start, Vector2 l_end, Ball c);
Line_id which_line(Ball c, Rectangle r, Line_id *line);
bool circle_paddle(Ball c, Paddle p);
void handle_paddle_collision(Ball *ball, Paddle *paddle);
