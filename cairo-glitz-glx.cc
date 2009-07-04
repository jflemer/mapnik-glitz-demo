/* NOTE: This file was hacked from an old cairo release from
 * "boilerplate/cairo-boilerplate.c" code, but the original does not
 * exist in the git repo in this form, it only seems to exist in the
 * tarball for cairo-1.4.0.
 */

/*
 * Copyright © 2004 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include <glitz-glx.h>
#include <cairomm/surface.h>

#include "cairo-glitz-glx.h"

glitz_surface_t *
create_glitz_glx_surface (glitz_format_name_t         formatname,
              int                 width,
              int                 height,
              glitz_glx_target_closure_t *closure)
{
    Display                 * dpy = closure->dpy;
    int                       scr = closure->scr;
    glitz_drawable_format_t   templ;
    glitz_drawable_format_t * dformat = NULL;
    unsigned long             mask;
    glitz_drawable_t        * drawable = NULL;
    glitz_format_t          * format;
    glitz_surface_t         * sr;

    XSizeHints                xsh;
    XSetWindowAttributes      xswa;
    XVisualInfo             * vinfo;

    memset(&templ, 0, sizeof(templ));
    templ.color.red_size = 8;
    templ.color.green_size = 8;
    templ.color.blue_size = 8;
    templ.color.alpha_size = 8;
    templ.color.fourcc = GLITZ_FOURCC_RGB;
    templ.samples = 1;

    glitz_glx_init (NULL);

    mask = GLITZ_FORMAT_SAMPLES_MASK | GLITZ_FORMAT_FOURCC_MASK |
    GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK |
    GLITZ_FORMAT_BLUE_SIZE_MASK;
    if (formatname == GLITZ_STANDARD_ARGB32)
    mask |= GLITZ_FORMAT_ALPHA_SIZE_MASK;

    /* Try for a pbuffer first */
    if (!getenv("CAIRO_TEST_FORCE_GLITZ_WINDOW"))
    dformat = glitz_glx_find_pbuffer_format (dpy, scr, mask, &templ, 0);

    if (dformat) {
    closure->win = None;

    drawable = glitz_glx_create_pbuffer_drawable (dpy, scr, dformat,
                              width, height);
    if (!drawable)
        goto FAIL;
    //fprintf(stderr, "got pbuffer\n");
    } else {
    /* No pbuffer, try window */
    dformat = glitz_glx_find_window_format (dpy, scr, mask, &templ, 0);

    if (!dformat)
        goto FAIL;

    vinfo = glitz_glx_get_visual_info_from_format(dpy,
                              DefaultScreen(dpy),
                              dformat);

    if (!vinfo)
        goto FAIL;

    xsh.flags = PSize;
    xsh.x = 0;
    xsh.y = 0;
    xsh.width = width;
    xsh.height = height;

    xswa.colormap = XCreateColormap (dpy, RootWindow(dpy, scr),
                     vinfo->visual, AllocNone);
    closure->win = XCreateWindow (dpy, RootWindow(dpy, scr),
                      xsh.x, xsh.y, xsh.width, xsh.height,
                      0, vinfo->depth, CopyFromParent,
                      vinfo->visual, CWColormap, &xswa);
    XFree (vinfo);

    drawable =
        glitz_glx_create_drawable_for_window (dpy, scr,
                          dformat, closure->win,
                          width, height);

    if (!drawable)
        goto DESTROY_WINDOW;
    //fprintf(stderr, "got window\n");
    }

    format = glitz_find_standard_format (drawable, formatname);
    if (!format)
    goto DESTROY_DRAWABLE;

    sr = glitz_surface_create (drawable, format, width, height, 0, NULL);
    if (!sr)
    goto DESTROY_DRAWABLE;

    if (closure->win == None || dformat->doublebuffer) {
    glitz_surface_attach (sr, drawable, GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
    } else {
    XMapWindow (closure->dpy, closure->win);
    glitz_surface_attach (sr, drawable, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
    }

    glitz_drawable_destroy (drawable);

    return sr;
 DESTROY_DRAWABLE:
    glitz_drawable_destroy (drawable);
 DESTROY_WINDOW:
    if (closure->win)
    XDestroyWindow (dpy, closure->win);
 FAIL:
    return NULL;
}

Cairo::RefPtr<Cairo::GlitzSurface> create_cairo_glitz_glx_surface (int width, int height,
                Cairo::Content content,
                void    **closure)
{
    glitz_glx_target_closure_t *gxtc;
    glitz_surface_t  * glitz_surface;

    *closure = gxtc = (glitz_glx_target_closure_t *)malloc(sizeof(glitz_glx_target_closure_t));
    memset(gxtc, 0, sizeof(glitz_glx_target_closure_t));

    if (width == 0)
    width = 1;
    if (height == 0)
    height = 1;

    gxtc->dpy = XOpenDisplay (getenv("CAIRO_TEST_GLITZ_DISPLAY"));
    if (!gxtc->dpy) {
    fprintf(stderr, "Failed to open display: %s\n", XDisplayName(0));
    throw;
    }

    XSynchronize (gxtc->dpy, 1);

    gxtc->scr = DefaultScreen(gxtc->dpy);

    switch (content) {
    case CAIRO_CONTENT_COLOR:
    glitz_surface = create_glitz_glx_surface (GLITZ_STANDARD_RGB24, width, height, gxtc);
    break;
    case CAIRO_CONTENT_COLOR_ALPHA:
    glitz_surface = create_glitz_glx_surface (GLITZ_STANDARD_ARGB32, width, height, gxtc);
    break;
    default:
    fprintf(stderr, "Invalid content for glitz-glx test: %d\n", content);
    XCloseDisplay (gxtc->dpy);
        throw;
    }
    if (!glitz_surface) {
    fprintf(stderr, "Failed to create glitz-glx surface\n");
    XCloseDisplay (gxtc->dpy);
        throw;
    }

    Cairo::RefPtr<Cairo::GlitzSurface> surface = Cairo::GlitzSurface::create(glitz_surface);

    gxtc->base.width = width;
    gxtc->base.height = height;
    gxtc->base.content = content;
    //cairo_surface_set_user_data (surface, &glitz_closure_key, gxtc, NULL);
    //fprintf(stderr, "got surface\n");

    return surface;
}

void
cleanup_cairo_glitz_glx (void *closure)
{
    glitz_glx_target_closure_t *gxtc = (glitz_glx_target_closure_t *)closure;

    glitz_glx_fini ();

    if (gxtc->win)
    XDestroyWindow (gxtc->dpy, gxtc->win);

    XCloseDisplay (gxtc->dpy);

    free (gxtc);
}
