#ifdef MOVIE
/*{{{}}}*/
/*{{{  #includes*/
#include <mgr/bitblit.h>
#include <mgr/share.h>
#include <stdio.h>
#include <signal.h>
/*}}}  */
/*{{{  #defines*/
/* get an unused bitmap id */

#define get_mid()	(next_free>0 ? freed_ids[--next_free] : next_id++)

/* free a bitmap id */

#define free_mid(n)	(next_id-1 == (n) ? next_id-- : (freed_ids[next_free++] = (n)))

/* register/unregister  a bitmap */

#define reg_map(map) \
	bit_maps[map->id] = map

#define unreg_map(map) \
	bit_maps[map->id] = NULL

/* macros for sending data */

#define SEND_data(id,wide,high,data) { \
	_m.type = T_DATA; \
	_m.stuff[0] = id; \
	_m.stuff[1] = wide; \
	_m.stuff[2] = high; \
	_m.stuff[3] = (data ? 1 : 0); \
	DO_MSG(_m,(char *)data); \
	}

#define SEND_screen(id,wide,high,data) { \
	_m.type = T_SCREEN; \
	_m.stuff[0] = id; \
	_m.stuff[1] = wide; \
	_m.stuff[2] = high; \
	_m.stuff[3] = (data ? 1 : 0); \
	DO_MSG(_m,(char *)data); \
	}

#define SEND_SRC(dst_id,dx,dy,w,h,op,src_id,sx,sy) { \
	_m.type = (op&0xF) + T_BLIT; \
	_m.stuff[0] = dst_id; \
	_m.stuff[1] = src_id; \
	_m.stuff[2] = dx; \
	_m.stuff[3] = dy; \
	_m.stuff[4] = w; \
	_m.stuff[5] = h; \
	_m.stuff[6] = sx; \
	_m.stuff[7] = sy; \
	DO_MSG(_m,0); \
	}

#define SEND_DST(id,x,y,w,h,op) { \
	_m.type = (op&0xF) + T_WRITE; \
	_m.stuff[0] = id; \
	_m.stuff[1] = x; \
	_m.stuff[2] = y; \
	_m.stuff[3] = w; \
	_m.stuff[4] = h; \
	DO_MSG(_m,0); \
	}

#define SEND_LINE(id,x0,y0,x1,y1,op) { \
	_m.type = (op&0xF) + T_LINE; \
	_m.stuff[0] = id; \
	_m.stuff[1] = x0; \
	_m.stuff[2] = y0; \
	_m.stuff[3] = x1; \
	_m.stuff[4] = y1; \
	DO_MSG(_m,0); \
	}

#define SEND_PNT(id,x,y,op) { \
	_m.type = (op&0xF) + T_POINT; \
	_m.stuff[0] = id; \
	_m.stuff[1] = x; \
	_m.stuff[2] = y; \
	DO_MSG(_m,0); \
	}

#define SEND_KILL(id) { \
	_m.type = T_KILL; \
	_m.stuff[0] = id; \
	DO_MSG(_m,0); \
	}

#define SEND_BYTESCROLL(id,x,y,w,h,delta) { \
        _m.type=T_BYTESCROLL; \
	_m.stuff[0]=id; \
        _m.stuff[1]=x; \
        _m.stuff[2]=y; \
        _m.stuff[3]=w; \
        _m.stuff[4]=h; \
        _m.stuff[5]=delta; \
        DO_MSG(_m,0); \
	}
/*}}}  */

/*{{{  variables*/
int log_noinitial = 0;				/* dont output initial image */

static BITMAP *bit_maps[MAX_MAPS];		/* pointers to used bitmaps */
static unsigned short freed_ids[MAX_MAPS];	/* list of freed id's */
static unsigned short next_id = 1;		/* next availiable ID (0 is not used) */
static int next_free = 0;			/* next free id from free list */
static struct share_msg _m;			/* place to hold the message */
static struct share_msg do_m;			/* place to hold compressed message */
static FILE *share_file;			/* file to dump output to */
static int do_save=0;				/* saving flag */
/*}}}  */

/*{{{  write_short -- write an array of short ints to log file*/
static void write_short(short int *buff, int cnt)
{
  int i;

  if (share_file)
  {
    for (i=0; i<cnt; i++)
    {
      fputc((*buff)>>8,share_file);
      if (fputc((*buff)&0xff,share_file)<0)
      {
        fprintf(stderr,"Error: Closing MGR log.\n");
        fclose(share_file);
        do_save=0;
        share_file=(FILE*)0;
      }
      buff++;
    }
  }
}
/*}}}  */
/*{{{  write_char -- write an array of chars to log file*/
static void write_char(buff,cnt) char *buff; int cnt;
{
  if (share_file && cnt!=fwrite(buff,1,cnt,share_file))
  {
    fprintf(stderr,"Error: Closing MGR log.\n");
    fclose(share_file);
    do_save=0;
    share_file=(FILE*)0;
  }
}
/*}}}  */
/*{{{  check_map -- check bitmap, register and/or download if needed*/
static int check_map(map) register BITMAP *map;
{
  if (map && map->primary->id == 0)
  {
    /* static bitmap */
    map->primary->id = get_mid();
    reg_map(map->primary);
    map->primary->type |= _DIRTY;
  }

  if (map && do_save && map->primary->type & _DIRTY)
  {
    SEND_data(map->id,map->primary->wide,map->primary->high,map->data);
    map->primary->type &= ~_DIRTY;
    return(1);
  }
  return(0);
}
/*}}}  */
/*{{{  DO_MSG*/
static int
DO_MSG(msg,data)
struct share_msg msg;				/* message to send */
char *data;					/* pointer to data */
{
  switch (msg.type&TYPE_MASK)
  {
    /*{{{  T_SCREEN*/
    case T_SCREEN:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),4);
    if (msg.stuff[3]!=0) write_char(data,bit_size(msg.stuff[1],msg.stuff[2],1));
    break;
    /*}}}  */
    /*{{{  T_BLIT*/
    case T_BLIT:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),8);
    break;
    /*}}}  */
    /*{{{  T_WRITE*/
    case T_WRITE:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),5);
    break;
    /*}}}  */
    /*{{{  T_LINE*/
    case T_LINE:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),5);
    break;
    /*}}}  */
    /*{{{  T_DATA*/
    case T_DATA:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),4);
    if (msg.stuff[3] != 0) write_char(data,bit_size(msg.stuff[1],msg.stuff[2],1));
    break;
    /*}}}  */
    /*{{{  T_NOP*/
    case T_NOP:
    write_short(&(msg.type),1);
    if (msg.type&0xF > 0) write_char(data,msg.type&0xF);
    /*}}}  */
    /*{{{  T_POINT*/
    case T_POINT:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),3);
    break;
    /*}}}  */
    /*{{{  T_TIME*/
    case T_TIME:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),2);
    break;
    /*}}}  */
    /*{{{  T_KILL*/
    case T_KILL:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),1);
    break;
    /*}}}  */
    /*{{{  T_BYTESCROLL*/
    case T_BYTESCROLL:
    write_short(&(msg.type),1);
    write_short(&(msg.stuff[0]),6);
    break;
    /*}}}  */
  }
}
/*}}}  */

/*{{{  log_blit -- do the logging for bitmaps*/
void log_blit(BITMAP *dst_map,int xd,int yd,int w,int h,int op,BITMAP *src_map,int xs, int ys)
{
  check_map(dst_map);
  check_map(src_map);

  /* log xaction iff turned on */

  if (do_save)
  {
    if (src_map)
    {
      src_map->primary->type &= ~_DIRTY;
      SEND_SRC(dst_map->id,xd,yd,w,h,op,src_map->id,xs,ys);
    }
    else
    {
      SEND_DST(dst_map->id,xd,yd,w,h,op);
    }
    dst_map->primary->type &= ~_DIRTY;
  }
}
/*}}}  */
/*{{{  log_line -- log lines*/
void log_line(dst,x0,y0,x1,y1,op) BITMAP *dst; int x0,y0,x1,y1; int op;
{
  check_map(dst);
  if (do_save)
  {
    SEND_LINE(dst->id,x0,y0,x1,y1,op);
  }
}
/*}}}  */
/*{{{  log_point -- log points*/
void log_point(dst,x,y,op)
BITMAP *dst;
int x,y;
int op;
{
  check_map(dst);
  if (do_save)
  {
    SEND_PNT(dst->id,x,y,op);
  }
}
/*}}}  */
/*{{{  log_open -- log bit_open*/
void log_open(bp) BITMAP *bp;
{
  bp->id=get_mid();
  reg_map(bp);
  if (do_save) SEND_screen(bp->id,bp->wide,bp->high,0);
}
/*}}}  */
/*{{{  log_destroy -- log bit_destroy*/
void log_destroy(bitmap) BITMAP *bitmap;
{
  if (do_save) SEND_KILL(bitmap->id);
  unreg_map(bitmap);
  free_mid(bitmap->id);
}
/*}}}  */
/*{{{  log_alloc -- log bit_alloc*/
void log_alloc(bp) BITMAP *bp;
{
  int id;

id = get_mid();
bp->id = id;
reg_map(bp);
if (do_save) SEND_data(id,BIT_WIDE(bp),BIT_HIGH(bp),BIT_DATA(bp));
}
/*}}}  */
/*{{{  log_create -- log bit_create*/
void log_create(bp) BITMAP *bp;
{
	if (bp->id ==0) {		/* static bitmap */
		bp->id = get_mid();
		reg_map(bp);
		if (do_save) SEND_data(bp->id,bp->wide,bp->high,bp->data);	/* create a "real" bitmap */
		bp->type &= ~_DIRTY;
		}
}
/*}}}  */
/*{{{  log_bytescroll -- log fast byte aligned scroll*/
void log_bytescroll(BITMAP *dst, int x, int y, int w, int h, int delta)
{
  check_map(dst);
  if (do_save) SEND_BYTESCROLL(dst->id,x,y,w,h,delta);
}
/*}}}  */
/*{{{  send_sync -- start a session by sending current bitmap state*/
void send_sync()
{
  BITMAP *map;
  register int i;

  for(i=1;i<next_id;i++)
  {
    if ((map=bit_maps[i]) && IS_SCREEN(map) && IS_PRIMARY(map))
    {
      SEND_screen(i,BIT_WIDE(map),BIT_HIGH(map),0);
      if (log_noinitial) log_noinitial = 0;
      else
      {
        map->primary->type |= _DIRTY;
#ifdef DEBUG
        fprintf(stderr,"Setting %d (%d x %d) to dirty SCREEN \n",i,
        BIT_WIDE(map), BIT_HIGH(map));
#endif
      }
    }
    else if (map && IS_PRIMARY(map) && map->primary->id == i) map->primary->type |= _DIRTY;
    else if (map) fprintf(stderr,"Bitmap %d is corrupted\n",i);
#ifdef DEBUG
    else fprintf(stderr,"Bitmap %d is not there!!!\n",i);
#endif
  }
}
/*}}}  */
/*{{{  log_time -- send a timestamp for sequencing of replay*/
void log_time(void)
{
  if (do_save)
  {
    register unsigned int time = timestamp();
    _m.type = T_TIME;
    _m.stuff[0] = time>>16;
    _m.stuff[1] = time&0xffff;
    DO_MSG(_m,0);
    fflush(share_file);
  }
}
/*}}}  */
/*{{{  log_start -- begin logging, send output to 'file'*/
int log_start(FILE *file)
{
  if (do_save==0 && file)
  {
    share_file = file;
    do_save = 1;
    send_sync();
    return(1);
  }
  else return(0);
}
/*}}}  */
/*{{{  log_end -- let application close the log file*/
int log_end(void)
{
  if (do_save)
  {
    do_save=0;
    SEND_KILL(0);
    return(1);
  }
  else
  return(0);
}
/*}}}  */
#endif
