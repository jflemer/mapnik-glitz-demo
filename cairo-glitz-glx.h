/* NOTE: This file was hacked from an old cairo release from
 * "boilerplate/cairo-boilerplate.h" code, but the original does not
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

#include <glitz-glx.h>
#include <cairomm/surface.h>

typedef struct _glitz_target_closure_base {
    int width;
    int height;
    Cairo::Content content;
} glitz_target_closure_base_t;

typedef struct _glitz_glx_target_closure {
    glitz_target_closure_base_t base;
    Display        *dpy;
    int             scr;
    Window          win;
} glitz_glx_target_closure_t;

glitz_surface_t *create_glitz_glx_surface(glitz_format_name_t formatname, int width, int height, glitz_glx_target_closure_t *closure);

Cairo::RefPtr<Cairo::GlitzSurface> create_cairo_glitz_glx_surface(int width, int height, Cairo::Content content, void **closure);

void cleanup_cairo_glitz_glx(void *closure);
