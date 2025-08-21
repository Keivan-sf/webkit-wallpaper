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
#include <X11/Xlib.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <gdk/gdkx.h>   /* GDK_WINDOW_XID / gdk_x11_window_foreign_new_for_display */

char * readHtml() {
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( "index.html", "rb" );
    if( !fp ) perror("index.html"),exit(1);

    fseek( fp, 0L, SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    if( 1!=fread( buffer, lSize, 1, fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);
    fclose(fp);

    printf("%s", buffer);
    return buffer;
}

int main(int argc, char **argv) {
    /* --- Create a raw X11 window that will host the GTK/WebKit content --- */
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    const unsigned int width = 800, height = 600;
    XSetWindowAttributes swa;
    swa.override_redirect = True; /* optional: prevents WM from decorating/managing it */
    Window xwin = XCreateWindow(
                      dpy, root,
                      100, 100, width, height, 0,
                      CopyFromParent, InputOutput, CopyFromParent,
                      CWOverrideRedirect, &swa
                  );

    XLowerWindow(dpy, xwin);
    XMapWindow(dpy, xwin);
    XFlush(dpy);

    /* --- Init GTK and create a GtkWindow + WebKitWebView --- */
    gtk_init(&argc, &argv);

    GtkWidget *gtk_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(gtk_win), width, height);
    gtk_window_set_decorated(GTK_WINDOW(gtk_win), FALSE); /* no frame */

    /* Add a WebKitWebView to the GtkWindow */
    GtkWidget *webview = webkit_web_view_new();
    gtk_container_add(GTK_CONTAINER(gtk_win), webview);

    /* Simple HTML */
    const char *html2 = readHtml();
    printf("read html:%s", html2);
    // const char *html = "<html><body><h1>Well hello there!</h1></body></html>";
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview), html2, NULL);

    /* Realize the GtkWindow so its GdkWindow (X window) is created */
    gtk_widget_realize(gtk_win);

    /* --- Wrap the raw X11 window as a GdkWindow and reparent the GTK window into it --- */
    GdkDisplay *gdk_display = gdk_display_get_default();
    if (!gdk_display) {
        fprintf(stderr, "gdk_display_get_default() failed\n");
        return 1;
    }

    /* Wrap the existing X window into a GdkWindow */
    GdkWindow *parent_gdk = gdk_x11_window_foreign_new_for_display(gdk_display, xwin);
    if (!parent_gdk) {
        fprintf(stderr, "Failed to wrap X window as GdkWindow\n");
        return 1;
    }

    /* Get the GtkWindow's GdkWindow (child) and reparent it */
    GdkWindow *child_gdk = gtk_widget_get_window(gtk_win);
    if (!child_gdk) {
        fprintf(stderr, "gtk_widget_get_window() returned NULL\n");
        return 1;
    }

    /* Reparent the GTK window's GdkWindow into our wrapped X11 GdkWindow */
    gdk_window_reparent(child_gdk, parent_gdk, 0, 0);
    gdk_window_show(child_gdk);

    /* Show widgets and enter GTK main loop */
    gtk_widget_show_all(gtk_win);
    gtk_main();

    /* Cleanup (not usually reached because gtk_main() runs until quit) */
    XDestroyWindow(dpy, xwin);
    XCloseDisplay(dpy);
    return 0;
}
