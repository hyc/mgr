/*                        Copyright (c) 1988,1989 Bellcore
 *                            All Rights Reserved
 *       Permission is granted to copy or use this program, EXCEPT that it
 *       may not be sold for profit, the copyright notice must be reproduced
 *       on copies, and credit should be given to Bellcore where it is due.
 *       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
 */
/* re-arrange MTX plumbing */

#include "term.h"
#include "mtx.h"
#include "color.h"

#define MAX_ITEM			50					/* max number of items on a menu */

/* name the menus */

#define FROM_MENU			2	
#define TO_MENU			3
#define EDIT_MENU			1
#define HIGHLIGHT_MENU	4

#define QUIT				'Q'				/* quit connection edit */
#define ADD					'A'				/* add a connection */
#define DELETE				'D'				/* delete a connection */
#define BUTTON				'B'				/* show window over button */
#define WINDOW				'W'				/* window info under button */
#define HIGHLIGHT			'H'				/* highlight object types */
#define NOP					'\n'				/* nuttin */

/* main menu options */

struct menu_entry connect_menu[] = {
	"Resume Normal Operation", "Q ",
	"Add a connection =>", "A ",
	"Delete a connection =>", "D ", 
	"Highlight objects =>", "H ",
	};

struct menu_entry highlight_menu[] = {		/* actions match object types */
	"Highlight windows", "2",
	"Highlight processes", "1",
	"Highlight hosts", "4",
	};

struct menu_entry from_menu[MAX_ITEM],to_menu[MAX_ITEM];
static char line[80];				/* mgr input line */

/* download all of the menus */

int
get_conmenus()
	{
	register struct object *object;
	register int i=1,j;
	char *str_save();
	static int cmp_menu();

	/* setup main menu and events */

	m_setmode(M_ABS);
	m_setevent(BUTTON_1,"B %p\n");
	m_setevent(BUTTON_2U,"\n");
	m_setevent(DEACTIVATE,"Q\n");
	set_color(MENU1_COLOR,MENU_BCOLOR);
	if (iscolor()) {
		Menu_load(EDIT_MENU,MENU_SIZE(connect_menu),connect_menu,0);
		Menu_load(HIGHLIGHT_MENU,MENU_SIZE(highlight_menu),highlight_menu,0);
		m_linkmenu(EDIT_MENU,3,HIGHLIGHT_MENU,MF_AUTO);
		}
	else
		Menu_load(EDIT_MENU,MENU_SIZE(connect_menu)-1,connect_menu,0);

	/* create FROM and TO menus */

	to_menu[0].value = str_save(" -TO- ");
	to_menu[0].action = str_save("");
	from_menu[0].value = str_save(" -FROM- ");
	from_menu[0].action = str_save("");

	for(object=object_list;object && i<MAX_ITEM;object=O(next),i++)  {	
		Dprintf('O',"Menu item for %d,%d\n",O(type),O(id));
		if (O(type)==D_HOST && H(state)!=H_CONN) {	/* skip unconnected hosts */
			i--;
			continue;
			}
		to_menu[i].value = str_save(O(name));
		sprintf(line,"%s =>",O(name),O(id));
		from_menu[i].value = str_save(line);
		sprintf(line,"%d %d",O(type),O(id));
		to_menu[i].action = str_save(line);
		sprintf(line,"%d %d ",O(type),O(id));
		from_menu[i].action=str_save(line);
		}

	/* sort the menu items */

	qsort(&to_menu[1],i-1,sizeof(to_menu[0]),cmp_menu);
	qsort(&from_menu[1],i-1,sizeof(to_menu[0]),cmp_menu);

	/* download and link connection menus */

	set_color(MENU2_COLOR,MENU_BCOLOR);
	Menu_load(FROM_MENU,i,from_menu,0);
	Menu_load(TO_MENU,i,to_menu,0);
	m_linkmenu(EDIT_MENU,1,FROM_MENU,MF_AUTO);
	m_linkmenu(EDIT_MENU,2,FROM_MENU,MF_AUTO);

	for(j=1;j<i;j++) {
		m_linkmenu(FROM_MENU,j,TO_MENU,0);
		}

	m_selectmenu(EDIT_MENU);
	return(i);
	}

/* free the menus */

free_conmenus()
	{
	register int i;

	for(i=0;to_menu[i].value;i++) {
		free(to_menu[i].value);
		free(to_menu[i].action);
		free(from_menu[i].value);
		free(from_menu[i].action);
		to_menu[i].value = to_menu[i].action = NULL;
		from_menu[i].value = from_menu[i].action = NULL;
		}
	}
	
/***************************************************************************
 * print all window and process connections
 */

int
old_print_cons()
   {
	register struct object *object;
	register struct dest *dest;
	char *str_save();
	
	m_move(0,0);
	fprintf(m_termout,"Connections...\033c\r\n");
	for(object=object_list;object;object=O(next))  {	
		if (O(type)==D_HOST && H(state)==H_THERE)	/* skip non-connected hosts */
			continue;
		fprintf(m_termout,"  %s:",O(name));
		for(dest=O(dest);dest;dest=D(next)) {
			fprintf(m_termout," %s",D(object)->name);
			}
		fprintf(m_termout,"\033c\r\n");
		}
	m_standout();
	fprintf(m_termout," Use the middle button to edit connections ");
	m_standend();
	fflush(m_termout);
   }

/***************************************************************************
 * print all window and process connections
 */

int
print_cons(from,to,type)
struct object *from, *to;				/* objects to highlight */
int type;									/* object type to highlight */
   {
	register struct object *object;
	register int i;
	register struct dest *dest;
	struct object **objects, **sort_objects();
	char *str_save();
	
	m_move(0,0);
	set_color(DIALOG1_COLOR,DIALOG_BCOLOR);
	fprintf(m_termout,"Connections...");
	set_color(DIALOG2_COLOR,DIALOG_BCOLOR);
	fprintf(m_termout,"\033c\r\n");
	objects = sort_objects();
	for(i=0;i<object_count;i++) {
		object = objects[i];
		Dprintf('X',"Got %s\n",O(name));
		if (O(type)==D_HOST && H(state)==H_THERE)	/* skip non-connected hosts */
			continue;
		if (object==from  || O(type) == type) {
			set_color(DIALOG3_COLOR,DIALOG_BCOLOR);
			fprintf(m_termout,"  %s:",O(name));
			set_color(DIALOG2_COLOR,DIALOG_BCOLOR);
			}
		else
			fprintf(m_termout,"  %s:",O(name));
		for(dest=O(dest);dest;dest=D(next)) {
			if ((object==from && D(object) == to) || D(object)->type == type) {
				set_color(DIALOG3_COLOR,DIALOG_BCOLOR);
				fprintf(m_termout," %s",D(object)->name);
				set_color(DIALOG2_COLOR,DIALOG_BCOLOR);
				}
			else
				fprintf(m_termout," %s",D(object)->name);
			}
		fprintf(m_termout,"\033c\r\n");
		}
	set_color(DIALOG1_COLOR,DIALOG_BCOLOR);
	fprintf(m_termout," Use the middle button to edit connections ");
	fflush(m_termout);
	free(objects);
   }

/* get the total number of connections (and other statistics) */

int
num_cons()
	{
	register struct object *object;
	register int count=0;

	/* setup window */
	for(object=object_list;object;object=O(next))
		if (O(type)!=D_HOST || H(state)!=H_THERE)
			count++;
	return(count);
	}

/* start user interface to connections */

int
start_con(font)
int font;									/* font for window */
	{
	struct object *find_byid();
	int free_dest();
	
	Dprintf('X',"Fixing connection window\n");
	set_color(DIALOG1_COLOR,DIALOG_BCOLOR);
	m_font(font);
	m_size(60,2+num_cons());
	m_setmode(M_NOWRAP);
	m_setcursor(CS_INVIS);
	get_conmenus();
	m_clear();
	print_cons(NULL,NULL);
	}

/* process a connection window command
 * This function should be subsumed by do_func
 */

int
conn_cmd(wx,wy,line)
int wx,wy;									/* window position */
char *line;									/* the command line */
	{
	extern int wmask;						/* file descriptor write mask */
	int done = 0;
	int src_type, dst_type;				/* connection types */
	int src_id, dst_id;					/* connection id's */
	int rows;								/* window size */
	struct object *from, *to;				/* objects to connect */
	struct object *find_byid();
	int free_dest();

	sscanf(line+1,"%d %d %d %d",&src_type, &src_id, &dst_type, &dst_id);
	switch(*line) {
		case QUIT:			/* go back to normal operation */
			clear_message();
			done++;
			break;

		case HIGHLIGHT:			/* highlight an object type */
			print_cons(NULL,NULL,src_type);
			break;

		case ADD:	/* add a connection  - should be replaced by do_func call */
			to = find_byid(dst_type, dst_id);
			from = find_byid(src_type, src_id);
			if (from && to) {
				from->dest = get_dest(from->dest,to);
				wmask |= set_fdmask(from,to);
				message(MSG_INFO,"Connection added %s => %s",from->name,to->name,0);
				print_cons(from,to,0);
				}
			break;

		case DELETE:	/* delete connection - should be replaced by do_func call*/
			to = find_byid(dst_type, dst_id);
			from = find_byid(src_type, src_id);
			if (from && to && free_dest(from,to)) {
				message(MSG_INFO,"Connection deleted %s => %s",
							from->name,to->name,0);
				clear_fdmask(from,to);
				print_cons(NULL,NULL,0);
				}
			else
				message(MSG_ERROR,"Error: %s !=> %s",from->name,to->name,0);
			break;

		case BUTTON:		/* Got button 1 hit */
			m_sendme("W");
			m_whatsat(wx+src_type,wy+src_id);
			break;
			case WINDOW:		/* Got window over */
			sscanf(line+1,"%*s %*s %d",&src_id);
			if (src_id > 0) {
				to  = find_byid(D_WINDOW, src_id);
				message(MSG_INFO,"On window %s (%d)",
								to?to->name:"???",src_id,0);
				}
			else
				message(MSG_WARN,"Not on an MTX window",0,0);
			break;

		default:
			/* message("Invalid command",0,0,0); */
			break;
		}
	return(done);
	}

/* malloc space for and save a string */

char *
str_save(s)
char *s;
	{
	char *malloc(), *strcpy();
	char *result;
	result = s ? malloc(strlen(s)+1) : NULL;
	if (!result) {
		Dprintf('E',"ARG!! str_save malloc %s\n",s);
		}
	return(strcpy(result,s));
	}


/* manage host menu */

extern struct menu_entry host_menu[];
static int num_hosts = 0;

/* add a host entry to the host menu */

int
menu_addhost(object)
struct object *object;					/* must be host object */
	{
	char *str_save();

	num_hosts++;
	Dprintf('O',"Adding %s to host menu\n",O(name));
	host_menu[num_hosts].value = str_save(O(name));
	sprintf(line,"H%d\n",O(id));
	host_menu[num_hosts].action = str_save(line);
	Menu_load(HOST_MENU,1+num_hosts,host_menu,dup_key);
	}

/* delete a host entry to the host menu */

int
menu_dlthost(object)
struct object *object;					/* must be host object */
	{
	int id;
	register int i,j;

	/* find the menu entry to delete */

	for(i=1;i<=num_hosts;i++) {
		sscanf(host_menu[i].action,"H%d",&id);
		if (id == O(id))
			break;
		}
	Dprintf('O',"deleting item %d from host menu\n",i);

	/* found it, now shift other entries down */

	if (i<=num_hosts) {
		free(host_menu[i].value);
		free(host_menu[i].action);
		Dprintf('O',"   freeing item %d from host menu\n",i);
		for(j=i+1;j<=num_hosts;j++) {
			Dprintf('O',"   shifting item %d to %d\n",j,j-1);
			host_menu[j-1].action = host_menu[j].action;
			host_menu[j-1].value = host_menu[j].value;
			}
		num_hosts--;
		Menu_load(HOST_MENU,1+num_hosts,host_menu,dup_key);
		Dprintf('O',"   loading %d items in host menu\n",1+num_hosts);
		}
	else
		message(MSG_ERROR,"Oops, Can't remove %s from host menu",H(host),0,0);
	}

/* generate sorted list of objects */

struct object **
sort_objects()
	{
	register struct object *object;
	struct object **objects;
	register int i=0;
	static int cmp_obj();
	char *malloc();

	objects = (struct object **)
					 malloc((1+object_count) * sizeof(struct object **));

	for(object=object_list;object;object=O(next),i++) {
		objects[i] = object;
		Dprintf('X',"will sort %s\n",O(name));
		}
	objects[i] = NULL;

	qsort(objects,object_count,sizeof(struct object *),cmp_obj);
	return(objects);
	}
	
	
/* sort objects by name */

static int 
cmp_obj(x,y)
register struct object **x, **y;
	{
	return(strcmp((*x)->name,(*y)->name));
	}

/* sort menu items by value */

static int
cmp_menu(x,y)
struct menu_entry *x,*y;
	{
	return(strcmp(x->value,y->value));
	}
