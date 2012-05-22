/**
 * FreeRDP: A Remote Desktop Protocol Client
 * X11 Windows
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <tsol/label.h>

#include "xf_window.h"

void xf_ConfigureLabel(xfInfo *xfi, xfWindow *xfw) 
{
	XGCValues gcv;
	m_label_t *plabel;
	XFontStruct *font;
	XColor bg_color;
	char *color_name;
	char *label_name;
	Status status;
	int dir_return;
	int ascent_return;
	int descent_return;
	XCharStruct overall_return;

	rdpWindow *rdp = xfw->window;
/*
	printf("window=0x%X offset=%d,%d delta=%d,%d, clientArea= %d,%d %d+%d  visible=%d,%d  size=%d+%d\n",
		xfw->handle,
		rdp->windowOffsetX, rdp->windowOffsetY,
		rdp->windowClientDeltaX, rdp->windowClientDeltaY,
		rdp->clientOffsetX, rdp->clientOffsetY, rdp->clientAreaWidth, rdp->clientAreaHeight,
		rdp->visibleOffsetX, rdp->visibleOffsetY,
		rdp->windowWidth, rdp->windowHeight);
*/
	if (xfw->is_transient) 
	{
		return;
	}

	/* Look up the label name and color */

	plabel = m_label_alloc(MAC_LABEL);

	if (getplabel(plabel) == -1) 
	{
		perror("getplabel");
		exit(1);
	}	

	if (label_to_str(plabel, &color_name, M_COLOR, 0) != 0) {
		perror("label_to_string(M_COLOR)");
		exit(1);
	}

	if (label_to_str(plabel, &label_name, M_LABEL, DEF_NAMES) != 0) {
		perror("label_to_str(M_LABEL)");
		exit(1);
	}

	m_label_free(plabel);

	xfw->label.label_name = label_name;

	bg_color.pixel = 0;
	status = XParseColor(xfi->display, DefaultColormap(xfi->display, xfi->screen_number), color_name, &bg_color);
	if (status == BadColor) {
		perror(color_name);
		exit(1);
	}
	status = XAllocColor(xfi->display, DefaultColormap(xfi->display, xfi->screen_number), &bg_color);
	if (status == BadColor) {
		perror(color_name);
		exit(1);
	}
	xfree(color_name);

	gcv.foreground = BlackPixel(xfi->display, DefaultScreen(xfi->display));
	gcv.background = bg_color.pixel;
	gcv.background = BlackPixel(xfi->display, DefaultScreen(xfi->display));

      	xfw->label.gc = XCreateGC(xfi->display, xfw->handle, GCForeground | GCBackground, &gcv);
	XQueryTextExtents(xfi->display, XGContextFromGC(xfw->label.gc), // xfw->label.font->fid, 
		label_name, strlen(label_name), 
		&dir_return, &ascent_return, &descent_return, &overall_return);

	xfw->label.pad = 3;
	xfw->label.height = overall_return.ascent + overall_return.descent + xfw->label.pad * 2;
	xfw->label.y = overall_return.ascent;
	xfw->label.width = overall_return.width;

	XSetWindowBackground(xfi->display, xfw->handle, bg_color.pixel);

}

void xf_DrawLabel(xfInfo *xfi, xfWindow *xfw, int x, int y, int width, int height)
{
	XRectangle rect;

	if (! xfw->is_transient && xfw->label.label_name && y < 0)
	{
		y += xfw->label.height;

		if (y + height >= xfw->label.height)
		{
			height = xfw->label.height - y;
		}
		rect.x = x;
		rect.y = y;
		rect.width = width;
		rect.height = height;

		XSetClipRectangles(xfi->display, xfw->label.gc, 0, 0, &rect, 1, Unsorted);

		XClearArea(xfi->display, xfw->handle, 
			x, y, width, height, False);

		XDrawString(xfi->display, xfw->handle, xfw->label.gc,
			(xfw->width / 2) - (xfw->label.width / 2), 
			xfw->label.height - xfw->label.pad,
			xfw->label.label_name, strlen(xfw->label.label_name));
	}
}

