#include "./clipboard.h"

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>


int XCopy(Atom selection, unsigned char *text, size_t size);

Display *display;
Window window;
Atom targets_atom, text_atom, UTF8, XA_ATOM = 4, XA_STRING = 31;

int save_in_clipboard(unsigned char *text)
{
    display = XOpenDisplay(0);
    int N = DefaultScreen(display);

    XSetWindowAttributes attrs;
    window = XCreateWindow(display, RootWindow(display, N), 0, 0, 1, 1, 0, 0, InputOnly, 0, 0, &attrs);

    targets_atom = XInternAtom(display, "TARGETS", 0);
    text_atom = XInternAtom(display, "TEXT", 0);
    UTF8 = XInternAtom(display, "UTF8_STRING", 1);

    if (UTF8 == None) {
        UTF8 = XA_STRING;
    }

    Atom selection = XInternAtom(display, "CLIPBOARD", 0);

    XCopy(selection, text, strlen((char *) text));

    return 0;
}

int XCopy(Atom selection, unsigned char *text, size_t size)
{
    XEvent event;

    XSetSelectionOwner(display, selection, window, CurrentTime);

    if (XGetSelectionOwner(display, selection) != window) {
        return -1;
    }

    while (1) {
        XNextEvent(display, &event);

        switch (event.type) {
        
            case SelectionRequest:
                if (event.xselectionrequest.selection != selection) {
                    break;
                }

                XSelectionRequestEvent *xsr = &event.xselectionrequest;
                XSelectionEvent ev = {0};

                int R = 0;

                ev.type = SelectionNotify, ev.display = xsr->display, ev.requestor = xsr->requestor,
                ev.selection = xsr->selection, ev.time = xsr->time, ev.target = xsr->target, ev.property = xsr->property;

                if (ev.target == targets_atom) {
                    R = XChangeProperty(ev.display, ev.requestor, ev.property, XA_ATOM, 32, PropModeReplace, (unsigned char *) &UTF8, 1);
                } else if (ev.target == XA_STRING || ev.target == text_atom) {
                    R = XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING, 8, PropModeReplace, text, size);
                } else if (ev.target == UTF8) {
                    R = XChangeProperty(ev.display, ev.requestor, ev.property, UTF8, 8, PropModeReplace, text, size);
                } else {
                    ev.property = None;
                }

                if ((R & 2) == 0) {
                    XSendEvent(display, ev.requestor, 0, 0, (XEvent *) &ev);
                }

                break;
        
            case SelectionClear:
                return 0;
        }
    }
    
    return 0;
}
