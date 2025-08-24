
#include <gtk/gtk.h>

GtkWidget *text_view;
char *current_filename = NULL;

void on_open(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    dialog = gtk_file_chooser_dialog_new("\u041e\u0442\u043a\u0440\u044b\u0442\u044c \u0444\u0430\u0439\u043b", NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_\u041e\u0442\u043c\u0435\u043d\u0430", GTK_RESPONSE_CANCEL,
        "_\u041e\u0442\u043a\u0440\u044b\u0442\u044c", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        char *contents;
        gsize length;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (g_file_get_contents(filename, &contents, &length, NULL)) {
            gtk_text_buffer_set_text(buffer, contents, length);
            g_free(contents);
            if (current_filename)
                g_free(current_filename);
            current_filename = filename;
        } else {
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
}

void on_save(GtkWidget *widget, gpointer data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (current_filename) {
        if (!g_file_set_contents(current_filename, text, -1, NULL)) {
            g_printerr("\u041e\u0448\u0438\u0431\u043a\u0430 \u043f\u0440\u0438 \u0441\u043e\u0445\u0440\u0430\u043d\u0435\u043d\u0438\u0438 \u0444\u0430\u0439\u043b\u0430: %s\n", current_filename);
        }
    } else {
        GtkWidget *dialog = gtk_file_chooser_dialog_new("\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u0444\u0430\u0439\u043b",
                                                        NULL,
                                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                                        "_Save", GTK_RESPONSE_ACCEPT,
                                                        NULL);
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            g_file_set_contents(filename, text, -1, NULL);
            if (current_filename)
                g_free(current_filename);
            current_filename = filename;
        }
        gtk_widget_destroy(dialog);
    }

    g_free(text);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *menubar;
    GtkWidget *filemenu;
    GtkWidget *fileitem;
    GtkWidget *openitem;
    GtkWidget *saveitem;
    GtkWidget *quititem;
    GtkWidget *scroll;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Simple GUI Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    menubar = gtk_menu_bar_new();
    filemenu = gtk_menu_new();

    fileitem = gtk_menu_item_new_with_label("\u0424\u0430\u0439\u043b");
    openitem = gtk_menu_item_new_with_label("\u041e\u0442\u043a\u0440\u044b\u0442\u044c");
    saveitem = gtk_menu_item_new_with_label("\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c");
    quititem = gtk_menu_item_new_with_label("\u0412\u044b\u0445\u043e\u0434");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), openitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quititem);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scroll), text_view);

    g_signal_connect(openitem, "activate", G_CALLBACK(on_open), NULL);
    g_signal_connect(saveitem, "activate", G_CALLBACK(on_save), NULL);
    g_signal_connect(quititem, "activate", G_CALLBACK(gtk_main_quit), NULL);

    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    gtk_widget_add_accelerator(openitem, "activate", accel_group,
                               GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(saveitem, "activate", accel_group,
                               GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(quititem, "activate", accel_group,
                               GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    if (argc >= 2) {
        const char *filename = argv[1];
        gchar *contents;
        gsize length;

        if (g_file_get_contents(filename, &contents, &length, NULL)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buffer, contents, length);
            g_free(contents);
            if (current_filename)
                g_free(current_filename);
            current_filename = g_strdup(filename);
        } else {
            g_printerr("\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c \u043e\u0442\u043a\u0440\u044b\u0442\u044c \u0444\u0430\u0439\u043b: %s\n", filename);
        }
    }

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

