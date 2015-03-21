#pragma once

typedef struct point {
	double x;
	double y;
} point_t;

typedef struct polygon {
	size_t point_n;
	point_t *points;
} polygon_t;

typedef struct map {
	size_t polygon_n;
	polygon_t *polygons;
} map_t;

map_t *map_read(map_t *map, const char *path);
bool polygon_cast_ray(	map_t *map,
			double x, double y,
			double dx, double dy,
			point_t *cull_list, size_t cull_n,
			double *hitx_out, double *hity_out,
			point_t *p1_out, point_t *p2_out);
