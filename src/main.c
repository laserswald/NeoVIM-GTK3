// Copyright (c) 2016 TheAifam5

#include <errno.h>
#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <vte/vte.h>

static void activate(GtkApplication* app, gpointer user_data);

static GtkWidget* find_child(GtkWidget* parent, const gchar* name);

static void vt_char_size_changed(VteTerminal* terminal,
                                 guint char_width,
                                 guint char_height,
                                 gpointer user_data);

static gboolean configure_event(GtkWidget* window,
                                GdkEvent* event,
                                gpointer user_data);

static void nvim_exited(VteTerminal* vteterminal,
                        gint status,
                        gpointer user_data);

static GMutex g_resize_lock;

int
main(int argc, char** argv)
{
    GtkApplication* app = NULL;
    int status;

    app = gtk_application_new("com.theaifam5.neovim-gtk3",
                              G_APPLICATION_FLAGS_NONE);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return (status);
}

static void
activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget *window = NULL, *vte = NULL;
    GError* error = NULL;

    // Initialize
    window = gtk_application_window_new(app);
    vte = vte_terminal_new();

    g_signal_connect(vte, "child-exited", G_CALLBACK(nvim_exited), NULL);

    gtk_widget_set_name(vte, "terminal");
    vte_terminal_set_size(VTE_TERMINAL(vte), 80, 45);
    gtk_window_set_title(GTK_WINDOW(window), "NeoVIM");
    gtk_container_add(GTK_CONTAINER(window), vte);

    vte_terminal_set_scrollback_lines(VTE_TERMINAL(vte), 0);
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(vte), FALSE);
    vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(vte), TRUE);
    vte_terminal_set_rewrap_on_resize(VTE_TERMINAL(vte), TRUE);
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(vte), TRUE);

    char* params[] = { "/usr/bin/nvim" };

    if (!vte_terminal_spawn_sync(
          VTE_TERMINAL(vte),
          VTE_PTY_DEFAULT,     // VtePtyFlags
          g_get_current_dir(), // const char *working_directory
          params,              // argv
          g_get_environ(),     // envv
          G_SPAWN_DEFAULT,     // GSpawnFlags
          NULL,                // GSpawnChildSetupFunc
          NULL,                // gpointer child_setup_data
          NULL,                // GPid *child_pid OUT
          NULL,                // GCancellable *cancellable
          NULL                 // GError **error
          ))
        gtk_window_close(GTK_WINDOW(window));

    GdkGeometry hints;
    hints.width_inc = vte_terminal_get_char_width(VTE_TERMINAL(vte));
    hints.height_inc = vte_terminal_get_char_height(VTE_TERMINAL(vte));

    gtk_window_set_geometry_hints(GTK_WINDOW(window), NULL, &hints,
                                  GDK_HINT_RESIZE_INC);

    gtk_widget_set_size_request(vte, -1, -1);
    gtk_widget_show_all(window);
}

static void
nvim_exited(VteTerminal* vteterminal, gint status, gpointer user_data)
{
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(vteterminal));
    if (gtk_widget_is_toplevel(toplevel))
        gtk_window_close(GTK_WINDOW(toplevel));
}

static GtkWidget*
find_child(GtkWidget* parent, const gchar* name)
{
    if (g_ascii_strncasecmp(gtk_widget_get_name((GtkWidget*) parent),
                            (gchar*) name, strlen(name))
        == 0) {
        return parent;
    }

    if (GTK_IS_BIN(parent)) {
        GtkWidget* child = gtk_bin_get_child(GTK_BIN(parent));
        return find_child(child, name);
    }

    if (GTK_IS_CONTAINER(parent)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(parent));
        while ((children = g_list_next(children)) != NULL) {
            GtkWidget* widget = find_child(children->data, name);
            if (widget != NULL) {
                return widget;
            }
        }
    }

    return NULL;
}
