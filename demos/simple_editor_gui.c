
#include <gtksourceview/gtksource.h>
#include <gtk/gtk.h>

GtkWidget *text_view;
char *current_filename = NULL;

// Установить язык по расширению
void set_language_from_filename(const char *filename, GtkSourceBuffer *buffer) {
    GtkSourceLanguageManager *lm = gtk_source_language_manager_get_default();
    GtkSourceLanguage *lang = gtk_source_language_manager_guess_language(lm, filename, NULL);
    if (lang) {
        gtk_source_buffer_set_language(buffer, lang);
    }
}

void load_file(const char *filename) {
    gchar *contents;
    gsize length;

    if (g_file_get_contents(filename, &contents, &length, NULL)) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_set_text(buffer, contents, length);
        set_language_from_filename(filename, GTK_SOURCE_BUFFER(buffer));

        if (current_filename)
            g_free(current_filename);
        current_filename = g_strdup(filename);

        g_free(contents);
    } else {
        g_printerr("Не удалось открыть файл: %s\n", filename);
    }
}

void on_open(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Открыть файл", NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Отмена", GTK_RESPONSE_CANCEL,
        "_Открыть", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        load_file(filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void save_to_file(const char *filename) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    g_file_set_contents(filename, text, -1, NULL);
    g_free(text);

    if (current_filename)
        g_free(current_filename);
    current_filename = g_strdup(filename);
}

void on_save(GtkWidget *widget, gpointer data) {
    if (current_filename) {
        save_to_file(current_filename);
    } else {
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Сохранить файл", NULL,
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "_Отмена", GTK_RESPONSE_CANCEL,
            "_Сохранить", GTK_RESPONSE_ACCEPT,
            NULL);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            save_to_file(filename);
            g_free(filename);
        }

        gtk_widget_destroy(dialog);
    }
}

void on_save_as(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Сохранить как", NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Отмена", GTK_RESPONSE_CANCEL,
        "_Сохранить", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        save_to_file(filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

GtkAccelGroup *add_shortcuts(GtkWidget *window) {
    GtkAccelGroup *accel_group = gtk_accel_group_new();

    gtk_widget_add_accelerator(window, "activate", accel_group,
        GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    return accel_group;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GUI Editor with Syntax Highlighting");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Меню
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *filemenu = gtk_menu_new();
    GtkWidget *fileitem = gtk_menu_item_new_with_label("Файл");
    GtkWidget *openitem = gtk_menu_item_new_with_label("Открыть");
    GtkWidget *saveitem = gtk_menu_item_new_with_label("Сохранить");
    GtkWidget *saveasitem = gtk_menu_item_new_with_label("Сохранить как");
    GtkWidget *quititem = gtk_menu_item_new_with_label("Выход");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), openitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveasitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quititem);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Виджет редактора
    GtkSourceBuffer *buffer = gtk_source_buffer_new(NULL);
    text_view = gtk_source_view_new_with_buffer(buffer);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(text_view), TRUE);
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(text_view), TRUE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // Горячие клавиши
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    gtk_widget_add_accelerator(openitem, "activate", accel_group, GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(saveitem, "activate", accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(saveasitem, "activate", accel_group, GDK_KEY_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(quititem, "activate", accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // Сигналы
    g_signal_connect(openitem, "activate", G_CALLBACK(on_open), NULL);
    g_signal_connect(saveitem, "activate", G_CALLBACK(on_save), NULL);
    g_signal_connect(saveasitem, "activate", G_CALLBACK(on_save_as), NULL);
    g_signal_connect(quititem, "activate", G_CALLBACK(gtk_main_quit), NULL);

    // Если файл передан в аргументах
    if (argc >= 2) {
        load_file(argv[1]);
    }

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

