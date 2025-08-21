/* webkit_embed.c
 *
 * Build for X11 + GTK3 + WebKitGTK (WebKit2)
 *
 * Creates a raw X11 Window, wraps it in a GdkWindow, then reparents
 * a realized GtkWindow (containing a WebKitWebView) into that GdkWindow.
 *
 * Note: GTK3 + X11 only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <gdk/gdkx.h>

int main(int argc, char **argv) {
    char *uri = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--uri") == 0 && i + 1 < argc) {
            uri = argv[i + 1];
            break;
        }
    }

    if (!uri) {
        fprintf(stderr, "Usage: %s --uri <URI>\n", argv[0]);
        return 1;
    }

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    int width = DisplayWidth(dpy, screen);
    int height = DisplayHeight(dpy, screen);

    XSetWindowAttributes swa;
    swa.override_redirect = True; /* prevents WM from decorating/managing it */
    Window xwin = XCreateWindow(
                      dpy, root,
                      0, 0, width, height, 0,
                      CopyFromParent, InputOutput, CopyFromParent,
                      CWOverrideRedirect, &swa
                  );

    XLowerWindow(dpy, xwin);
    XMapWindow(dpy, xwin);
    XFlush(dpy);

    gtk_init(&argc, &argv);

    GtkWidget *gtk_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(gtk_win), width, height);
    gtk_window_set_decorated(GTK_WINDOW(gtk_win), FALSE); /* no frame */

    GtkWidget *webview = webkit_web_view_new();
    gtk_container_add(GTK_CONTAINER(gtk_win), webview);

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri);

    gtk_widget_realize(gtk_win);

    GdkDisplay *gdk_display = gdk_display_get_default();
    if (!gdk_display) {
        fprintf(stderr, "gdk_display_get_default() failed\n");
        return 1;
    }

    GdkWindow *parent_gdk = gdk_x11_window_foreign_new_for_display(gdk_display, xwin);
    if (!parent_gdk) {
        fprintf(stderr, "Failed to wrap X window as GdkWindow\n");
        return 1;
    }

    GdkWindow *child_gdk = gtk_widget_get_window(gtk_win);
    if (!child_gdk) {
        fprintf(stderr, "gtk_widget_get_window() returned NULL\n");
        return 1;
    }

    gdk_window_reparent(child_gdk, parent_gdk, 0, 0);
    gdk_window_show(child_gdk);

    gtk_widget_show_all(gtk_win);
    gtk_main();

    XDestroyWindow(dpy, xwin);
    XCloseDisplay(dpy);
    return 0;
}
