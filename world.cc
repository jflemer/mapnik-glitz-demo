/**
 * This file uses a few bits from cali.cpp from:
 * http://gist.github.com/5009 / git://gist.github.com/5009.git
 * which says it was "modified from Mapnik rundemo.cpp (r727, license
 * LGPL)."
 *
 * It also is based loosly on some of the code from the Mapnik "Hello,
 * world" in Python tutorial that doesn't have a stated license:
 * http://trac.mapnik.org/wiki/GettingStarted
 *
 * It also contains a few lines to draw some shapes that came from the
 * cairomm examples directory that are also under the LGPL (L as in
 * Library in this case).
 *
 * So, though I would normally use a 2-clause BSD License, I'll say:
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with This file.  If not, see
 * <http://www.gnu.org/licenses/>.
 * 
 */

// define before any includes
#define BOOST_SPIRIT_THREADSAFE

#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>

#include <cairomm/cairommconfig.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

#include <X11/Xlib.h>
#include <glitz-glx.h>

#include "cairo-glitz-glx.h"

#include <sys/time.h>

#include <iostream>

#if 0
glitz_surface_t* glitz_get(unsigned int width, unsigned int height)
{
    glitz_drawable_format_t *dformat;
    glitz_surface_t        *surface;
    glitz_drawable_t        *drawable;
    glitz_format_t          *format;

    Display              *display;
    int                  screen;
    XSetWindowAttributes xswa = { 0 };
    Window               win;
    XVisualInfo          *vinfo;
    glitz_drawable_format_t templ = { 0 };
    templ.color.red_size = 8;
    templ.color.green_size = 8;
    templ.color.blue_size = 8;
    templ.color.alpha_size = 8;
    templ.samples = 1;
    templ.depth_size = 24;

    if ((display = XOpenDisplay (NULL)) == NULL)
    {
	fprintf (stderr, "Error: can't open %s\n", XDisplayName (NULL));
	return NULL;
    }

    screen = DefaultScreen (display);

    glitz_glx_init (NULL);

    dformat = glitz_glx_find_window_format (display, screen, 0, &templ,
        //GLITZ_FORMAT_DEPTH_SIZE_MASK |
        // GLITZ_FORMAT_SAMPLES_MASK |
        GLITZ_FORMAT_RED_SIZE_MASK |
        GLITZ_FORMAT_GREEN_SIZE_MASK |
        GLITZ_FORMAT_BLUE_SIZE_MASK |
        GLITZ_FORMAT_ALPHA_SIZE_MASK);
    if (!dformat)
    {
	fprintf (stderr, "Error: couldn't find window format\n");
	return NULL;
    }

    vinfo = glitz_glx_get_visual_info_from_format (display, screen, dformat);
    if (!vinfo) {
	fprintf (stderr, "Error: no visual info from format\n");
	return NULL;
    }

    xswa.colormap = XCreateColormap (display, RootWindow (display, screen),
				     vinfo->visual, AllocNone);

    win = XCreateWindow (display, RootWindow (display, screen),
			 0, 0, width, height,
			 0, vinfo->depth, InputOutput,
			 vinfo->visual, CWColormap, &xswa);

    XFree (vinfo);

    drawable = glitz_glx_create_drawable_for_window (display, screen,
						     dformat, win, width, height);
    format = glitz_find_standard_format (drawable, GLITZ_STANDARD_ARGB32);

    surface = glitz_surface_create(drawable, format, width, height, 0, NULL);
    if (surface == NULL)
    {
	fprintf (stderr, "Error in glitz_surface_create\n");
	return NULL;
    }
    return surface;
}
#endif

static void draw_stuff(Cairo::RefPtr<Cairo::Surface> surface, int width, int height)
{
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

    cr->save(); // save the state of the context
    cr->set_source_rgb(0.86, 0.85, 0.47);
    cr->paint();    // fill image with the color
    cr->restore();  // color is back to black now

    cr->save();
    // draw a border around the image
    cr->set_line_width(20.0);    // make the line wider
    cr->rectangle(0.0, 0.0, width, height);
    cr->stroke();

    cr->set_source_rgba(0.0, 0.0, 0.0, 0.7);
    // draw a circle in the center of the image
    cr->arc(width / 2.0, height / 2.0, 
            height / 4.0, 0.0, 2.0 * M_PI);
    cr->stroke();

    // draw a diagonal line
    cr->move_to(width / 4.0, height / 4.0);
    cr->line_to(width * 3.0 / 4.0, height * 3.0 / 4.0);
    cr->stroke();
    cr->restore();
}

static void draw_map(Cairo::RefPtr<Cairo::Surface> surface, int width = 600, int height = 300)
{
    using namespace mapnik;
        std::string mapnik_dir(getenv("MAPNIK_HOME") ? std::string(getenv("MAPNIK_HOME")) + "/lib/mapnik" : "/usr/local/lib/mapnik");
        datasource_cache::instance()->register_datasources(mapnik_dir + "/input");
        freetype_engine::register_font(mapnik_dir + "/fonts/dejavu-ttf-2.14/DejaVuSans.ttf");

        Map m(width,height,"+proj=latlong +datum=WGS84");
        m.set_background(color_factory::from_string("steelblue"));

        // create styles

        feature_type_style s;
        rule_type r;
        r.append(polygon_symbolizer(color("#f2eff9")));
        s.add_rule(r);

        m.insert_style("My Style", s);

        // Layers
        // Provincial  polygons
        {
            parameters p;
            p["type"]="shape";
            p["file"]="data/world_borders";

            Layer lyr("World");
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("My Style");
            m.addLayer(lyr);
        }

        // Get layer's featureset
        Layer lyr = m.getLayer(0);
        m.zoomToBox(lyr.envelope());

    cairo_renderer<Cairo::Surface> ren(m, surface);
    ren.apply();
}

static void draw_map(mapnik::Image32::Image32 &buf, int width = 600, int height = 300)
{
    using namespace mapnik;
        std::string mapnik_dir(getenv("MAPNIK_HOME") ? std::string(getenv("MAPNIK_HOME")) + "/lib/mapnik" : "/usr/local/lib/mapnik");
        datasource_cache::instance()->register_datasources(mapnik_dir + "/input");
        freetype_engine::register_font(mapnik_dir + "/fonts/dejavu-ttf-2.14/DejaVuSans.ttf");

        Map m(width,height,"+proj=latlong +datum=WGS84");
        m.set_background(color_factory::from_string("steelblue"));

        // create styles

        feature_type_style s;
        rule_type r;
        r.append(polygon_symbolizer(color("#f2eff9")));
        s.add_rule(r);

        m.insert_style("My Style", s);

        // Layers
        // Provincial  polygons
        {
            parameters p;
            p["type"]="shape";
            p["file"]="data/world_borders";

            Layer lyr("World");
            lyr.set_datasource(datasource_cache::instance()->create(p));
            lyr.add_style("My Style");
            m.addLayer(lyr);
        }

        // Get layer's featureset
        Layer lyr = m.getLayer(0);
        m.zoomToBox(lyr.envelope());

   agg_renderer<Image32> ren(m,buf);
   ren.apply();
}

int main ( int argc , char** argv)
{
    struct timeval t1, t2;
    int width = 600;
    int height = 300;
    try {
#if USE_AGG
    using namespace mapnik;
        Image32::Image32 buf(width, height);
        gettimeofday(&t1, NULL);
        draw_map(buf, width, height);
        gettimeofday(&t2, NULL);
        save_to_file<ImageData32>(buf.data(),"world-agg.png","png");
#elif USE_CAIRO_SVG
        Cairo::RefPtr<Cairo::SvgSurface> surface =
                Cairo::SvgSurface::create("world.svg", width, height);
        gettimeofday(&t1, NULL);
        //draw_stuff(surface, width, height);
        draw_map(surface, width, height);
        gettimeofday(&t2, NULL);
        surface->flush();
        surface->write_to_png("world-svg.png");
#elif 0
    glitz_surface_t *glitz_surf = glitz_get(width, height);
    if (glitz_surf == NULL) exit(1);
        Cairo::RefPtr<Cairo::GlitzSurface> surface =
                Cairo::GlitzSurface::create(glitz_surf);
        cairo_renderer<Cairo::Surface> ren(m, surface);
        ren.apply();
        //surface->write_to_png("world-glx.png");
        surface->flush();
        //surface->finish();
    //printf("done ... sleeping 60\n");
    //sleep(60);
#elif USE_CAIRO_GLITZ
        void *closure = NULL;
        Cairo::RefPtr<Cairo::GlitzSurface> surface = create_cairo_glitz_glx_surface(
            width, height, Cairo::CONTENT_COLOR, &closure);
        //draw_stuff(surface, width, height);
        gettimeofday(&t1, NULL);
        draw_map(surface, width, height);
        surface->flush();
        gettimeofday(&t2, NULL);
        //surface->show_page();
        surface->write_to_png("world-glx.png");
        //surface->finish();
        cleanup_cairo_glitz_glx(closure);
    //printf("done ...\n");
#endif

    }
    catch ( const mapnik::config_error & ex )
    {
            std::cerr << "### Configuration error: " << ex.what() << std::endl;
            return EXIT_FAILURE;
    }
    catch ( const std::exception & ex )
    {
            std::cerr << "### std::exception: " << ex.what() << std::endl;
            return EXIT_FAILURE;
    }
    catch ( ... )
    {
            std::cerr << "### Unknown exception." << std::endl;
            return EXIT_FAILURE;
    }
    printf("%f ms\n", 1000*(t2.tv_sec-t1.tv_sec) + (t2.tv_usec-t1.tv_usec)/1000.0);
    return EXIT_SUCCESS;
}

