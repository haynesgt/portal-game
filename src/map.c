#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "debug.h"

#define DEF_LINK(x, name) \
	struct name##_link { \
		struct name##_link *next; \
		x data; \
	}

DEF_LINK(point_t*, point);
typedef struct point_link point_link_t;
DEF_LINK(polygon_t *, polygon);
typedef struct polygon_link polygon_link_t;

double min(double a, double b) {
	return (a<b)?a:b;
}

double max(double a, double b) {
	return (a>b)?a:b;
}

/* File format like
M -145.46196,-2.2370729 -229.30462,94.737619 -87.883277,237.16912 20.203046,139.18432 z
M -230.31477,-26.480733 -121.2183,46.250219 -80.812207,-84.059423 -113.13708,-174.97315 z
...
*/
map_t *map_read(map_t *map, const char *path) {
	FILE *f;
	char c;
	bool relative;
	f = fopen(path, "r");

	polygon_link_t *polygon_link;
	polygon_link_t *polygon_link_last;
	size_t polygon_n;
	polygon_link_last = NULL;
	polygon_n = 0;
	while (!feof(f)) {
		do { c = fgetc(f); } while (c != 'M' && c != 'm' && c != EOF);
		if (c == EOF) break;
		relative = (c == 'm');

		point_link_t *point_link;
		point_link_t *point_link_last;
		size_t point_n;
		double xabs, yabs;
		point_link_last = NULL;
		point_n = 0;
		xabs = yabs = 0;
		for (;;) {
			point_t *point;
			point = malloc(sizeof(*point));

			// See if we are done the polygon
			c = fgetc(f);
			if (c == 'z') break;
			ungetc(c, f);
		
			// Get the coordinates of the next point
			fscanf(f, "%lf,%lf ", &point->x, &point->y);

			if (relative) {
				xabs += point->x;
				yabs += point->y;
				point->x = xabs;
				point->y = yabs;
			}

			point_link = malloc(sizeof(*point_link));
			point_link->next = point_link_last;
			point_link->data = point;
			point_link_last = point_link;
			point_n++;
			DEBUG(6, "Point: %0.5lf, %0.5lf at %p\n", point->x, point->y, point_link);
		}

		point_t *points;
		size_t i;
		points = calloc(point_n, sizeof(*points));
		DEBUG(6, "Need %zd points\n", point_n);
		for (i = point_n - 1; i < point_n; i--) {
			DEBUG(6, "Getting %p (i=%zd)\n", point_link, i);
			points[i].x = point_link->data->x;
			points[i].y = point_link->data->y;
			DEBUG(6, "Got: %0.5lf, %0.5lf\n", points[i].x, points[i].y);
			point_link_last = point_link;
			point_link = point_link->next;
			free(point_link_last);
		}

		polygon_t *polygon;
		polygon = malloc(sizeof(*polygon));
		polygon->point_n = point_n;
		polygon->points = points;

		polygon_link = malloc(sizeof(*polygon_link));
		polygon_link->next = polygon_link_last;
		polygon_link->data = polygon;
		polygon_link_last = polygon_link;
		polygon_n++;
	}

	polygon_t *polygons;
	size_t i;
	polygons = calloc(polygon_n, sizeof(*polygons));
	for (i = polygon_n - 1; i < polygon_n; i--) {
		polygons[i].point_n = polygon_link->data->point_n;
		polygons[i].points = polygon_link->data->points;
		polygon_link_last = polygon_link;
		polygon_link = polygon_link->next;
		free(polygon_link_last);
	}

	map->polygon_n = polygon_n;
	map->polygons = polygons;
	fclose(f);
	return map;
}

double dot2d(double x1, double y1, double x2, double y2) {
	return x1*x2+y1*y2;
}

bool polygon_cast_ray(	map_t *map,
			double x, double y,
			double dx, double dy,
			point_t *cull_list, size_t cull_n,
			double *hitx_out, double *hity_out,
			point_t *p1_out, point_t *p2_out) {
	size_t i, j, k;
	double dsqr_min;
	double dummyd;
	point_t dummyp;
	if (hitx_out == NULL) hitx_out = &dummyd;
	if (hity_out == NULL) hity_out = &dummyd;
	if (p1_out == NULL) p1_out = &dummyp;
	if (p2_out == NULL) p2_out = &dummyp;
	dsqr_min = 1.0/0.0;
	for (i = 0; i < map->polygon_n; i++) {
		polygon_t *polygon;
		polygon = &map->polygons[i];
		for (j = 0; j < polygon->point_n; j++) {
			point_t *p1, *p2;
			double d;
			bool cull;
			p1 = &polygon->points[j];
			p2 = &polygon->points[(j+1)%polygon->point_n];
			cull = false;
			for (k = 0; k < cull_n; k += 2) {
				if (cull_list[k].x == p1->x &&
						cull_list[k].y == p1->y &&
						cull_list[k+1].x == p2->x &&
						cull_list[k+1].y == p2->y) {
					cull = true;
					break;
				}
			}
			if (cull) continue;
			// Check if the source is pointing against the wall
			d = dot2d(dx, dy, -(p2->y-p1->y), (p2->x-p1->x));
			if (d > 0) continue;
			// Check if the wall is facing the source
			d = dot2d(x-p1->x, y-p1->y, -(p2->y-p1->y), (p2->x-p1->x));
			if (d < 0) continue;
			// Find the intersection point
			double a, dsqr, mx, my, ix, iy;
			mx = p2->x - p1->x;
			my = p2->y - p1->y;
			a = ((p1->y-y)*mx+(x-p1->x)*my) / (dy*mx - dx*my);
			ix = a*dx;
			iy = a*dy;
			dsqr = ix*ix + iy*iy;
			ix += x;
			iy += y;
			// Make sure the intersection is in the axis aligned bounding box
			if (ix < min(p1->x, p2->x) || ix > max(p1->x, p2->x) ||
					iy < min(p1->y, p2->y) || iy > max(p1->y, p2->y))
				continue;
			if (dsqr < dsqr_min) {
				*hitx_out = ix;
				*hity_out = iy;
				*p1_out = *p1;
				*p2_out = *p2;
				dsqr_min = dsqr;
			}
		}
	}
	if (dsqr_min < 1.0/0.0) {
		return 1;
	} else {
		*hitx_out = x + dx * 1000;
		*hity_out = y + dy * 1000;
		return 0;
	}
}

void map_draw(map_t *map) {
	size_t i, j;
	for (i = 0; i < map->polygon_n; i++) {
		polygon_t *polygon;
		polygon = &map->polygons[i];
		if (polygon->point_n >= 3) glBegin(GL_TRIANGLE_FAN);
		else if (polygon->point_n == 2) glBegin(GL_LINES);
		else glBegin(GL_POINTS);
		for (j = 0; j < polygon->point_n; j++) {
			point_t *p1, *p2;
			p1 = &polygon->points[j];
			p2 = &polygon->points[(j+1)%polygon->point_n];
			glVertex2d(p1->x, p1->y);
			glVertex2d(p2->x, p2->y);
		}
		glEnd();
	}
}

#if 0
int main(int argc, char *argv) {
	map_t map;
	map_read(&map, "gearmap.map");
	size_t i, j;
	for (i = 0; i < map.polygon_n; i++) {
		polygon_t *polygon;
		polygon = &map.polygons[i];
		printf("{");
		for (j = 0; j < polygon->point_n; j++) {
			double x, y;
			x = polygon->points[j].x;
			y = polygon->points[j].y;
			printf("%lf,%lf ", x, y);
		}
		printf("}\n");
	}
	polygon_cast_ray(&map, 0, 0, 1, 0, NULL, NULL, NULL, NULL);
	return 0;
}
#endif
