#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct flight_list_node
{
	int flightno;				   /* !< The flight number */
	char dep[20];				   /* !< Departure airport code */
	char des[20];				   /* !< Destination airport code */
	char datestr[20];			   /* !< Date departure*/
	char timestr[20];			   /* !< Time departure */
	int nofseat;				   /* !< Number of First class rows */
	int *fflags;				   /* !< An array with flags of if a seat is taken or not. 1==occupied, 0==not occupied */
	int nobseat;				   /* !< Number of Business class rows */
	int *bflags;				   /* !< An array with flags of if a seat is taken or not. 1==occupied, 0==not occupied */
	int noeseat;				   /* !< Number of Economy class rows */
	int *eflags;				   /* !< An array with flags of if a seat is taken or not. 1==occupied, 0==not occupied */
	char canceled[200];			   /* !< Stores the canceled flights*/
	struct flight_list_node *next; /* !< A pointer to the next flight information */

} FlightListNode;

typedef struct booking_list_node
{
	int booking;					/* !< The booking number */
	char datestr[15];				/* !< The departure date */
	char timestr[15];				/* !< The departure time */
	char dep[10];					/* !< The departure airport */
	char des[10];					/* !< The destination airport */
	char class[20];					/* !< The seat class */
	char fname[25];					/* !< Firstname */
	char lname[25];					/* !< Lastname */
	struct booking_list_node *next; /* !< a pointer to the next booking */

} BookingListNode;

FlightListNode *read_flights(const char *filename);
BookingListNode *read_bookings(const char *filename);
int create_tickets(BookingListNode *bookings, FlightListNode *flights);
int allocate_seat(FlightListNode *flight, BookingListNode *booking, int *row, int *seat);
void print_ticket(BookingListNode *blnp, FlightListNode *flnp, int seat, int row);
void canceled_flight(FlightListNode *flight);

int main()
{
	FlightListNode *flightlist = read_flights("flights.csv");
	BookingListNode *booklist = read_bookings("bookings.csv");
	create_tickets(booklist, flightlist);
	FlightListNode *flight;
	canceled_flight();

	for (FlightListNode *fptr = flightlist; fptr != NULL; fptr = fptr->next)
	{
		printf("flight number NR: %d\n", fptr->flightno);
	}

	for (BookingListNode *bptr = booklist; bptr != NULL; bptr = bptr->next)
	{
		printf("booking number NR: %d\n", bptr->booking);
	}
}

FlightListNode *read_flights(const char *filename)
{
	FlightListNode flight_read, *head = NULL;
	FILE *fp = fopen(filename, "r");
	while (fscanf(fp, "%d,%[^,],%[^,],%[^,],%[^,],%d,%d,%d\n", &flight_read.flightno, flight_read.dep, flight_read.des, flight_read.datestr, flight_read.timestr, &flight_read.nofseat, &flight_read.nobseat, &flight_read.noeseat) == 8)
	{
		FlightListNode *flight_stored = malloc(sizeof(FlightListNode));
		memcpy(flight_stored, &flight_read, sizeof(FlightListNode));
		flight_stored->fflags = malloc(flight_read.nofseat * sizeof(int) * 7); /** 7 is the number of seats in a row */
		memset(flight_stored->fflags, 0, flight_read.nofseat);
		flight_stored->bflags = malloc(flight_read.nobseat * sizeof(int) * 7); /** 7 is the number of seats in a row */
		memset(flight_stored->bflags, 0, flight_read.nobseat);
		flight_stored->eflags = malloc(flight_read.noeseat * sizeof(int) * 7); /** 7 is the number of seats in a row */
		memset(flight_stored->eflags, 0, flight_read.noeseat);
		flight_stored->next = head;
		head = flight_stored;
	}
	fclose(fp);
	return (head);
}

BookingListNode *read_bookings(const char *filename)
{
	BookingListNode booking_read, *head = NULL;
	FILE *fp = fopen(filename, "r");
	while (fscanf(fp, "%d,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]", &booking_read.booking, booking_read.datestr, booking_read.timestr, booking_read.dep, booking_read.des, booking_read.class, booking_read.fname, booking_read.lname) == 8)
	{
		BookingListNode *booking_list = malloc(sizeof(BookingListNode));
		memcpy(booking_list, &booking_read, sizeof(BookingListNode));
		booking_list->next = head;
		head = booking_list;
	}
	fclose(fp);
	return (head);
}

int allocate_seat(FlightListNode *flight, BookingListNode *booking, int *row, int *seat)
{
	int seatnum = 0;
	int rownum = 0;
	if (strcmp("first", booking->class) == 0)
	{
		for (int p = 0; p < flight->nofseat * 7; p++)
		{
			if (flight->fflags[p] == 0)
			{
				flight->fflags[p] = 1;
				seatnum = p + 1;
				rownum = (int)p / 7 + 1;
				break;
			}
		}
	}
	if (strcmp("business", booking->class) == 0)
	{
		for (int i = 0; i < flight->nobseat * 7; i++)
		{
			if (flight->bflags[i] == 0)
			{
				flight->bflags[i] = 1;
				seatnum = i + flight->nofseat * 7 + 1;
				rownum = flight->nofseat + (int)i / 7 + 1;
				break;
			}
		}
	}
	if (strcmp("economy", booking->class) == 0)
	{
		for (int p = 0; p < flight->noeseat * 7; p++)
		{
			if (flight->eflags[p] == 0)
			{
				flight->eflags[p] = 1;
				seatnum = p + flight->nofseat * 7 + flight->nobseat * 7 + 1; /* humans usually start counting at 1 */
				rownum = flight->nofseat + flight->nobseat + (int)p / 7 + 1;
				break;
			}
		}
	}
	if (rownum == 0 || seatnum == 0)
	{
		fprintf(stdout, "did not find class \"%s\" for booking on this plane\n", booking->class);
		return (0);
	}
	*row = rownum;
	*seat = seatnum;
	return (1);
}

int create_tickets(BookingListNode *bookings, FlightListNode *flights)
{
	int num_tickets = 0;
	fprintf(stdout, "Writing tickets: ");
	for (BookingListNode *blnp = bookings; blnp != NULL; blnp = blnp->next)
	{
		for (FlightListNode *flnp = flights; flnp != NULL; flnp = flnp->next)
		{
			if (!strcmp(blnp->dep, flnp->dep) && !strcmp(blnp->des, flnp->des) && !strcmp(blnp->datestr, flnp->datestr) && !strcmp(blnp->timestr, flnp->timestr))
			{
				int row = 0, seat = 0;
				if (allocate_seat(flnp, blnp, &row, &seat))
				{
					fprintf(stdout, "[ticket-%d.txt]", blnp->booking);
					print_ticket(blnp, flnp, seat, row);
					num_tickets++;
				}
			}
		}
	}
	fprintf(stdout, "Created  %d tickets\n\n", num_tickets);
	return (num_tickets);
}

void print_ticket(BookingListNode *blnp, FlightListNode *flnp, int seat, int row)
{
	/*char filename[255];
	sprintf(filename,"ticket-%d.txt",blnp->booking);	FILE *fp = fopen(filename,"w");
	if( fp )
	{*/
	fprintf(stdout, "BOOKING:%d\n", blnp->booking);
	fprintf(stdout, "FLIGHT:%d DEPARTURE:%s DESTINATION: %s %s %s\n", flnp->flightno, flnp->dep, flnp->des, flnp->datestr, flnp->timestr);
	fprintf(stdout, "PASSENGER %s %s\n", blnp->fname, blnp->lname);
	fprintf(stdout, "CLASS: %s\n", blnp->class);
	fprintf(stdout, "ROW %d SEAT %d\n\n", row, seat);
	/*fclose(fp);
}*/
}
void canceled_flight(FlightListNode *flight)
{
	char filename[20];
	sprintf(filename, "ticket-%d.txt");
	FILE *fp = fopen(filename, "w");
	int canceled;
	int counter;
	for (FlightListNode *flnp = flight; flnp != NULL; flnp->next)
	{
		for (int p = 0; p < flnp->nofseat * 7; p++)
		{
			if (flnp->fflags[p] != 0)
			{
				counter++;
			}
		}
		for (int p = 0; p < flnp->noeseat * 7; p++)
		{
			if (flnp->eflags[p] != 0)
			{
				counter++;
			}
		}
		for (int p = 0; p < flnp->nobseat * 7; p++)
		{
			if (flnp->bflags[p])
			{
				counter++;
			}
		}
	}
	counter *canceled;
	return (1);
}

