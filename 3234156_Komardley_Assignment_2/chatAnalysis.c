#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SUBJECT 500
#define MAX_LINE 600

/* ---------------------------------------------------------
   Email structure
   --------------------------------------------------------- */
typedef struct {
    char sender[30];
    char subject[MAX_SUBJECT];

    int month;
    int day;
    int year;

    int priority;
} Email;


/* ---------------------------------------------------------
   MaxHeap structure
   --------------------------------------------------------- */
typedef struct {
    Email *emails;
    int size;
    int capacity;
} MaxHeap;


/* ---------------------------------------------------------
   Get priority for sender category

   Lower number = higher priority.
   Boss is highest priority.
   --------------------------------------------------------- */
int getPriority(char sender[]) {
    if (strcmp(sender, "Boss") == 0)
        return 5;

    if (strcmp(sender, "Subordinate") == 0)
        return 4;

    if (strcmp(sender, "Peer") == 0)
        return 3;

    if (strcmp(sender, "ImportantPerson") == 0)
        return 2;

    if (strcmp(sender, "OtherPerson") == 0)
        return 1;

    return 0;
}


/* ---------------------------------------------------------
   Convert date to a number so dates can be compared.

   A larger value means a newer date.
   --------------------------------------------------------- */
int dateValue(Email *email) {
    return email->year * 10000 +
           email->month * 100 +
           email->day;
}


/* ---------------------------------------------------------
   Compare two emails.

   Returns 1 if a should appear BEFORE b in the priority
   queue.

   Priority category is considered first.
   If categories are equal, newest date comes first.
   --------------------------------------------------------- */
int higherPriority(Email *a, Email *b) {

    if (a->priority > b->priority)
        return 1;

    if (a->priority < b->priority)
        return 0;

    /* Same sender category:
       newest email goes first */
    if (dateValue(a) > dateValue(b))
        return 1;

    return 0;
}


/* ---------------------------------------------------------
   Initialize heap
   --------------------------------------------------------- */
void initializeHeap(MaxHeap *heap) {
    heap->capacity = 10;
    heap->size = 0;

    heap->emails = malloc(heap->capacity * sizeof(Email));

    if (heap->emails == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
}


/* ---------------------------------------------------------
   Resize heap when it becomes full
   --------------------------------------------------------- */
void resizeHeap(MaxHeap *heap) {
    heap->capacity *= 2;

    Email *temp = realloc(
        heap->emails,
        heap->capacity * sizeof(Email)
    );

    if (temp == NULL) {
        printf("Memory allocation failed.\n");
        free(heap->emails);
        exit(1);
    }

    heap->emails = temp;
}


/* ---------------------------------------------------------
   Swap two emails
   --------------------------------------------------------- */
void swapEmails(Email *a, Email *b) {
    Email temp = *a;
    *a = *b;
    *b = temp;
}


/* ---------------------------------------------------------
   Insert an email into the MaxHeap
   --------------------------------------------------------- */
void insertHeap(MaxHeap *heap, Email email) {

    if (heap->size == heap->capacity) {
        resizeHeap(heap);
    }

    /* Put new email at the end */
    int index = heap->size;
    heap->emails[index] = email;
    heap->size++;

    /* Bubble up */
    while (index > 0) {

        int parent = (index - 1) / 2;

        if (!higherPriority(
                &heap->emails[index],
                &heap->emails[parent])) {
            break;
        }

        swapEmails(
            &heap->emails[index],
            &heap->emails[parent]
        );

        index = parent;
    }
}


/* ---------------------------------------------------------
   Return the highest-priority email.

   This does NOT remove the email.
   --------------------------------------------------------- */
Email *peekHeap(MaxHeap *heap) {

    if (heap->size == 0) {
        return NULL;
    }

    return &heap->emails[0];
}


/* ---------------------------------------------------------
   Remove the highest-priority email.

   Used by READ.
   --------------------------------------------------------- */
Email removeHeap(MaxHeap *heap) {

    Email removed = heap->emails[0];

    /* Move last item to root */
    heap->size--;

    if (heap->size > 0) {

        heap->emails[0] =
            heap->emails[heap->size];

        /* Bubble down */
        int index = 0;

        while (1) {

            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;

            if (left < heap->size &&
                higherPriority(
                    &heap->emails[left],
                    &heap->emails[largest])) {

                largest = left;
            }

            if (right < heap->size &&
                higherPriority(
                    &heap->emails[right],
                    &heap->emails[largest])) {

                largest = right;
            }

            if (largest == index) {
                break;
            }

            swapEmails(
                &heap->emails[index],
                &heap->emails[largest]
            );

            index = largest;
        }
    }

    return removed;
}


/* ---------------------------------------------------------
   Display an email in the required format
   --------------------------------------------------------- */
void displayEmail(Email *email) {

    printf("Next email:\n");
    printf("    Sender: %s\n", email->sender);
    printf("    Subject: %s\n", email->subject);

    printf("    Date: %02d-%02d-%04d\n",
           email->month,
           email->day,
           email->year);
}


/* ---------------------------------------------------------
   Parse an EMAIL command.

   Format:
   EMAIL <sender category>,<subject>,<date>

   Example:
   EMAIL Boss,Important,12-20-2024
   --------------------------------------------------------- */
int parseEmail(char *line, Email *email) {

    char *contents;
    char *comma1;
    char *comma2;

    /* Find "EMAIL " */
    contents = line + 6;

    /* Remove newline */
    contents[strcspn(contents, "\n")] = '\0';

    /* Find commas */
    comma1 = strchr(contents, ',');
    
    if (comma1 == NULL)
        return 0;

    comma2 = strchr(comma1 + 1, ',');

    if (comma2 == NULL)
        return 0;

    /* Sender */
    *comma1 = '\0';
    strcpy(email->sender, contents);

    /* Subject */
    *comma2 = '\0';
    strcpy(email->subject, comma1 + 1);

    /* Date */
    if (sscanf(
            comma2 + 1,
            "%d-%d-%d",
            &email->month,
            &email->day,
            &email->year
        ) != 3) {

        return 0;
    }

    email->priority = getPriority(email->sender);

    return 1;
}


/* ---------------------------------------------------------
   Main
   --------------------------------------------------------- */
int main(void) {

    char filename[100];
    char line[MAX_LINE];

    MaxHeap heap;

    initializeHeap(&heap);

    /* Get test file name */
    printf("Enter test file name: ");
    scanf("%99s", filename);

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Could not open file.\n");
        free(heap.emails);
        return 1;
    }


    /* -----------------------------------------------------
       Read commands from file
       ----------------------------------------------------- */
    while (fgets(line, sizeof(line), file) != NULL) {

        /* Remove newline */
        line[strcspn(line, "\n")] = '\0';


        /* EMAIL command */
        if (strncmp(line, "EMAIL ", 6) == 0) {

            Email email;

            if (parseEmail(line, &email)) {
                insertHeap(&heap, email);
            }
        }


        /* NEXT command */
        else if (strcmp(line, "NEXT") == 0) {

            Email *email = peekHeap(&heap);

            if (email != NULL) {
                displayEmail(email);
            }
        }


        /* READ command */
        else if (strcmp(line, "READ") == 0) {

            if (heap.size > 0) {
                removeHeap(&heap);
            }
        }


        /* COUNT command */
        else if (strcmp(line, "COUNT") == 0) {

            printf("There are %d emails to read.\n",
                   heap.size);
        }
    }


    fclose(file);
    free(heap.emails);

    return 0;
}
