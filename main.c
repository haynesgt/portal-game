#include <math.h>
#include "game.h"
#include "map.h"
#include "glDraw.h"
#include "debug.h"

bool find_portal_spot(map_t *map, point_t start, point_t angle, double portalsize,
		point_t *portal_left_out, point_t *portal_right_out,
		point_t *p1_out, point_t *p2_out) {
	double x, y, x0, y0, xright, yright, xleft, yleft;
	point_t p1, p2;
	x = start.x;
	y = start.y;
	polygon_cast_ray(map, x, y, angle.x, angle.y, NULL, 0, &x0, &y0, &p1, &p2);
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
		return 0;
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
	return 1;
}

map_t map;

map_t portal_map;
polygon_t portals[2];
polygon_t *portal1, *portal2;

double x = 0;
double y = 0;
double xv = 0;
double yv = 0;

void update(game_t *game) {
	double xd, yd, xhit, yhit, l, dist;
	point_t p1, p2;
	bool hit;
	xd = key('D') - key('A');
	yd = key('W') - key('S');
	xv += xd;
	yv += yd;
	hit = polygon_cast_ray(&map, x, y, xv, yv, NULL, 0, &xhit, &yhit, &p1, &p2);
	xv *= 0.9;
	yv *= 0.9;
	l = norm2d(xhit - x, yhit - y);
	dist = norm2d(xv, yv);
	if (hit && l > dist) {
		// The wall is not near. Move on ahead
		x = x + xv;
		y = y + yv;
	} else {
		point_t p1p1, p1p2, p2p1, p2p2;
		point_t cull_list[4];
		p1p1 = portal1->points[1];
		p1p2 = portal1->points[0];
		p2p1 = portal2->points[1];
		p2p2 = portal2->points[0];
		cull_list[0] = p1p2;
		cull_list[1] = p1p1;
		cull_list[2] = p2p2;
		cull_list[3] = p2p1;
		hit = polygon_cast_ray(&portal_map, x, y, xv, yv, cull_list, 4, &xhit, &yhit, &p1, &p2);
		if (hit && norm2d(xhit - x, yhit - y) <= dist) {
			// We hit a portal!
			double dot, l1, l2, cos1, sin1, cos2, sin2, xv_to, yv_to;
			dot = dot2d(xhit-p1.x, yhit-p1.y, p2.x-p1.x, p2.y-p1.y);
			dot /= norm2d(p2.x-p1.x, p2.y-p1.y);
			// Angle of portal 1
			l1 = norm2d(p1p2.x-p1p1.x, p1p2.y-p1p1.y);
			cos1 = (p1p2.x-p1p1.x) / l1;
			sin1 = (p1p2.y-p1p1.y) / l1;
			l2 = norm2d(p2p2.x-p2p1.x, p2p2.y-p2p1.y);
			cos2 = (p2p2.x-p2p1.x) / l2;
			sin2 = (p2p2.y-p2p1.y) / l2;
			if (p1.x == p1p1.x && p1.y == p1p1.y && p2.x == p1p2.x && p2.y == p1p2.y ) {
				// We hit portal 1
				xhit = p2p2.x + (p2p1.x-p2p2.x) * dot / 64.0;
				yhit = p2p2.y + (p2p1.y-p2p2.y) * dot / 64.0;
				xv_to = cos1 * xv + sin1 * yv;
				yv_to =-sin1 * xv + cos1 * yv;
				xv = xv_to;
				yv = yv_to;
				xv_to =-cos2 * xv + sin2 * yv;
				yv_to =-sin2 * xv - cos2 * yv;
				xv = xv_to;
				yv = yv_to;
			}
			if (p1.x == p2p1.x && p1.y == p2p1.y && p2.x == p2p2.x && p2.y == p2p2.y ) {
				xhit = p1p2.x + (p1p1.x-p1p2.x) * dot / 64.0;
				yhit = p1p2.y + (p1p1.y-p1p2.y) * dot / 64.0;
				xv_to = cos2 * xv + sin2 * yv;
				yv_to =-sin2 * xv + cos2 * yv;
				xv = xv_to;
				yv = yv_to;
				xv_to =-cos1 * xv + sin1 * yv;
				yv_to =-sin1 * xv - cos1 * yv;
				xv = xv_to;
				yv = yv_to;
			}
			x = xhit;
			y = yhit;
		}
	}
}

double theta = 0;

void draw(game_t *game) {
	DEBUG(6, "DRAW\n");
	glPushMatrix();
	glTranslated(256, 256, 0);
	glColor3d(0.8, 1.0, 0.8);
	map_draw(&map);
	glColor3d(0.0, 0.0, 0.0);
	glDrawCircle(x, y, 4, 8, 0);
#if 0
#else
	point_t start, angle, left, right, p1, p2;
	theta += 1/60.0;
	start.x = x;
	start.y = y;
#if 0
	angle.x = cos(theta);
	angle.y = sin(theta);
#else
	angle.x = get_mouse_x() - 256 - x;
	angle.y = get_mouse_y() - 256 - y;
#endif
	if (find_portal_spot(&map, start, angle, 64, &left, &right, &p1, &p2)) {
		glDrawX(left.x, left.y, 4);
		glDrawX(right.x, right.y, 4);

		if (get_mouse_button(game, 0)) {
			DEBUG(6, "Added a portal. %lf,%lf\n", left.x, left.y);
			portal1->point_n = 2;
			portal1->points[0] = left;
			portal1->points[1] = right;
		}
		if (get_mouse_button(game, 1)) {
			DEBUG(6, "Added a portal. %lf,%lf\n", left.x, left.y);
			portal2->point_n = 2;
			portal2->points[0] = left;
			portal2->points[1] = right;
		}
	}
#endif
	DEBUG(6, "Drawing portals\n");
	if (portal1->point_n == 2) {
		glColor3d(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex2d(portal1->points[0].x, portal1->points[0].y);
		glVertex2d(portal1->points[1].x, portal1->points[1].y);
		glEnd();
	}
	if (portal2->point_n == 2) {
		glColor3d(0.0, 0.0, 1.0);
		glBegin(GL_LINES);
		glVertex2d(portal2->points[0].x, portal2->points[0].y);
		glVertex2d(portal2->points[1].x, portal2->points[1].y);
		glEnd();
	}
	DEBUG(6, "Draw them\n");
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

	portal_map.polygon_n = 2;
	portal_map.polygons = portals;
	portals[0].point_n = 0;
	portals[0].points = calloc(2, sizeof(point_t));
	portals[1].point_n = 0;
	portals[1].points = calloc(2, sizeof(point_t));
	portal1 = &portals[0];
	portal2 = &portals[1];

	map_read(&map, "gearmap.map");
	game_init(&g);
	game_run(&g);

	return EXIT_SUCCESS;
}
