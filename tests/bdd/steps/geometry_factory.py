# SPDX-License-Identifier: GPL-2.0-or-later
#
# This file is part of osm2pgsql (https://osm2pgsql.org/).
#
# Copyright (C) 2022 by the osm2pgsql developer community.
# For a full list of authors see the git log.

class GeometryFactory:
    """ Provides functions to create geometries from data grids.
    """

    def __init__(self):
        self.grid = {}


    def parse_geometry(self, geom, scene):
        """ Create a WKT SQL term for the given geometry.
            The function understands the following formats:

              <P>
                 Point geometry
              <P>,...,<P>
                 Line geometry
              (<P>,...,<P>)
                 Polygon geometry

           <P> may either be a coordinate of the form '<x> <y>' or a single
           number. In the latter case it must refer to a point in
           a previously defined grid.
        """
        if geom.find(',') < 0:
            out = "POINT({})".format(self.mk_wkt_point(geom))
        elif geom.find('(') < 0:
            out = "LINESTRING({})".format(self.mk_wkt_points(geom))
        else:
            out = "POLYGON(({}))".format(self.mk_wkt_points(geom.strip('() ')))

        return "ST_SetSRID('{}'::geometry, 4326)".format(out)


    def mk_wkt_point(self, point):
        """ Parse a point description.
            The point may either consist of 'x y' cooordinates or a number
            that refers to a grid setup.
        """
        geom = point.strip()
        if geom.find(' ') >= 0:
            return geom

        try:
            pt = self.grid_node(int(geom))
        except ValueError:
            assert False, "Scenario error: Point '{}' is not a number".format(geom)

        assert pt is not None, "Scenario error: Point '{}' not found in grid".format(geom)
        return "{} {}".format(*pt)


    def mk_wkt_points(self, geom):
        """ Parse a list of points.
            The list must be a comma-separated list of points. Points
            in coordinate and grid format may be mixed.
        """
        return ','.join([self.mk_wkt_point(x) for x in geom.split(',')])


    def set_grid(self, lines, grid_step):
        """ Replace the grid with one from the given lines.
        """
        self.grid = {}
        y = 0
        for line in lines:
            x = 0
            for pt_id in line:
                if pt_id.isdigit():
                    self.grid[int(pt_id)] = (x, y)
                x += grid_step
            y += grid_step


    def grid_node(self, nodeid):
        """ Get the coordinates for the given grid node.
        """
        return self.grid.get(nodeid)
