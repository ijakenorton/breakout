#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include <math.h>
#include <stdio.h>

float dist(Vector2 point1, Vector2 point2)
{
	float distX = point1.x - point2.x;
	float distY = point1.y - point2.y;
	return sqrtf((distX * distX) + (distY * distY));
}

bool pointPoint(Vector2 point1, Vector2 point2)
{
	if (point1.x == point2.x && point1.y == point2.y) {
		return true;
	}
	return false;
}

bool point_circle(Vector2 point, Ball c)
{
	float distance = dist(point, c.centre);
	if (distance <= c.radius) {
		return true;
	}
	return false;
}

bool circle_circle(Ball circle1, Ball circle2)
{
	float distance = dist(circle1.centre, circle2.centre);
	if (distance <= circle1.radius + circle2.radius) {
		return true;
	}
	return false;
}

bool point_rect(Vector2 point, Rectangle rect)
{
	if (point.x >= rect.x && point.x <= rect.x + rect.width &&
	    point.y >= rect.y && point.y <= rect.y + rect.height) {
		return true;
	}
	return false;
}

bool rect_rect(Rectangle r1, Rectangle r2)
{
	if (r1.x + r1.width >= r2.x && // r1 right edge past r2 left
	    r1.x <= r2.x + r2.width && // r1 left edge past r2 right
	    r1.y + r1.height >= r2.y && // r1 top edge past r2 bottom
	    r1.y <= r2.y + r2.height) { // r1 bottom edge past r2 top
		return true;
	}
	return false;
}

void which_line(Ball c, Rectangle r, Line_id *line)
{
	// which edge is closest?
	if (c.centre.x < r.x) { // left edge
		line->testX = r.x;
		line->d = LEFT;

	} else if (c.centre.x > r.x + r.width) {
		line->testX = r.x;
		line->d = RIGHT; //right edge
	}

	if (c.centre.y < r.y) {
		line->testY = r.y;
		line->d = TOP; // top edge

	} else if (c.centre.y > r.y + r.height) {
		line->testY = r.y;
		line->d = BOTTOM; // bottom edge
	}
}

bool circle_paddle(Ball c, Paddle p)
{
	// Translate circle position to local space
	Vector2 circle_pos = { c.centre.x - p.rec->x, c.centre.y - p.rec->y };

	// Convert rotation to radians and negate for correct transform
	float angle = -p.rotation * DEG2RAD;

	// Transform circle position to paddle's rotated space using standard rotation matrix
	Vector2 rotated_circle_pos = {
		circle_pos.x * cosf(angle) - circle_pos.y * sinf(angle),
		circle_pos.x * sinf(angle) + circle_pos.y * cosf(angle)
	};

	float testX = rotated_circle_pos.x;
	float testY = rotated_circle_pos.y;

	// Use half dimensions for testing against centered rectangle
	float half_width = p.rec->width / 2; // Note: divided by 2 now
	float half_height = p.rec->height / 2; // Note: divided by 2 now

	// Find closest point on rectangle to circle center
	if (testX < -half_width)
		testX = -half_width; // test left edge
	else if (testX > half_width)
		testX = half_width; // right edge
	if (testY < -half_height)
		testY = -half_height; // top edge
	else if (testY > half_height)
		testY = half_height; // bottom edge

	// Calculate squared distance from closest point
	float distX = rotated_circle_pos.x - testX;
	float distY = rotated_circle_pos.y - testY;
	float distance = (distX * distX) + (distY * distY);

	// Compare squared distance with squared radius
	return distance <= (c.radius * c.radius);
}

bool circle_rect(Ball c, Rectangle r)
{
	/* Line_id line = which_line(c, r); */
	float testX = c.centre.x;
	float testY = c.centre.y;

	// which edge is closest?
	if (c.centre.x < r.x)
		testX = r.x; // test left edge
	else if (c.centre.x > r.x + r.width)
		testX = r.x + r.width; // right edge
	if (c.centre.y < r.y)
		testY = r.y; // top edge
	else if (c.centre.y > r.y + r.height)
		testY = r.y + r.height; // bottom edge

	// get distance from closest edges
	float distX = c.centre.x - testX;
	float distY = c.centre.y - testY;
	float distance = sqrt((distX * distX) + (distY * distY));

	// if the distance is less than the radius, collision!
	if (distance <= c.radius) {
		return true;
	}
	return false;
}

bool line_point(Vector2 l_start, Vector2 l_end, Vector2 p)
{
	// get distance from the point to the two ends of the line
	float d1 = dist(p, l_start);
	float d2 = dist(p, l_end);

	// get the length of the line
	float lineLen = dist(l_start, l_end);

	// since floats are so minutely accurate, add
	// a little buffer zone that will give collision
	float buffer = 0.1; // higher # = less accurate

	// if the two distances are equal to the line's
	// length, the point is on the line!
	// note we use the buffer here to give a range,
	// rather than one #
	if (d1 + d2 >= lineLen - buffer && d1 + d2 <= lineLen + buffer) {
		return true;
	}
	return false;
}

// LINE/CIRCLE
bool line_circle(Vector2 l_start, Vector2 l_end, Ball c)
{
	// is either end INSIDE the circle?
	// if so, return true immediately
	bool inside1 = point_circle(l_start, c);
	bool inside2 = point_circle(l_end, c);
	if (inside1 || inside2)
		return true;

	// get length of the line
	float distX = l_start.x - l_end.x;
	float distY = l_start.y - l_end.y;
	float len = sqrt((distX * distX) + (distY * distY));

	// get dot product of the line and circle
	float dot = (((c.centre.x - l_start.x) * (l_end.x - l_start.x)) +
		     ((c.centre.y - l_start.y) * (l_end.y - l_start.y))) /
		    pow(len, 2);

	// find the closest point on the line
	float closestX = l_start.x + (dot * (l_end.x - l_start.x));
	float closestY = l_start.y + (dot * (l_end.y - l_start.y));

	// is this point actually on the line segment?
	// if so keep going, but if not, return false
	bool onSegment = line_point(l_start, l_end,
				    (Vector2){ .x = closestX, .y = closestY });
	if (!onSegment)
		return false;

	// get distance to closest point
	distX = closestX - c.centre.x;
	distY = closestY - c.centre.y;
	float distance = sqrt((distX * distX) + (distY * distY));

	if (distance <= c.radius) {
		return true;
	}
	return false;
}

void handle_paddle_collision(Ball *ball, Paddle *paddle)
{
	// Get paddle corners and center in local space
	Vector2 paddle_center = { paddle->rec->x, paddle->rec->y };

	// Convert ball position to paddle's local space
	Vector2 local_ball = Vector2Subtract(ball->centre, paddle_center);
	local_ball = Vector2Rotate(local_ball, -paddle->rotation * DEG2RAD);

	// Determine which side of the paddle was hit
	Vector2 normal;

	// Local space collision checks
	float abs_x = fabsf(local_ball.x);
	float abs_y = fabsf(local_ball.y);

	if (abs_x > abs_y) {
		// Side collision
		normal = (Vector2){ (local_ball.x > 0) ? 1.0f : -1.0f, 0 };
	} else {
		// Top/bottom collision
		normal = (Vector2){ 0, (local_ball.y > 0) ? 1.0f : -1.0f };
	}

	// Rotate normal back to world space
	normal = Vector2Rotate(normal, paddle->rotation * DEG2RAD);
	normal = Vector2Normalize(normal);

	// Apply reflection
	ball->velocity = Vector2Reflect(ball->velocity, normal);
}
/* Vector2 get_paddle_bounce(Ball *ball, Paddle *paddle) */
/* { */
/* 	// Get paddle's normal vector (perpendicular to paddle surface) */
/* 	float paddle_angle = paddle->rotation * DEG2RAD; */

/* 	// Create unit vector pointing up */
/* 	Vector2 up = { 0.0f, -1.0f }; */

/* 	// Rotate it by paddle angle to get paddle normal */
/* 	Vector2 paddle_normal = Vector2Rotate(up, paddle_angle); */

/* 	// Normalize in case of any floating point drift */
/* 	paddle_normal = Vector2Normalize(paddle_normal); */

/* 	// Use Raylib's reflect function with normalized vectors */
/* 	return Vector2Reflect(ball->velocity, paddle_normal); */
/* } */

/* // If you want to add hit position influence: */
/* Vector2 get_paddle_bounce_with_position(Ball *ball, Paddle *paddle) */
/* { */
/* 	// Get basic reflection */
/* 	Vector2 reflected = get_paddle_bounce(ball, paddle); */

/* 	// Calculate hit position relative to paddle center (-1 to 1) */
/* 	Vector2 paddle_center = { paddle->rec->x, paddle->rec->y }; */
/* 	float hit_position = (ball->centre.x - paddle_center.x) / */
/* 			     (paddle->rec->width / 2.0f); */
/* 	hit_position = Clamp(hit_position, -1.0f, 1.0f); */

/* 	// Add some horizontal influence based on hit position */
/* 	float angle_adjustment = hit_position * (PI / 6.0f); // Max ±30 degrees */

/* 	// Rotate the reflected vector */
/* 	return Vector2Rotate(reflected, angle_adjustment); */
/* } */

/* Vector2 bounce(Vector2 n, Vector2 v) */
/* { */
/*         float dot = -2.0f * Vector2DotProduct(n, v); */
/*         Vector2Scale(-2.0f * Vector2DotProduct(n, v), n) */
/*         Ve */

/* } */

/* int main(void) */
/* { */
/* 	Vector2 point1 = { 10.0f, 10.0f }; */
/* 	Vector2 point2 = { 40.0f, 50.0f }; */

/* 	float distance = dist(point1, point2); */
/* 	printf("dist: %f", distance); */
/* } */
