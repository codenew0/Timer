#include <gtk/gtk.h>
#include "apue.h"
#include "timer.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 350
#define NOTIFICATION_WIDTH 200
#define NOTIFICATION_HEIGHT 50

Window Timer;

/**************************************
  Only can input numbers into the time entry
 **************************************/
static void entry_insert_text (GtkWidget *widget, gchar *str, gpointer data)
{
	if (!isdigit (str[0]))
	{
		str[0] = '\0';
	}
}

/**************************************
  Whenever clicked the remove button
 **************************************/
static void remove_button_clicked (GtkWidget *widget, gpointer data)
{
	unsigned int line_num = *(unsigned int *)data;
	WidgetList *p = Timer.head;

	//Search for the set which will be removed
	while (p != NULL)
	{
		if (p->cur_line_num == line_num)
		{
			//Widget remove
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->label_timer);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->entry_hour);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->label_hour);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->entry_minute);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->label_minute);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->entry_second);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->label_second);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->entry_remark);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->button_ok);
			gtk_container_remove (GTK_CONTAINER (Timer.grid), p->widget_set->button_remove);

			//Struct update
			p->prev->next = p->next;
			if (p->next != NULL)
			{
				p->next->prev = p->prev;
			}
			
			break;
		}
		p = p->next;
	}

	if (p->widget_set->time_id != -1)
	{
		g_source_remove (p->widget_set->time_id);
	}
	free (p->widget_set);
	free (p);
}

/**************************************
  Transform the numbers entered in the entry to time
 **************************************/
void num_2_time (WidgetList *p)
{
	//Transfer to legal time datas
	if (p->widget_set->time_data.sec < 0)
	{
		p->widget_set->time_data.sec = 59;
		p->widget_set->time_data.min--;
		if (p->widget_set->time_data.min < 0)
		{
			p->widget_set->time_data.min = 59;
			p->widget_set->time_data.hour--;
		}
	}
}

/**************************************
  Notification pops up for 3 seconds, and then remove it automatically 
 **************************************/
static gboolean notification_remove (gpointer data)
{
	GtkWidget *p = (GtkWidget *)data;

	gtk_widget_destroy (p);

	return FALSE;
}

/**************************************
  Notification pops up after the timer ended
 **************************************/
void notification (const char *remark)
{
	//Get screen size
	GdkScreen *screen;
	int screen_width, screen_height;
	screen = gdk_screen_get_default();
	screen_width = gdk_screen_get_width(screen);
	screen_height= gdk_screen_get_height(screen);

	//Notification window
	GtkWidget *notification_window;
	notification_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_default_size (GTK_WINDOW (notification_window), NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);
	gtk_window_move (GTK_WINDOW (notification_window), screen_width - NOTIFICATION_WIDTH, 
			screen_height - NOTIFICATION_HEIGHT - 50);

	//Notification label
	GtkWidget *label;
	label = gtk_label_new (remark);
	gtk_container_add (GTK_CONTAINER(notification_window), label);

	gtk_widget_show_all (notification_window);

	g_timeout_add (3000, (GSourceFunc)notification_remove, notification_window);
}

/**************************************
  Timer starts after clicked button "GO!"
 **************************************/
static gboolean timer_start (gpointer data)
{
	WidgetList *p = (WidgetList *)data;

	p->widget_set->time_data.sec--;
	num_2_time (p);

	if (p->widget_set->time_data.hour < 0)
	{
		p->widget_set->time_id = -1;
		//Time over, play sound
		pthread_t tid;
		pthread_create (&tid, NULL, play_sound, (void*)"bell.wav");
		notification (p->widget_set->time_data.remark);

		return FALSE;
	}

	char str[16];
	memset (str, 0, sizeof(str));
	sprintf (str, "%02d:%02d:%02d", p->widget_set->time_data.hour, p->widget_set->time_data.min, p->widget_set->time_data.sec);
	gtk_label_set_text (GTK_LABEL (p->widget_set->label_timer), str);

	return TRUE;
}

/**************************************
  Whenever click the button "GO!"
 **************************************/
static void ok_button_clicked (GtkWidget *widget, gpointer data)
{
	unsigned int line_num = *(unsigned int *)data;
	WidgetList *p = Timer.head;

	//Search for the current set 
	while (p != NULL)
	{
		if (p->cur_line_num == line_num)
		{
			break;
		}
		p = p->next;
	}
	
	//Get datas in the entries
	int h, m, s;
	h = atoi (gtk_entry_get_text (GTK_ENTRY (p->widget_set->entry_hour)));
	m = atoi (gtk_entry_get_text (GTK_ENTRY (p->widget_set->entry_minute)));
	s = atoi (gtk_entry_get_text (GTK_ENTRY (p->widget_set->entry_second)));
	p->widget_set->time_data.sec  = s % 60;
	p->widget_set->time_data.min  = (s / 60 + m) % 60;
	p->widget_set->time_data.hour = (s / 60 + m) / 60 + h;
	strcpy (p->widget_set->time_data.remark, gtk_entry_get_text (GTK_ENTRY (p->widget_set->entry_remark)));

	//Show the wirten time
	num_2_time (p);
	char str[16];
	memset (str, 0, sizeof(str));
	sprintf (str, "%02d:%02d:%02d", p->widget_set->time_data.hour, p->widget_set->time_data.min, p->widget_set->time_data.sec);
	gtk_label_set_text (GTK_LABEL (p->widget_set->label_timer), str);

	if (p->widget_set->time_id == -1)
	{
		p->widget_set->time_id = g_timeout_add (1000, (GSourceFunc)timer_start, p);
	}
	else
	{
		g_source_remove (p->widget_set->time_id);
		p->widget_set->time_id = g_timeout_add (1000, (GSourceFunc)timer_start, p);
	}
}

/**************************************
  Whenever click the add button
 **************************************/
static void add_button_clicked (GtkWidget *widget, gpointer data)
{
	//Column num
	int col = 0;

	//Window struct update
	Timer.line_num++;

	//Search for the last node
	WidgetList *p = Timer.head;
	while (p->next != NULL)
	{
		p = p->next;
	}

	//WidgetSet initialize
	WidgetList *cur_set		= (WidgetList*) malloc (sizeof (WidgetList));
	cur_set->widget_set		= (WidgetSet*) malloc (sizeof (WidgetSet));
	cur_set->cur_line_num	= Timer.line_num;
	cur_set->prev			= p;
	cur_set->next			= NULL;
	p->next					= cur_set;
	cur_set->widget_set->time_id = -1;

	//Label (timer)
	cur_set->widget_set->label_timer = gtk_label_new ("00:00:00");
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->label_timer, col, cur_set->cur_line_num, 2, 1);
	col += 2;

	//Entry (hour)
	cur_set->widget_set->entry_hour = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (cur_set->widget_set->entry_hour), 5);
	gtk_entry_set_text (GTK_ENTRY (cur_set->widget_set->entry_hour), "0");
	gtk_entry_set_max_length (GTK_ENTRY (cur_set->widget_set->entry_hour), 5);
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->entry_hour, col, cur_set->cur_line_num, 1, 1);
	col++;
	
	//Label (hour)
	cur_set->widget_set->label_hour = gtk_label_new ("h");
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->label_hour, col, cur_set->cur_line_num, 1, 1);
	col++;
	
	//Entry (minute)
	cur_set->widget_set->entry_minute = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (cur_set->widget_set->entry_minute), 5);
	gtk_entry_set_text (GTK_ENTRY (cur_set->widget_set->entry_minute), "0");
	gtk_entry_set_max_length (GTK_ENTRY (cur_set->widget_set->entry_minute), 5);
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->entry_minute, col, cur_set->cur_line_num, 1, 1);
	col++;
	
	//Label (minute)
	cur_set->widget_set->label_minute = gtk_label_new ("m");
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->label_minute, col, cur_set->cur_line_num, 1, 1);
	col++;

	//Entry (second)
	cur_set->widget_set->entry_second = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (cur_set->widget_set->entry_second), 5);
	gtk_entry_set_text (GTK_ENTRY (cur_set->widget_set->entry_second), "0");
	gtk_entry_set_max_length (GTK_ENTRY (cur_set->widget_set->entry_second), 5);
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->entry_second, col, cur_set->cur_line_num, 1, 1);
	col++;
	
	//Label (second)
	cur_set->widget_set->label_second = gtk_label_new ("s");
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->label_second, col, cur_set->cur_line_num, 1, 1);
	col++;

	//Entry (remark)
	cur_set->widget_set->entry_remark = gtk_entry_new_with_buffer (gtk_entry_buffer_new ("Remark", 6));
	gtk_entry_set_max_length (GTK_ENTRY (cur_set->widget_set->entry_remark), 255);
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->entry_remark, col, cur_set->cur_line_num, 1, 1);
	col++;

	//Button (OK)
	cur_set->widget_set->button_ok = gtk_button_new_with_label ("GO!");
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->button_ok, col, cur_set->cur_line_num, 1, 1);
	col++;

	//Icon button (Remove)
	cur_set->widget_set->button_remove = gtk_button_new_from_icon_name ("window-close", GTK_ICON_SIZE_BUTTON);
	gtk_grid_attach (GTK_GRID (Timer.grid), cur_set->widget_set->button_remove, col, cur_set->cur_line_num, 1, 1);

	/***************
	   Signal handle
	 ***************/
	//entry signal
	g_signal_connect (cur_set->widget_set->entry_hour, "insert-text", G_CALLBACK(entry_insert_text), NULL);
	g_signal_connect (cur_set->widget_set->entry_minute, "insert-text", G_CALLBACK(entry_insert_text), NULL);
	g_signal_connect (cur_set->widget_set->entry_second, "insert-text", G_CALLBACK(entry_insert_text), NULL);
	//button_ok signal
	g_signal_connect (cur_set->widget_set->button_ok, "clicked", G_CALLBACK (ok_button_clicked), &(cur_set->cur_line_num));
	//button_remove signal
	g_signal_connect (cur_set->widget_set->button_remove, "clicked", G_CALLBACK (remove_button_clicked), &(cur_set->cur_line_num));

	//test
	/*
	WidgetList *q = Timer.head;
	while (q != NULL)
	{
		printf("Line: %d\n", q->cur_line_num);
		q = q->next;
	}
	*/	
	//Show window
	gtk_widget_show_all (Timer.window);
}

/**************************************
  Whenever the tray icon is clicked
 **************************************/
static void tray_icon_clicked (GtkWidget *widget, gpointer data)
{
	if (gtk_widget_is_visible (Timer.window))
	{
		gtk_widget_hide (Timer.window);
	}
	else
	{
		gtk_widget_show_all (Timer.window);
	}
}

/**************************************
 Create pixel buffer for window's icon
 **************************************/
GdkPixbuf *create_pixbuf(const gchar* filename)
{
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	pixbuf = gdk_pixbuf_new_from_file(filename, &error);

	if (!pixbuf)
	{
		fprintf(stderr, "%s\n", error->message);
		g_error_free(error);
	}

	return pixbuf;
}

/**************************************
  Main window
 **************************************/
int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	//Struct initialize
	Timer.line_num = 0;
	Timer.head = (WidgetList*) malloc (sizeof (WidgetList));

	Timer.head->widget_set 		= NULL;
	Timer.head->prev			= NULL;
	Timer.head->next			= NULL;
	Timer.head->cur_line_num	= 0;


	//Window
	Timer.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (Timer.window), "Timer");
	gtk_window_set_icon (GTK_WINDOW(Timer.window), create_pixbuf("icon.jpg"));
	gtk_window_set_default_size (GTK_WINDOW (Timer.window), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_position (GTK_WINDOW (Timer.window), GTK_WIN_POS_CENTER);
	g_signal_connect (Timer.window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (Timer.window), 10);

	//Tray icon
	GtkWidget *tray_icon;
	tray_icon = gtk_status_icon_new_from_file ("icon.jpg");
	gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (tray_icon), "Timer");
	g_signal_connect (tray_icon, "activate", G_CALLBACK (tray_icon_clicked), NULL);

	//Grid
	Timer.grid = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (Timer.grid), 2);
	gtk_grid_set_column_homogeneous (GTK_GRID (Timer.grid), FALSE);
	gtk_container_add (GTK_CONTAINER (Timer.window), Timer.grid);

	//Button (Add)
	Timer.button_add = gtk_button_new_with_label ("Add");
	g_signal_connect (Timer.button_add, "clicked", G_CALLBACK (add_button_clicked), NULL);
	gtk_grid_attach (GTK_GRID (Timer.grid), Timer.button_add, 0, 0, 1, 1);

	//Show
	gtk_widget_show_all (Timer.window);

	gtk_main ();

	return 0;
}
