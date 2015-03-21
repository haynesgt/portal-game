#include <math.h>
#include "game.h"
#include "polygon.h"
#include "glDraw.h"

void find_portal_spot(map_t *map, point_t start, point_t angle, double portalsize,
		point_t *portal_left_out, point_t *portal_right_out,
		point_t *p1_out, point_t *p2_out) {
	double x, y, x0, y0, xright, yright, xleft, yleft;
	point_t p1, p2;
	x = start.x;
	y = start.y;
	polygon_cast_ray(map, x, y, angle.x, angle.y, NULL, 0, &x, &y, &p1, &p2);
	x0 = x;
	y0 = y;
	*p1_out = p1;
	*p2_out = p2;
	point_t plist[2];
	plist[0] = p1;
	plist[1] = p2;
	// Left (counterclockwise)
	polygon_cast_ray(map, x0, y0, (p2.x-p1.x), (p2.y-p1.y), plist, 2, &x, &y, NULL, NULL);
	xleft = x;
	yleft = y;
	// Right (clockwise)
	polygon_cast_ray(map, x0, y0, (p1.x-p2.x), (p1.y-p2.y), plist, 2, &x, &y, NULL, NULL);
	xright = x;
	yright = y;
	// Set xright and xleft to be the extents to which a portal can be placed
	if (norm2d(p1.x - x0, p1.y - y0) < norm2d(xright - x0, yright - y0)) {
		xright = p1.x;
		yright = p1.y;
	}
	if (norm2d(p2.x - x0, p2.y - y0) < norm2d(xleft - x0, yleft - y0) ) {
		xleft = p2.x;
		yleft = p2.y;
	}
	double leftnorm, rightnorm;
	leftnorm = norm2d(xleft - x0, yleft - y0);
	rightnorm = norm2d(xright - x0, yright - y0);
	if (leftnorm > portalsize / 2.0 && rightnorm > portalsize / 2.0) {
		portal_left_out->x = x0 + (xleft-x0)*portalsize/2.0/leftnorm;
		portal_left_out->y = y0+(yleft-y0)*portalsize/2.0/leftnorm;
		portal_right_out->x = x0 + (xright-x0)*portalsize/2.0/rightnorm;
		portal_right_out->y = y0+(yright-y0)*portalsize/2.0/rightnorm;
	} else if (leftnorm + rightnorm < portalsize) { // pass
	} else if (leftnorm < rightnorm) {
		portal_left_out->x = xleft;
		portal_left_out->y = yleft;
		portal_right_out->x = xleft + (xright-x0)*portalsize/rightnorm;
		portal_right_out->y = yleft + (yright-y0)*portalsize/rightnorm;
	} else {
		portal_left_out->x = xright + (xleft-x0)*portalsize/leftnorm;
		portal_left_out->y = yright + (yleft-y0)*portalsize/leftnorm;
		portal_right_out->x = xright;
		portal_right_out->y = yright;
	}
}

map_t map;

void update(void) {
}

double theta = 0;

void draw(void) {
	theta += 1/60.0;
	glPushMatrix();
	glScaled(0.8, 0.8, 0.8);
	glTranslated(256, 256, 0);
	double x, y, x0, y0, xright, yright, xleft, yleft;
	point_t p1, p2;
	x = -20;
	y = 10;
	glBegin(GL_LINES);
	glVertex2d(x, y);
	glVertex2d(x + 512.0*cos(theta), y + 512.0*sin(theta));
	glEnd();
	polygon_cast_ray(&map, x, y, cos(theta), sin(theta), NULL, 0, &x, &y, &p1, &p2);
	x0 = x;
	y0 = y;
	glColor3d(0, 1, 0);
	glBegin(GL_LINES);
	glVertex2d(x-5, y-5);
	glVertex2d(x+5, y+5);
	glEnd();
	glColor3d(0, 0, 1);
	glDrawCircle(p1.x, p1.y, 4, 8, 0);
	glColor3d(1, 0, 0);
	glDrawCircle(p2.x, p2.y, 4, 8, 0);
	//printf("%lf,%lf %lf,%lf\n", p1.x, p1.y, p2.x, p2.y);
	point_t plist[2];
	plist[0] = p1;
	plist[1] = p2;
	// Left (counterclockwise)
	polygon_cast_ray(&map, x0, y0, (p2.x-p1.x), (p2.y-p1.y), plist, 2, &x, &y, NULL, NULL);
	glColor3d(1, 0, 0);
	glDrawX(x, y, 5);
	xleft = x;
	yleft = y;
	// Right (clockwise)
	polygon_cast_ray(&map, x0, y0, (p1.x-p2.x), (p1.y-p2.y), plist, 2, &x, &y, NULL, NULL);
	glColor3d(0, 0, 1);
	glDrawX(x, y, 5);
	xright = x;
	yright = y;

	// Set xright and xleft to be the extents to which a portal can be placed
	if (norm2d(p1.x - x0, p1.y - y0) < norm2d(xright - x0, yright - y0)) {
		xright = p1.x;
		yright = p1.y;
	}
	if (norm2d(p2.x - x0, p2.y - y0) < norm2d(xleft - x0, yleft - y0) ) {
		xleft = p2.x;
		yleft = p2.y;
	}
	glDrawCircle(xleft, yleft, 8, 8, 0);
	glDrawCircle(xright, yright, 8, 8, 0);

	double portalsize = 128;

	double leftnorm, rightnorm;
	leftnorm = norm2d(xleft - x0, yleft - y0);
	rightnorm = norm2d(xright - x0, yright - y0);

	if (leftnorm > portalsize / 2.0 && rightnorm > portalsize / 2.0) {
		glDrawX(x0 + (xleft-x0)*portalsize/2.0/leftnorm,
				y0+(yleft-y0)*portalsize/2.0/leftnorm, 10.0);
		glDrawX(x0 + (xright-x0)*portalsize/2.0/rightnorm,
				y0+(yright-y0)*portalsize/2.0/rightnorm, 10.0);
	} else if (leftnorm + rightnorm < portalsize) {
	} else if (leftnorm < rightnorm) {
		glDrawX(xleft, yleft, 10.0);
		glDrawX(xleft + (xright-x0)*portalsize/rightnorm,
			yleft + (yright-y0)*portalsize/rightnorm, 10.0);
	} else {
		glDrawX(xright, yright, 10.0);
		glDrawX(xright + (xleft-x0)*portalsize/leftnorm,
				yright + (yleft-y0)*portalsize/leftnorm, 10.0);
	}
	
	glPopMatrix();
}

int main(int argc, char *argv[]) {
	game_t g;
	g.width = 640;
	g.height = 480;
	g.window_name = "Portal";
	g.update_callback = update;
	g.draw_callback = draw;
	g.disable_cursor = false;

	map_read(&map, "gearmap.map");
	game_init(&g);
	game_run(&g);

	return EXIT_SUCCESS;
}
