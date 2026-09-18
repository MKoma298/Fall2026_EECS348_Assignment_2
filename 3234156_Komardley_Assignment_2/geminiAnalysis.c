/*
Prologue:
Name: EECS 348 Assignment 2
Desc.: This assignment is using a max heap to prioritize emails
for a busy company. Also, comparing two different gen ai code to 
one another and using the better code for the company.
Inputs: A .txt file with the emails in it
Outputs: 
"There are ___ emails to read"

Next email:
        Sender: ______
        Subject:______
        Date: ______

There are ____ emails to read"

This is just an example formatting, the formatting will change
based on the inputed .txt file.

Code Sources: Chatgpt, Gemini
Name: Miakah Komardley
Creation Date: Sept. 17th, 2026
Revision Date: Sept. 17th, 2026
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Priority constants
#define PRIORITY_BOSS 5
#define PRIORITY_SUBORDINATE 4
#define PRIORITY_PEER 3
#define PRIORITY_IMPORTANT 2
#define PRIORITY_OTHER 1
#define PRIORITY_UNKNOWN 0

typedef struct {
    char sender[50];
    char subject[200];
    char date[15];
    int priority_level;
    long date_val;
    int insert_id; // Tie-breaker for same-day emails (newer = inserted later)
} Email;

typedef struct {
    Email* array;
    int size;
    int capacity;
} MaxHeap;

// ---------------------------------------------------------
// Helper Functions for Parsing & Prioritization
// ---------------------------------------------------------

int get_priority(const char* sender) {
    if (strcmp(sender, "Boss") == 0) return PRIORITY_BOSS;
    if (strcmp(sender, "Subordinate") == 0) return PRIORITY_SUBORDINATE;
    if (strcmp(sender, "Peer") == 0) return PRIORITY_PEER;
    if (strcmp(sender, "ImportantPerson") == 0) return PRIORITY_IMPORTANT;
    if (strcmp(sender, "OtherPerson") == 0) return PRIORITY_OTHER;
    return PRIORITY_UNKNOWN;
}

// Converts MM-DD-YYYY to an integer YYYYMMDD for easy comparison
long parse_date(const char* date_str) {
    int m, d, y;
    if (sscanf(date_str, "%d-%d-%d", &m, &d, &y) == 3) {
        return (long)y * 10000 + m * 100 + d;
    }
    return 0;
}

// Returns >0 if a has higher priority than b, <0 if lower, 0 if equal
int compare_emails(Email a, Email b) {
    if (a.priority_level != b.priority_level) {
        return a.priority_level - b.priority_level;
    }
    if (a.date_val != b.date_val) {
        return (a.date_val > b.date_val) ? 1 : -1;
    }
    return a.insert_id - b.insert_id;
}

// ---------------------------------------------------------
// MaxHeap Implementation (From Scratch)
// ---------------------------------------------------------

void init_heap(MaxHeap* heap, int initial_capacity) {
    heap->capacity = initial_capacity;
    heap->size = 0;
    heap->array = (Email*)malloc(sizeof(Email) * heap->capacity);
}

void free_heap(MaxHeap* heap) {
    free(heap->array);
    heap->size = 0;
    heap->capacity = 0;
}

void heapify_up(MaxHeap* heap, int index) {
    while (index > 0) {
        int parent_index = (index - 1) / 2;
        if (compare_emails(heap->array[index], heap->array[parent_index]) > 0) {
            Email temp = heap->array[index];
            heap->array[index] = heap->array[parent_index];
            heap->array[parent_index] = temp;
            index = parent_index;
        } else {
            break;
        }
    }
}

void heapify_down(MaxHeap* heap, int index) {
    int max_index = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < heap->size && compare_emails(heap->array[left_child], heap->array[max_index]) > 0) {
        max_index = left_child;
    }
    if (right_child < heap->size && compare_emails(heap->array[right_child], heap->array[max_index]) > 0) {
        max_index = right_child;
    }

    if (max_index != index) {
        Email temp = heap->array[index];
        heap->array[index] = heap->array[max_index];
        heap->array[max_index] = temp;
        heapify_down(heap, max_index);
    }
}

void insert_email(MaxHeap* heap, Email email) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->array = (Email*)realloc(heap->array, sizeof(Email) * heap->capacity);
    }
    heap->array[heap->size] = email;
    heapify_up(heap, heap->size);
    heap->size++;
}

void extract_max(MaxHeap* heap) {
    if (heap->size == 0) return;
    
    // Replace root with the last element
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    
    if (heap->size > 0) {
        heapify_down(heap, 0);
    }
}

// ---------------------------------------------------------
// Main Execution
// ---------------------------------------------------------

int main() {
    MaxHeap inbox;
    init_heap(&inbox, 100); // Start with an arbitrary reasonable capacity

    char line[512];
    int global_insert_id = 0;

    // Read commands line by line from stdin (or a redirected file)
    while (fgets(line, sizeof(line), stdin)) {
        // Strip trailing newlines
        line[strcspn(line, "\r\n")] = 0;
        
        if (strlen(line) == 0) continue;

        if (strncmp(line, "EMAIL ", 6) == 0) {
            char* payload = line + 6; // Skip "EMAIL "
            
            char* sender = strtok(payload, ",");
            char* subject = strtok(NULL, ",");
            char* date = strtok(NULL, ",");

            if (sender && subject && date) {
                Email new_email;
                strncpy(new_email.sender, sender, sizeof(new_email.sender) - 1);
                new_email.sender[sizeof(new_email.sender) - 1] = '\0';
                
                strncpy(new_email.subject, subject, sizeof(new_email.subject) - 1);
                new_email.subject[sizeof(new_email.subject) - 1] = '\0';
                
                strncpy(new_email.date, date, sizeof(new_email.date) - 1);
                new_email.date[sizeof(new_email.date) - 1] = '\0';
                
                new_email.priority_level = get_priority(new_email.sender);
                new_email.date_val = parse_date(new_email.date);
                new_email.insert_id = ++global_insert_id;
                
                insert_email(&inbox, new_email);
            }
        } 
        else if (strcmp(line, "NEXT") == 0) {
            if (inbox.size > 0) {
                Email top_email = inbox.array[0];
                printf("Next email:\n");
                printf("    Sender: %s\n", top_email.sender);
                printf("    Subject: %s\n", top_email.subject);
                printf("    Date: %s\n\n", top_email.date);
            }
        } 
        else if (strcmp(line, "READ") == 0) {
            extract_max(&inbox);
        } 
        else if (strcmp(line, "COUNT") == 0) {
            printf("There are %d emails to read.\n\n", inbox.size);
        }
    }

    free_heap(&inbox);
    return 0;
}