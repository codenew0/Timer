#ifndef _CLOCK_H_
#define _CLOCK_H_

typedef struct TimeData
{
	int hour, min, sec;
	char remark[256];
}TimeData;

typedef struct WidgetSet
{
	GtkWidget *label_timer;
	GtkWidget *entry_hour;
	GtkWidget *label_hour;
	GtkWidget *entry_minute;
	GtkWidget *label_minute;
	GtkWidget *entry_second;
	GtkWidget *label_second;
	GtkWidget *entry_remark;
	GtkWidget *button_ok;
	GtkWidget *button_remove;
	TimeData  time_data;
	gint 	  time_id;
}WidgetSet;

typedef struct WidgetList 
{
	WidgetSet *widget_set;
	unsigned int cur_line_num;
	struct WidgetList *prev;
	struct WidgetList *next;
}WidgetList;

typedef struct Window
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *button_add;
	WidgetList *head;
	unsigned int line_num;
}Window;

extern void *play_sound (void *filename);

#endif
