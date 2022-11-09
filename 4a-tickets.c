#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct flight_list_node 
{
	int flightno;          /* !< The flight number */
	char dep[20];          /* !< Departure airport code */
	char des[20];          /* !< Destination airport code */
	char datestr[20];      /* !< Date departure*/
	char timestr[20];      /* !< Time departure */
	int nfs;               /* !< Number of First class rows */
	int *fs;	       /* !< An array with flags of if a seat is taken or not. 1==occupied, 0==not occupied */
	int nbs;               /* !< Number of Business class rows */
	int *bs;               /* !< An array with flags of if a seat is taken or not. 1==occupied, 0==not occupied */
	int nes;               /* !< Number of Economy class rows */
	int *es;               /* !< An array with flags of if a seat is taken or not. 1==occupied, 0==not occupied */
	struct flight_list_node *next; /* !< A pointer to the next flight information */
} FlightListNode;

typedef struct booking_list_node 
{
	int booking;          /* !< The booking number */
	char datestr[15];     /* !< The departure date */
	char timestr[15];     /* !< The departure time */
	char dep[10];         /* !< The departure airport */
	char des[10];         /* !< The destination airport */
	char class[20];       /* !< The seat class */
	char fname[25];       /* !< Firstname */
	char lname[25];       /* !< Lastname */
	struct booking_list_node *next; /* !< a pointer to the next booking */
} BookingListNode;

FlightListNode *read_flights(const char *filename);
BookingListNode *read_bookings(const char *filename);
int create_tickets(BookingListNode *bookings, FlightListNode *flights);
int allocate_seat(FlightListNode *flight,BookingListNode *booking,int *row, int *seat);
void print_ticket(BookingListNode *blnp, FlightListNode *flnp,int seat,int row);


int main ()
{
    FlightListNode *flist = read_flights("flights.csv");
    BookingListNode *blist = read_bookings("bookings.csv");
    create_tickets(blist, flist);

    for(FlightListNode *fptr = flist; fptr !=NULL; fptr = fptr->next)
    {
	 	printf("flight number NR: %d\n", fptr->flightno);
    }

    for(BookingListNode *bptr = blist; bptr !=NULL; bptr = bptr->next)
    {
	 	printf("booking number NR: %d\n", bptr->booking);
    }

 
}

FlightListNode *read_flights(const char *filename)
{
	FlightListNode fln,*head=NULL;
	FILE *fp = fopen(filename,"r");
	while( fscanf(fp,"%d,%[^,],%[^,],%[^,],%[^,],%d,%d,%d\n",&fln.flightno,fln.dep,fln.des,fln.datestr,fln.timestr,&fln.nfs,&fln.nbs,&fln.nes) == 8 ) 
    {
		FlightListNode *flp = malloc(sizeof(FlightListNode));
		memcpy(flp,&fln,sizeof(FlightListNode));
		flp->fs = malloc(fln.nfs*sizeof(int)*7); /** 7 is the number of seats in a row */
		memset(flp->fs,0,fln.nfs);
		flp->bs = malloc(fln.nbs*sizeof(int)*7); /** 7 is the number of seats in a row */
		memset(flp->bs,0,fln.nbs);
		flp->es = malloc(fln.nes*sizeof(int)*7); /** 7 is the number of seats in a row */
		memset(flp->es,0,fln.nes);
		flp->next =  head;
		head = flp;
	}
	return(head);
}

BookingListNode *read_bookings(const char *filename) 
{
	BookingListNode bln,*head=NULL;
    FILE *fp = fopen(filename,"r");
    while(fscanf(fp,"%d,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]",&bln.booking,bln.datestr,bln.timestr,bln.dep,bln.des,bln.class,bln.fname,bln.lname)==8)
    {
        BookingListNode *blp = malloc(sizeof(BookingListNode));
        memcpy(blp,&bln,sizeof(BookingListNode));
        blp->next = head;
        head = blp;
    }
    fclose(fp);
    return(head);
}

int allocate_seat(FlightListNode *flight,BookingListNode *booking,int *row, int *seat)
{
	int sn = 0;
	int rn = 0;
	if( strcmp("first",booking->class) == 0 ) {
		for(int p=0;p<flight->nfs*7;p++) {
			if( flight->fs[p] == 0 ) {
				flight->fs[p] = 1;
				sn = p+1;
				rn = (int)p/7+1;
				break;
			}
		}
	}
	if( strcmp("business",booking->class) == 0 ) 
    {
		for(int p=0;p<flight->nbs*7;p++) 
        {
			if( flight->bs[p] == 0 ) {
				flight->bs[p] = 1;
				sn = p+flight->nfs*7+1;  /* humans usually start counting at 1 */
				rn = flight->nfs+(int)p/7+1;
				break;
			}
		}
	}
	if( strcmp("economy",booking->class) == 0 ) 
    {
		for(int p=0;p<flight->nes*7;p++) {
			if( flight->es[p] == 0 ) 
            {
				flight->es[p] = 1;
				sn = p+flight->nfs*7+flight->nbs*7+1;  /* humans usually start counting at 1 */
				rn = flight->nfs+flight->nbs+(int)p/7+1;
				break;
			}
		}
	}
	if( rn ==0 || sn == 0 ) 
    {
		fprintf(stdout,"did not find class \"%s\" for booking on this plane\n",booking->class);
		return(0);
	}
	*row = rn;
	*seat = sn;
	return(1);
}

int create_tickets(BookingListNode *bookings, FlightListNode *flights)
{
	int num_tickets = 0;
	fprintf(stdout,"Writing tickets: ");
	for(BookingListNode *blnp = bookings; blnp != NULL; blnp=blnp->next) 
    {
        for(FlightListNode *flnp = flights; flnp != NULL; flnp=flnp->next)
        {
         if( !strcmp(blnp->dep,flnp->dep) &&  !strcmp(blnp->des,flnp->des) &&  !strcmp(blnp->datestr,flnp->datestr) &&  !strcmp(blnp->timestr,flnp->timestr) )
			{
                int row=0, seat=0;
				if(allocate_seat(flnp,blnp,&row,&seat))
                {
					fprintf(stdout,"[ticket-%d.txt]",blnp->booking);
					print_ticket(blnp,flnp,seat,row);
					num_tickets++;
				}            
			}   
        }
	}
	fprintf(stdout,"Created  %d tickets\n\n",num_tickets);
	return(num_tickets);
}


void print_ticket(BookingListNode *blnp, FlightListNode *flnp,int seat,int row)
{
	/*char filename[255];
	sprintf(filename,"ticket-%d.txt",blnp->booking);
	FILE *fp = fopen(filename,"w");
	if( fp ) 
    {*/
		fprintf(stdout,"BOOKING:%d\n",blnp->booking);
		fprintf(stdout,"FLIGHT:%d DEPARTURE:%s DESTINATION: %s %s %s\n",flnp->flightno,flnp->dep,flnp->des,flnp->datestr,flnp->timestr);
		fprintf(stdout,"PASSENGER %s %s\n",blnp->fname,blnp->lname);
		fprintf(stdout,"CLASS: %s\n",blnp->class);
		fprintf(stdout,"ROW %d SEAT %d\n\n",row,seat);
		/*fclose(fp);
	}*/
}
