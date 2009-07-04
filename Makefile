# This file is
# Copyright (c) 2009, James E. Flemer <jflemer@alum.rpi.edu>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# install the following in some $(devbase):
#   pixman-0.15.11+ (git://anongit.freedesktop.org/git/pixman)
#   cairo-1.9.1+ (git://git.cairographics.org/git/cairo)
#   cairomm-1.0.8+ (git://git.cairographics.org/git/cairomm)
#   mapnik
devbase=/home/ubuntu/glitz-base

CXXFLAGS=-g -pipe -Wall -I$(devbase)/include `pkg-config --cflags freetype2 cairomm-1.0 glitz-glx`
LDFLAGS=-L$(devbase)/lib -L/usr/local/lib -lmapnik `pkg-config --libs freetype2`
GLXLDFLAGS=`pkg-config --libs cairomm-1.0 glitz-glx`

all: world-glitz world-agg world-svg

world-glitz: cairo-glitz-glx.o world-glitz.o
	$(CXX) $(LDFLAGS) $(GLXLDFLAGS) -o $@ $^

world-agg: world-agg.o
	$(CXX) $(LDFLAGS) -o $@ $^

world-svg: world-svg.o
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	-rm -f world cairo-glitz-glx.o world.o world-glitz.o world-agg.o world-svg.o
	-rm -f world.png world-agg.png world-svg.png world-glx.png

test: all
	@echo -n "==> AGG: " && ./world-agg
	@echo -n "==> Cairo-SVG: " && ./world-svg
	@echo -n "==> Cairo-Glitz: " && ./world-glitz

DATE=$(shell date '+%Y%m%d-%H%M%S')
CALLGRIND=valgrind --tool=callgrind --demangle=no --num-callers=20 --callgrind-out-file='callgrind.out.%q{TEST}.$(DATE).txt'
profile: all
	env TEST=world-glitz $(CALLGRIND) --toggle-collect=_ZL8draw_mapN5Cairo6RefPtrINS_7SurfaceEEEii ./world-glitz
	env TEST=world-agg $(CALLGRIND) --toggle-collect=_ZL8draw_mapRN6mapnik7Image32Eii ./world-agg
	env TEST=world-svg $(CALLGRIND) --toggle-collect=_ZL8draw_mapN5Cairo6RefPtrINS_7SurfaceEEEii ./world-svg

world-glitz.o: world.cc
	$(CXX) $(CXXFLAGS) -D USE_CAIRO_GLITZ -c -o $@ $^

world-agg.o: world.cc
	$(CXX) $(CXXFLAGS) -D USE_AGG -c -o $@ $^

world-svg.o: world.cc
	$(CXX) $(CXXFLAGS) -D USE_CAIRO_SVG -c -o $@ $^
