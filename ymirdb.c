#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <ctype.h>
#include "ymirdb.h"

/* Function: command_bye
 * ----------------------------
 *   Prints bye.
 */
void command_bye()
{
	printf("bye\n");
}

/* Function: command_bye
 * ----------------------------
 *   Prints help.
 */
void command_help()
{
	printf("%s", HELP);
}

/* Function: element_assign_integer
 * ----------------------------
 *   Assign element values an integers element.
 *
 *   pointer: a pointer to the integer element
 *   value: the value of the element
 */
void element_assign_integer(element **integer_element, char *value)
{
	(*integer_element)->type = INTEGER;
	(*integer_element)->value = atoi(value);
	(*integer_element)->next = NULL;
}

/* Function: find_entry
 * ----------------------------
 *   In the entry linked list find an entry with a key.
 *
 *   node: the header node of the entry linked list
 *   key: the key string we want to find for an entry node
 *   return find_entry: a pointer to the entry with the key, returns NULL if no match
 */
entry *find_entry(entry **entry_head, char *key)
{
	if (*entry_head == NULL)
	{
		return NULL;
	}
	entry *node = *entry_head;
	while (node != NULL)
	{
		if (strcmp((node)->key, key) == 0)
		{
			return node;
		}
		else
		{
			node = (node)->next;
		}
	}

	return NULL;
}

/* Function: clean_entry
 * ----------------------------
 *   Cleans all elements in an entry
 *
 *   entry_free: the entry to free from memory
 */
void clean_entry(entry *entry_free)
{
	element *val = entry_free->values;
	while (val != NULL)
	{
		element *toFree = val;
		val = val->next;

		free(toFree);
	}
	if (entry_free->forward != NULL)
	{
		free(entry_free->forward);
	}
	if (entry_free->backward != NULL)
	{
		free(entry_free->backward);
	}
}

/* Function: clean_entry_override
 * ----------------------------
 *   In the entry linked list find an entry with a key. But if we are overriding an element we only clean its forward references.
 *
 *   entry_free: the entry to free
 */
void clean_entry_override(entry *entry_free)
{
	element *val = entry_free->values;
	while (val != NULL)
	{
		element *toFree = val;
		val = val->next;

		free(toFree);
	}
	if (entry_free->forward != NULL)
	{
		free(entry_free->forward);
	}
}

/* Function: clean_memory
 * ----------------------------
 *   For a header of an entry linked list, call clean_entry on all its entries.
 *
 *   entry_head: the header node of the entry linked list
 */
void clean_memory(entry **entry_head)
{
	entry *node = *entry_head;
	while (node != NULL)
	{
		clean_entry(node);
		entry *node_to_free = node;
		node = node->next;
		free(node_to_free);
	}
}

/* Function: clean_snapshots
 * ----------------------------
 *   For a snapshot of a snapshot linked list, call clean_memory on all its entries.
 *
 *   snapshot_current: the current snapshot (last one in the linked list)
 */
void clean_snapshots(snapshot **snapshot_current)
{
	while ((*snapshot_current) != NULL)
	{
		clean_memory(&((*snapshot_current)->entries));
		snapshot *temp = *snapshot_current;
		*snapshot_current = (*snapshot_current)->prev;
		free(temp);
	}
}

/* For qsort() - citation: https://stackoverflow.com/questions/34565028/cmpfunc-in-qsort-function-in-c
 * Checks if a is bigger than b - casts to int and gets difference */
int cmp_func(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

/* Function: find_entry_count
 * ----------------------------
 *   Finds and entry and returns its index in the linked list.
 *
 *   entry_head: the head of the entry linked list
 * 	 key: the key of the entry to find
 *   returns the index of the entry
 */
int find_entry_count(entry *node, char *key)
{
	int count = -1;
	do
	{
		count++;
		if (strcmp((node)->key, key) == 0)
		{
			return count;
		}
		else
		{
			node = node->next;
		}
	} while (node != NULL);
	return -1;
}

/* Function: find_entry_at_count
 * ----------------------------
 *   Finds and entry at a position in the linked list.
 *
 *   entry_head: the head of the entry linked list
 * 	 count: the index of the entry to find
 * 	 returns the a pointer to the entry in the linked list
 */
entry *find_entry_at_count(entry *entry_head, int count)
{
	entry *node = entry_head;
	int i = 0;
	while (count != i && node != NULL)
	{
		i++;
		node = node->next;
	};
	return node;
}

/* Function: add_entry
 * ----------------------------
 *   Add an entry to the entry linked list.
 *
 *   entry_head: the header node of the linked list
 *   new_entry: the entry to add next in the linked list
 */
void add_entry(entry **entry_head, entry **new_entry)
{
	if (*entry_head == NULL)
	{
		*entry_head = *new_entry;
	}
	else
	{

		entry *node = *entry_head;
		while (node->next != NULL)
		{
			node = node->next;
		}

		node->next = *new_entry;
	}
}

/* Function: element_assign_reference
 * ----------------------------
 *   For an entry element, assign its values
 *
 *   reference_element: Pointer heaad of the reference element array
 *   found: the key of the entry to chek the reference of
 * 	 entry_head: the head of the entry linked list to find the element to reference the element to
 */
entry *element_assign_reference(element **reference_element, char *found, entry *entry_head)
{
	(*reference_element)->type = ENTRY;
	entry *found_entry = find_entry(&entry_head, found);
	(*reference_element)->entry = found_entry;
	(*reference_element)->next = NULL;
	return found_entry;
}

/* Function: add_element_to_value
 * ----------------------------
 *   Add an entry to the entry linked list.
 *
 *   element_head: the header node of the element linked list for the given entry
 *   next_element: the element to add next in the linked list
 */
void add_element_to_value(element **element_head, element **next_element)
{
	if ((*element_head) == NULL)
	{
		*element_head = *next_element;
	}
	else
	{
		element *node = *element_head;
		while (node->next != NULL)
		{
			node = node->next;
		}
		node->next = *next_element;
	}
}

/* Function: check_ref_exits
 * ----------------------------
 *   Check whether for an element being set in an entry if the reference already exists
 *
 *   ref_array: either the forward or backwards reference array
 *   key: the key of the entry to chek the reference of
 * 	 size: the number of elements in the forward or backward array
 */
int check_ref_exits(entry **ref_array, char *key, int size)
{
	int i = 0;
	while (i < size)
	{
		if (strcmp(((ref_array)[i])->key, key) == 0)
		{
			return FALSE;
		}
		i++;
	}
	return TRUE;
}

/* Function: add_reference
 * ----------------------------
 *   For an entry with a forward reference, update the value in the forward reference linked list
 *
 *   forwardRef: the entry to forward ref to
 *   entry_node: the entry to add this forward reference to its forward reference linked list
 */
void add_reference(entry *forwardRef, entry *entry_node)
{
	if (entry_node->forward == NULL)
	{
		entry_node->forward = (entry **)calloc(sizeof(entry *), RESIZE); // Space for 5 entries
		entry_node->forward[0] = forwardRef;
		entry_node->forward_size = 1;
		entry_node->forward_max = RESIZE;
	}
	else
	{
		if (entry_node->forward_size == entry_node->forward_max)
		{
			entry_node->forward = (entry **)realloc(entry_node->forward, RESIZE * sizeof(entry *));
			entry_node->forward_max = entry_node->forward_max + RESIZE;
		}
		int unique = check_ref_exits((entry_node->forward), forwardRef->key, entry_node->forward_size);
		if (unique)
		{
			entry_node->forward[entry_node->forward_size] = forwardRef;
			entry_node->forward_size++;
		}
	}

	if (forwardRef->backward == NULL)
	{
		forwardRef->backward = (entry **)calloc(sizeof(entry *), RESIZE); // Space for 5 entries
		forwardRef->backward[0] = entry_node;
		forwardRef->backward_size++;
		forwardRef->backward_max = RESIZE;
	}
	else
	{
		if (forwardRef->backward_size == forwardRef->backward_max)
		{
			forwardRef->backward = (entry **)realloc(entry_node->forward, RESIZE * sizeof(entry *));
			forwardRef->backward_max = forwardRef->backward_max + RESIZE;
		}
		int unique = check_ref_exits((forwardRef->backward), entry_node->key, forwardRef->backward_size);
		if (unique)
		{
			forwardRef->backward[forwardRef->backward_size] = entry_node;
			forwardRef->backward_size++;
		}
	}
}

/* Function: print_forward
 * ----------------------------
 *   Print the forward references of an entry recursively
 *
 *   entry_found: the entry to print all its forward references
 *   cache: the cache to put the forward references in (makes sure we have no duplicates)
 *   numberOfAddedEntries: how many entries we put in the cache
 *   cacheSize: max size of the cache
 */
void print_forward(entry *entry_found, char **cache, int *numberOfAddedEntries, int cacheSize)
{

	int i = 0;
	while (i < entry_found->forward_size)
	{
		int j = 0;
		int notFound = TRUE;
		while (j < *numberOfAddedEntries)
		{
			if ((strcmp(entry_found->forward[i]->key, (cache[j])) == 0))
			{
				notFound = FALSE;
			}
			j++;
		}
		if (notFound)
		{
			cache[*numberOfAddedEntries] = entry_found->forward[i]->key;

			(*numberOfAddedEntries)++;
			print_forward(entry_found->forward[i], &(*cache), numberOfAddedEntries, cacheSize);
		}

		i++;
	}
}

/* Function: number_of_entries
 * ----------------------------
 *   Count the number of entries in the entry linked list
 *
 *   entry_head: the header pointer to the entry linked list
 */
int number_of_entries(entry *entry_head)
{
	int count = 0;
	while (entry_head != NULL)
	{
		count++;
		entry_head = entry_head->next;
	}
	return count;
}

// To compare strings - citation from https://stackoverflow.com/questions/23189630/how-to-use-qsort-for-an-array-of-strings
int sort_string(const void *str1, const void *str2)
{
	char *const *pp1 = str1;
	char *const *pp2 = str2;
	return strcmp(*pp1, *pp2);
}

// To sort an array of chars - citation from https://stackoverflow.com/questions/23189630/how-to-use-qsort-for-an-array-of-strings
void sort_util(char *lines[], int count)
{
	qsort(lines, count, sizeof(*lines), sort_string);
}

/* Function: get_forward
 * ----------------------------
 *   Get the forward references of an entry
 *
 *   line: the input line
 * 	 entry_head: pointer to the end of the entry linked list
 */
void get_forward(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	if (entry_found->forward_size == 0)
	{
		printf("nil\n");
		return;
	}
	int number = number_of_entries(*entry_head);
	int added_entries = 0;
	char **cache = (char **)malloc(sizeof(char *) * number);
	print_forward(entry_found, &(*cache), &added_entries, number);
	sort_util(cache, added_entries);

	for (int i = 0; i < added_entries; i++)
	{
		if (i != added_entries - 1)
		{
			printf("%s, ", cache[i]);
		}

		else
		{
			printf("%s\n", cache[i]);
		}
	}
	free(cache);
}

/* Function: print_backward
 * ----------------------------
 *   Print the backward references of an entry recursively
 *
 *   entry_found: the entry to print all its forward references
 *   cache: the cache to put the forward references in (makes sure we have no duplicates)
 *   added_entries: how many entries we put in the cache
 *   cache_size: max size of the cache
 */
void print_backward(entry *entry_found, char **cache, int *added_entries, int cache_size)
{

	int i = 0;
	while (i < entry_found->backward_size)
	{
		int j = 0;
		int notFound = TRUE;
		while (j < *added_entries)
		{
			if ((strcmp(entry_found->backward[i]->key, (cache[j])) == 0))
			{
				notFound = FALSE;
			}
			j++;
		}
		if (notFound)
		{
			cache[*added_entries] = entry_found->backward[i]->key;
			(*added_entries)++;
			print_backward(entry_found->backward[i], &(*cache), added_entries, cache_size);
		}

		i++;
	}
}

/* Function: get_backward
 * ----------------------------
 *   Get the backward references of an entry
 *
 *   line: the input line
 * 	 entry_head: pointer to the end of the entry linked list
 */
void get_backward(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	if (entry_found->backward_size == 0)
	{
		printf("nil\n");
		return;
	}
	int number = number_of_entries(*entry_head);
	int added_entries = 0;
	char **cache = (char **)malloc(sizeof(char *) * number);
	print_backward(entry_found, &(*cache), &added_entries, number);
	sort_util(cache, added_entries);
	for (int i = 0; i < added_entries; i++)
	{
		if (i != added_entries - 1)
		{
			printf("%s, ", cache[i]);
		}

		else
		{
			printf("%s\n", cache[i]);
		}
	}
	free(cache);
}

/* Function: negative_number
 * ----------------------------
 *   Returns TRUE/FALSE (1/0) on whether the string is negative or positive
 *
 *   input: the input line
 */
bool negative_number(char *input)
{
	return input[0] == '-';
}

/* Function: set_elements
 * ----------------------------
 *   Add the elements of an entry as a linked list.
 *
 *   element_head_values: the header node of the linked list
 *   is_simple: a pointer to the is_simple for the current entry
 *   found: the element value we are assigning
 *   entry_node: add reference to node
 *   entry_head: the entry linked list
 */
int set_elements(element **element_head_values, int *is_simple, char *found, entry *entry_node, entry *entry_head)
{
	element *temp_element = (element *)malloc(sizeof(element));
	if ((*found >= '0' && *found <= '9') || (negative_number(found)))
	{
		element_assign_integer(&temp_element, found);
		*is_simple = TRUE;
	}

	else
	{
		entry *found_entry = find_entry(&entry_head, found);
		if (found_entry == NULL)
		{
			free(temp_element);
			printf("no such key\n");
			return FALSE;
		}
		else
		{
			entry *forwardRef = element_assign_reference(&temp_element, found, entry_head);
			add_reference(forwardRef, entry_node);
		}
		*is_simple = FALSE;
	}

	add_element_to_value(&(*element_head_values), &temp_element);
	return TRUE;
}

/* Function: override_general
 * ----------------------------
 *   Override an entry.
 *
 *   entry_head: the header node of the linked list
 *   override_entry: the entry to override
 *   line: the input line
 */
void override_general(entry **entry_head, entry *override_entry, char *line)
{
	entry **ref_array = override_entry->forward;
	int i = 0;
	// In the forward references of the node to change, delete the backward references to it
	while (i < override_entry->forward_size)
	{
		entry *ref_modify = find_entry(&(*entry_head), ((ref_array)[i])->key);
		entry **remove_array = ref_modify->backward;
		int j = 0;
		while (j < ref_modify->backward_size)
		{
			if (strcmp(((remove_array)[j])->key, override_entry->key) == 0)
			{
				(remove_array)[j] = (remove_array)[ref_modify->backward_size - 1];
				(remove_array)[ref_modify->backward_size - 1] = NULL;
				ref_modify->backward_size--;
			}
			j++;
		}
		i++;
	}
	// Delete the forward reference of the override entry
	int k = 0;
	entry **remove_array = override_entry->forward;
	while (k < override_entry->forward_size)
	{
		(remove_array)[k] = NULL;
		k++;
	}

	clean_entry_override(override_entry);

	override_entry->forward_size = 0;
	override_entry->forward_max = 0;
	override_entry->forward = NULL;
	override_entry->values = NULL;
	override_entry->is_simple_change = TRUE;

	// Set the elements of the override entry
	// Denotes whether we have just have integers or forward/backward references
	int is_simple = TRUE;
	char *found = "";
	override_entry->length = 0;
	while ((found = strsep(&line, " \n")) != NULL && *found != '\0')
	{
		int valid = set_elements(&(override_entry->values), &is_simple, found, override_entry, *entry_head);
		if (override_entry->is_simple_change && is_simple == FALSE)
		{
			override_entry->is_simple = is_simple;
			override_entry->is_simple_change = FALSE;
		}
		if (valid)
		{
			override_entry->length += 1;
		}
		else
		{
			entry *toFree = override_entry;
			clean_entry(override_entry);
			free(toFree);
			return;
		}
	}
	printf("ok\n");
}

/* Function: set_command
 * ----------------------------
 *   Create an entry and add it to the entry linked list.
 *
 *   line: the input line
 *   entry_head: the header node of the entry linked list
 */
void set_command(char *line, entry **entry_head)
{
	char *found = strsep(&line, " \n");
	// First check all of the inputs are valid
	char *new_line = (char *)malloc(sizeof(char) * MAX_LINE);
	char *new_line_free = new_line;
	strcpy(new_line, line);
	char *line_check = "";
	while ((line_check = strsep(&new_line, " \n")) != NULL && *line_check != '\0')
	{
		if (!((*line_check >= '0' && *line_check <= '9') || (negative_number(line_check))))
		{
			if (strcmp(found, line_check) == 0)
			{
				printf("not permitted\n");
				free(new_line_free);
				return;
			}
			entry *find = find_entry(&(*entry_head), line_check);

			if (find == NULL)
			{
				printf("no such key\n");
				free(new_line_free);
				return;
			}
		}
	}
	free(new_line_free);

	// We have no elements in the linked list
	int createEntry = TRUE;
	entry *entry_found = find_entry(&(*entry_head), found);
	if (entry_found != NULL)
	{
		createEntry = FALSE;
	}

	if (createEntry)
	{
		// Create a new entry
		entry *new_entry = (entry *)malloc(sizeof(entry));
		new_entry->next = NULL;
		new_entry->values = NULL;
		new_entry->forward = NULL;
		new_entry->forward_size = 0;
		new_entry->forward_max = 0;
		new_entry->backward = NULL;
		new_entry->backward_size = 0;
		new_entry->backward_max = 0;
		new_entry->is_simple_change = TRUE;

		int is_simple = TRUE;

		// Set the key of entry
		strcpy(new_entry->key, found);
		new_entry->length = 0;
		// Add the values to the 'values' pointer in the new_entry struct as a linked list
		while ((found = strsep(&line, " \n")) != NULL && *found != '\0')
		{
			int valid = set_elements(&(new_entry->values), &is_simple, found, new_entry, *entry_head);
			if (new_entry->is_simple_change && is_simple == FALSE)
			{
				new_entry->is_simple = is_simple;
				new_entry->is_simple_change = FALSE;
			}
			if (valid)
			{
				new_entry->length += 1;
			}
			else
			{
				entry *to_free = new_entry;
				clean_entry(new_entry);
				free(to_free);
				return;
			}
		}

		printf("ok\n");

		add_entry(&(*entry_head), &new_entry);
		return;
	}

	// We know we have a duplicate entry
	override_general(&(*entry_head), entry_found, line);
}

/* Function: set_command
 * ----------------------------
 *   Print the elements of an entry.
 *
 *   node: the element to print
 *   first: whether the element in the first one
 */
void print_sequence(element *node, int *first)
{
	// If we are printing the first value don't print a space
	if (*first)
	{
		if (node->type == ENTRY)
		{
			printf("%s", node->entry->key);
		}
		else
		{
			printf("%d", node->value);
		}
		*first = FALSE;
	}
	else
	{
		if (node->type == ENTRY)
		{
			printf(" %s", node->entry->key);
		}
		else
		{
			printf(" %d", node->value);
		}
	}
}

/* Function: print_element_values
 * ----------------------------
 *   Prints the values of an entry given its header node
 *
 *   entry: the entry in the linked list
 *   node: the entry whose values we want to print
 */
void print_element_values(entry **entry)
{
	element *node = (*entry)->values;
	printf("[");
	int first = TRUE;

	if (node == NULL)
	{
		printf("]\n");
		return;
	}
	while (node->next != NULL)
	{
		print_sequence(node, &first);
		node = node->next;
	}

	print_sequence(node, &first);

	printf("]\n");
}

/* Function: get_command
 * ----------------------------
 *   Prints the values of a key
 *
 *   line: the input line
 *   entry_head: the head node of the entry linked list
 */
void get_command(char *line, entry **entry_head)
{
	char *found = strsep(&line, " \n");
	entry *found_entry = NULL;
	found_entry = find_entry(&(*entry_head), found);
	if (found_entry == NULL)
	{
		printf("no such key\n");
	}
	else
	{
		print_element_values(&found_entry);
	}
}

/* Function: append_command
 * ----------------------------
 *   Appends values to the end of an entry
 *
 *   line: the input line
 *   entry_head: the head node of the entry linked list
 */
void append_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	char *found = strsep(&line, " \n");
	while ((found) != NULL && *found != '\0')
	{
		entry *found_entry = NULL;
		found_entry = find_entry(&(*entry_head), key);
		if (found_entry == NULL)
		{
			printf("no such key\n");
			return;
		}
		else
		{
			int simple = FALSE;
			set_elements(&(found_entry->values), &simple, found, found_entry, *entry_head);
			found_entry->is_simple = simple;
		}

		found_entry->length += 1;
		found = strsep(&line, " \n");
	}
	printf("ok\n");
}

/* Function: element_assign_reference_pop
 * ----------------------------
 *   Reassign forward/backward references when we pop
 *
 *   reference_element: the reference array
 * 	 found: the input line
 *   entry_head: the head node of the entry linked list
 * 	 current: the entry we wan to do the reassigning on
 */
entry *element_assign_reference_pop(element **reference_element, char *found, entry *entry_head, entry *current)
{
	(*reference_element)->type = ENTRY;
	entry *found_entry = find_entry(&entry_head, found);
	(*reference_element)->entry = found_entry;
	add_reference(found_entry, current);
	(*reference_element)->next = NULL;
	return found_entry;
}

/* Function: push_command
 * ----------------------------
 *   Pushes values to the beginning of the entry
 *
 *   line: the input line
 *   entry_head: the head node of the entry linked list
 */
void push_element(char *found, entry **entry_head, entry *current)
{
	element **element_head_values = &(current->values);
	element *temp_element = (element *)malloc(sizeof(element));

	if ((*found >= '0' && *found <= '9') || (negative_number(found)))
	{
		element_assign_integer(&temp_element, found);
	}
	else
	{
		element_assign_reference_pop(&temp_element, found, *entry_head, current);
		current->is_simple = FALSE;
	}
	temp_element->next = (*element_head_values);
	(*element_head_values) = temp_element;
}

/* Function: push_command
 * ----------------------------
 *   Calls push element for each value to push.
 *
 *   line: the input line
 *   entry_head: the head node of the entry linked list
 */
void push_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	char *found = strsep(&line, " \n");
	while ((found) != NULL && *found != '\0')
	{
		entry *found_entry = NULL;
		found_entry = find_entry(&(*entry_head), key);
		if (found_entry == NULL)
		{
			printf("no such key\n");
			return;
		}

		push_element(found, entry_head, found_entry);
		found_entry->length += 1;
		found = strsep(&line, " \n");
	}
	printf("ok\n");
}

/* Function: print_reverse
 * ----------------------------
 *   Prints the keys and values in the linked list.
 *
 *   node: the entry to print
 *   key: boolean as to whether we are prinng the key and value or just the key
 */
void print_reverse(entry *node, int key)
{
	// Base case
	if (node == NULL)
		return;

	// Recursion on the next node till null
	print_reverse(node->next, key);

	// Print head
	if (key)
	{
		printf("%s ", node->key);
		print_element_values(&(node));
	}
	else
	{
		printf("%s\n", node->key);
	}
}

/* Function: list_snapshots
 * ----------------------------
 *   Prints the snapshots.
 *
 *   snapshot_node: the last snapshot in the snapshot linked list
 */
void list_snapshots(snapshot *snapshot_node)
{
	snapshot *moving_ptr = snapshot_node;
	snapshot *ptr = snapshot_node;
	int count = 0;
	while (moving_ptr->prev != NULL)
	{
		count++;
		moving_ptr = moving_ptr->prev;
	}
	if (count == 0)
	{
		printf("no snapshots\n");
		return;
	}
	else if (count == 1)
	{
		printf("1\n");
		return;
	}
	while (ptr->prev != NULL)
	{
		printf("%d\n", ptr->id - 1);
		ptr = ptr->prev;
	}
}

/* Function: list_command
 * ----------------------------
 *   Manages the list commands: entries and keys
 *
 *   line: the input line
 *   entry_head: the head node of the entry linked list
 */
void list_command(char *line, entry **entry_head, snapshot *snapshot_node)
{
	char *command = strsep(&line, " ");
	if (strncasecmp("KEYS", command, 4) == 0)
	{
		if (*entry_head == NULL)
		{
			printf("no keys\n");
		}
		else
		{
			print_reverse(&(*(*entry_head)), FALSE);
		}
	}
	else if (strncasecmp("ENTRIES", command, 7) == 0)
	{
		if (*entry_head == NULL)
		{
			printf("no entries\n");
		}
		else
		{
			print_reverse(&(*(*entry_head)), TRUE);
		}
	}
	else if (strncasecmp("SNAPSHOTS", command, 9) == 0)
	{
		list_snapshots(snapshot_node);
	}
}

/* Function: del_entry_in_linked_list
 * ----------------------------
 *   Removes an entry in the linked list
 *
 *  entry_head: the head node of the entry linked list
 *  key: key of the entry to remove
 */
void del_entry_in_linked_list(entry **entry_head, char *key, int purge)
{
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		if (!purge)
		{
			printf("no such key\n");
		}

		return;
	}

	int count = find_entry_count(*entry_head, key);
	int node_before = count - 1;
	if (count == 0)
	{
		(*entry_head) = entry_found->next;
	}
	else
	{
		entry *entry_before = find_entry_at_count(*entry_head, node_before);
		entry_before->next = &(*(entry_found)->next);
	}
	entry **forward_array = entry_found->forward;
	int i = 0;
	// Loop over forward entries
	while (i < entry_found->forward_size)
	{
		entry *to_del = ((forward_array)[i]);
		int j = 0;
		entry **to_del_backward = to_del->backward;
		// For the forward ref, go to its backward ref and delete the ref to the entry to delete
		while (j < to_del->backward_size)
		{
			entry *maybe_del = ((to_del_backward)[i]);
			if (strcmp(maybe_del->key, key) == 0)
			{
				to_del->backward_size = to_del->backward_size - 1;
				// Set the pointer to null
				(to_del_backward)[i] = NULL;
			}
			j++;
		}
		i++;
	}
	clean_entry(entry_found);
	free(entry_found);
	if (!purge)
	{
		printf("ok\n");
	}
}

/* Function: del_command
 * ----------------------------
 *   Manage flow for del command
 *
 * 	line: the input line
 *  entry_head: the head node of the entry linked list
 */
void del_command(char *line, entry **entry_head, int purge)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	if (entry_found->backward_size > 0)
	{
		printf("not permitted\n");
		return;
	}
	// We can delete
	del_entry_in_linked_list(&(*entry_head), key, FALSE);
}

/* Function: purge_command
 * ----------------------------
 *   Deletes the entry in all snapshots if it's valid
 *
 * 	line: the input line
 *  snapshot: the last node of the snapshot linked list (current snapshot)
 */
void purge_command(char *line, snapshot **snapshot_node)
{

	char *key = strsep(&line, " \n");
	snapshot *snapshot_pointer = *snapshot_node;
	snapshot *snapshot_ptr_valid = *snapshot_node;
	while (snapshot_ptr_valid != NULL)
	{
		entry *entry_found = find_entry(&(snapshot_ptr_valid->entries), key);
		if (entry_found != NULL && entry_found->backward_size > 0)
		{
			printf("not permitted\n");
			return;
		}
		snapshot_ptr_valid = snapshot_ptr_valid->prev;
	}
	while (snapshot_pointer != NULL)
	{
		del_entry_in_linked_list(&(snapshot_pointer->entries), key, TRUE);
		snapshot_pointer = snapshot_pointer->prev;
	}
	printf("ok\n");
}

/* Function: remove_first_value_of_entry
 * ----------------------------
 *  Removes the first value of an entry
 *
 *  entry_head: the head node of the entry linked list
 * 	key: the entry's key
 */
void remove_first_value_of_entry(entry **entry_head, char *key)
{
	if (*entry_head == NULL)
	{
		printf("no such key\n");
		return;
	}
	entry *entry_found = find_entry(&(*entry_head), key);
	if ((entry_found)->values == NULL)
	{
		printf("nil\n");
		return;
	}

	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}

	element *next_element = (entry_found)->values->next;
	printf("%d\n", (entry_found)->values->value);
	free(entry_found->values);
	(entry_found)->values = next_element;
}

/* Function: pick_command
 * ----------------------------
 *  Display a element at an index in the entry
 *
 *  line: the input line
 * 	entry_head: the head node of the entry linked list
 */
void pick_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	char *index = strsep(&line, " \n");
	int index_int = atoi(index);

	if (*entry_head == NULL)
	{
		printf("no such key\n");
		return;
	}

	entry *entry_found = find_entry(&(*entry_head), key);
	if ((entry_found)->values == NULL)
	{
		printf("nil\n");
		return;
	}

	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}

	if ((entry_found->length) < index_int || index_int < 1)
	{
		printf("index out of range\n");
		return;
	}

	element *node = (entry_found)->values;
	int i = 0;
	while (node != NULL && i != index_int - 1)
	{
		node = node->next;
		i++;
	}
	if (node->type == ENTRY)
	{
		printf("%s\n", node->entry->key);
		return;
	}
	printf("%d\n", node->value);
}

/* Function: find_element_index_entry
 * ----------------------------
 *  Display a element at an index in the entry
 *
 *  val: header to element linked list for an entry
 * 	index: the index to find
 *  returns the elemenet at an index for an entry
 */
element *find_element_index_entry(element *val, int index)
{
	int i = 0;
	while (val != NULL && i != index)
	{
		val = val->next;
		i++;
	}

	return val;
}

/* Function: pluck_command
 * ----------------------------
 *  Displays and removes value at index
 *
 *  line: the input line
 * 	entry_head: node to the first entry in the entry linked list
 *  pop: boolean on whether we need to remove the element or not
 */
void pluck_command(char *line, entry **entry_head, int pop)
{
	char *key = strsep(&line, " \n");
	char *index = strsep(&line, " \n");
	int index_int = atoi(index);
	if (pop)
	{
		index_int = 1;
	}
	if (*entry_head == NULL)
	{
		printf("no such key\n");
		return;
	}

	entry *entry_found = find_entry(&(*entry_head), key);

	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}

	else if ((entry_found)->values == NULL)
	{
		if (pop)
		{
			printf("nil\n");
		}
		else
		{
			printf("index out of range\n");
		}
		return;
	}

	if ((entry_found->length) < index_int || index_int < 1)
	{
		printf("index out of range\n");
		return;
	}

	element *node = (entry_found)->values;

	int i = 0;
	while (node != NULL && i != index_int - 1)
	{
		node = node->next;
		i++;
	}

	if (node->type == ENTRY)
	{
		printf("%s\n", node->entry->key);
	}
	else
	{
		printf("%d\n", node->value);
	}

	element *next_value = node->next;

	element *node_before = find_element_index_entry(entry_found->values, index_int - 2);

	entry_found->length -= 1;

	if (node_before == NULL)
	{
		entry_found->values = next_value;
	}
	else
	{
		node_before->next = next_value;
	}

	element *type_node = (entry_found)->values;
	int entryFound = FALSE;
	while (type_node != NULL)
	{
		if (type_node->type == ENTRY)
		{
			entryFound = TRUE;
		}
		type_node = type_node->next;
	}
	if (entryFound)
	{
		entry_found->is_simple = FALSE;
	}
	else
	{
		entry_found->is_simple = TRUE;
	}

	free(node);
}

/* Function: sort_command
 * ----------------------------
 *  Sorts the values of an entry in ascending order
 *
 *  line: the input line
 * 	entry_head: node to the first entry in the entry linked list
 */
void sort_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	int *sort_array = (int *)(malloc(sizeof(int) * entry_found->length));
	element *entry_val = entry_found->values;
	for (int i = 0; i < entry_found->length && entry_val != NULL; i++)
	{
		sort_array[i] = entry_val->value;
		entry_val = entry_val->next;
	}
	qsort(sort_array, entry_found->length, sizeof(int), cmp_func);

	entry_val = entry_found->values;
	for (int i = 0; i < entry_found->length && entry_val != NULL; i++)
	{
		entry_val->value = sort_array[i];
		entry_val = entry_val->next;
	}

	printf("ok\n");

	free(sort_array);
}

/* Function: uniq_command
 * ----------------------------
 *  Removes adjacent duplicate values in an entry
 *
 *  line: the input line
 * 	entry_head: node to the first entry in the entry linked list
 */
void uniq_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	element *entry_val = entry_found->values;
	element *prev_val = entry_found->values;
	for (int i = 0; i < entry_found->length && entry_val != NULL; i++)
	{

		if (i == 0)
		{
			entry_val = entry_val->next;
		}

		if (prev_val->value == entry_val->value)
		{
			prev_val->next = entry_val->next;
			element *temp = entry_val;
			entry_val = entry_val->next;
			free(temp);
		}

		else
		{
			entry_val = entry_val->next;
			prev_val = prev_val->next;
		}
	}
	printf("ok\n");
}

/* Function: rev_command
 * ----------------------------
 *  Reverses the values of an entry
 *
 *  line: the input line
 * 	entry_head: node to the first entry in the entry linked list
 */
void rev_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}

	int *reverse_array = (int *)(malloc(sizeof(int) * entry_found->length));
	int *value_array = (int *)(malloc(sizeof(int) * entry_found->length));

	element *entry_val = entry_found->values;

	for (int i = 0; i < entry_found->length && entry_val != NULL; i++)
	{
		value_array[i] = entry_val->value;
		entry_val = entry_val->next;
	}

	entry_val = entry_found->values;
	int length = entry_found->length;
	for (int i = 0; i < entry_found->length && entry_val != NULL; i++)
	{
		reverse_array[i] = value_array[length - 1 - i];
		entry_val = entry_val->next;
	}

	// Assign elements to reversed array
	entry_val = entry_found->values;
	for (int i = 0; i < entry_found->length && entry_val != NULL; i++)
	{
		entry_val->value = reverse_array[i];
		entry_val = entry_val->next;
	}

	printf("ok\n");

	free(reverse_array);
	free(value_array);
}

/* Function: snap_entry
 * ----------------------------
 *  Set the new entry's forward and backward references as per the old entry
 *
 *  entry_head: the entry to copy forward and backward references from
 *  found: the key of the entry to forward ref to
 *  temp_element: the copied element to change its forward and backward references
 * 	entry_node: the entry of the new snapshot
 */
void snap_entry(entry **entry_head, char *found, element *temp_element, entry *entry_node)
{
	entry *forward_reference = element_assign_reference(&temp_element, found, *entry_head);
	add_reference(forward_reference, entry_node);
}

/* Function: copy_entries
 * ----------------------------
 *  Copy's the entries and their elements from the current state to a new snapshot
 *
 *  old_snapshot: the snapshot to copy values from
 * 	new_snapshot: the snapshot to copy the values to
 */
void copy_entries(snapshot *old_snapshot, snapshot *new_snapshot)
{
	entry *current_entry_old_snapshot = old_snapshot->entries;
	entry *prev_entry = NULL; // Don't have a previous yet
	// While we have not reached the end of the entry linked list in the old snapshot
	// -- CREATE ENTRIES
	while (current_entry_old_snapshot != NULL)
	{
		entry *new_entry = (entry *)malloc(sizeof(entry));
		new_entry->next = NULL;
		strcpy(new_entry->key, current_entry_old_snapshot->key);
		new_entry->is_simple = current_entry_old_snapshot->is_simple;
		new_entry->length = current_entry_old_snapshot->length;
		new_entry->forward = NULL;
		new_entry->backward = NULL;
		new_entry->forward_size = 0;
		new_entry->backward_size = 0;
		new_entry->forward_max = 0;
		new_entry->backward_max = 0;
		new_entry->values = NULL;

		if (prev_entry == NULL)
		{
			new_snapshot->entries = new_entry;
		}
		else
		{
			prev_entry->next = new_entry;
		}
		prev_entry = new_entry;
		current_entry_old_snapshot = current_entry_old_snapshot->next;
	}

	current_entry_old_snapshot = old_snapshot->entries;

	entry *new_snapshot_entries = new_snapshot->entries;

	// --- FOR THE ENTRIES, ADD THE ELEMENTS
	current_entry_old_snapshot = old_snapshot->entries;
	while (new_snapshot_entries != NULL)
	{
		element *current_entry_old_snapshot_elements = current_entry_old_snapshot->values;
		element *prev_element = NULL; // Don't have a previous yet
		while (current_entry_old_snapshot_elements != NULL)
		{
			element *new_element = (element *)malloc(sizeof(element));
			new_element->next = NULL;
			if (prev_element == NULL)
			{
				new_snapshot_entries->values = new_element;
			}
			else
			{
				prev_element->next = new_element;
			}

			prev_element = new_element;
			new_element->type = current_entry_old_snapshot_elements->type;
			if (current_entry_old_snapshot_elements->type == INTEGER)
			{
				new_element->value = current_entry_old_snapshot_elements->value;
			}
			else
			{
				entry *found_entry = find_entry(&(new_snapshot->entries), current_entry_old_snapshot->key);
				snap_entry(&(new_snapshot->entries), current_entry_old_snapshot_elements->entry->key, new_element, found_entry);
			}

			current_entry_old_snapshot_elements = current_entry_old_snapshot_elements->next;
		}

		new_snapshot_entries = new_snapshot_entries->next;
		current_entry_old_snapshot = current_entry_old_snapshot->next;
	}
}

/* Function: snapshot_command
 * ----------------------------
 *  Creates a new snapshot object
 *
 *  snapshot_node: the snapshot to copy values from
 */
void snapshot_command(snapshot **snapshot_node)
{
	snapshot *snapshot_new = (snapshot *)malloc(sizeof(snapshot));
	snapshot_new->entries = NULL;
	(*snapshot_node)->next = snapshot_new;
	snapshot_new->prev = *snapshot_node;
	*snapshot_node = snapshot_new;
	snapshot_new->id = snapshot_new->prev->id + 1;
	copy_entries((*snapshot_node)->prev, *snapshot_node);
	snapshot_new->next = NULL;
	printf("saved as snapshot %d\n", snapshot_new->id - 1);
}

/* Function: get_snapshot_with_id
 * ----------------------------
 *  Creates a new snapshot object
 *
 *  snapshot_node: the snapshot to copy values from
 *  id: the snapshot id
 *  returns the snapshot with the id
 */
snapshot *get_snapshot_with_id(snapshot *snapshot_node, int id)
{
	snapshot *moving_node = snapshot_node;
	while (moving_node != NULL)
	{
		if (moving_node->id == id)
		{
			return moving_node;
		}
		moving_node = moving_node->prev;
	}
	return NULL;
}

/*  Function: find_mi
 * ----------------------------
 *  Finds the minimum element in an entry's values
 *
 *  node: the current element to find min on
 * 	returns: the minimum element
 */
int find_min(element *node)
{
	int current_min = 2147483647; // Max integer upperbound
	if (node->type == ENTRY)
	{
		current_min = find_min(node->entry->values);
	}
	current_min = node->value;
	node = node->next;
	while (node != NULL)
	{
		if (node->type == ENTRY)
		{
			int local_min = find_min(node->entry->values);
			if (local_min < current_min)
			{
				current_min = local_min;
			}
			node = node->next;
		}

		else if (node->value < current_min)
		{
			current_min = node->value;
		}
		else
		{
			node = node->next;
		}
	}
	return current_min;
}

/* Function: find_max
 * ----------------------------
 *  Finds the maximum element in an entry's values
 *
 *  node: the current element to find min on
 * 	returns: the maximum element
 */
int find_max(element *node)
{
	int current_max = -2147483648; // Min integer lowerbound

	if (node->type == ENTRY)
	{
		current_max = find_max(node->entry->values);
	}
	current_max = node->value;
	node = node->next;
	while (node != NULL)
	{
		if (node->type == ENTRY)
		{
			int local_max = find_max(node->entry->values);
			if (local_max > current_max)
			{
				current_max = local_max;
			}
			node = node->next;
		}

		else if (node->value > current_max)
		{
			current_max = node->value;
			node = node->next;
		}
		else
		{
			node = node->next;
		}
	}
	return current_max;
}

/* Function: aggregate_command
 * ----------------------------
 *  Calls either min or max depending on input
 *
 *  line: the the input line
 * 	aggregate: which aggregate to perform
 */
void aggregate_command(char *line, entry **entry_head, int aggregate)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	else if (aggregate == MIN)
	{
		printf("%d\n", find_min(entry_found->values));
	}
	else if (aggregate == MAX)
	{
		printf("%d\n", find_max(entry_found->values));
	}
}

/* Function: checkout_snapshot
 * ----------------------------
 *  Makes a copy of the snapshot to checkout to into the current state
 *
 *  line: the the input line
 * 	current_snapshot: the current snapshot to copy into
 */
void checkout_snapshot(char *line, snapshot **current_snapshot)
{
	char *key = strsep(&line, " \n");
	int id = atoi(key);
	snapshot *copy_from_id = get_snapshot_with_id(*current_snapshot, id);
	if (copy_from_id == NULL)
	{
		printf("no such snapshot\n");
		return;
	}
	clean_memory(&((*current_snapshot)->entries));
	(*current_snapshot)->entries = NULL;
	copy_entries(copy_from_id, *current_snapshot);
	printf("ok\n");
}

/* Function: delete_snapshot
 * ----------------------------
 *  Creates a new snapshot object
 *
 * 	delete_snap: the snapshot to delete
 */
void delete_snapshot(snapshot *delete_snap)
{
	if (delete_snap == NULL)
	{
		printf("no such snapshot\n");
		return;
	}

	if (delete_snap->prev == NULL)
	{
		delete_snap->next->prev = NULL;
		delete_snap->next = NULL;
	}

	else
	{
		delete_snap->next->prev = delete_snap->prev;
		delete_snap->prev->next = delete_snap->next;
	}

	clean_memory(&(delete_snap->entries));
}

/* Function: drop_snapshot
 * ----------------------------
 *  Deletes a snapshot
 *
 *  line the input line with the snapshot to delete
 * 	current_snapshot: the current snapshot in the snapshot linked list
 */
void drop_snapshot(char *line, snapshot **current_snapshot)
{
	char *key = strsep(&line, " \n");
	int id = atoi(key);
	if ((*current_snapshot)->entries == NULL)
	{
		printf("no such snapshot\n");
		return;
	}
	snapshot *delete_snap = get_snapshot_with_id(*current_snapshot, id);
	if (delete_snap == NULL)
	{
		printf("no such snapshot\n");
		return;
	}
	delete_snapshot(delete_snap);

	snapshot *temp = delete_snap;
	free(temp);
	printf("ok\n");
}

/* Function: rollback_snapshot
 * ----------------------------
 *  Restores to snapshot and deletes newer snapshots
 *
 *  line the input line with the snapshot to delete
 * 	current_snapshot: the current snapshot in the snapshot linked list
 */
void rollback_snapshot(char *line, snapshot **current_snapshot)
{
	char *key = strsep(&line, " \n");
	int id = atoi(key);
	if ((*current_snapshot)->entries == NULL)
	{
		printf("no such snapshot\n");
		return;
	}
	snapshot *copy_from_id = get_snapshot_with_id(*current_snapshot, id);
	if (copy_from_id == NULL)
	{
		printf("no such snapshot\n");
		return;
	}
	clean_memory(&((*current_snapshot)->entries));
	(*current_snapshot)->entries = NULL;
	copy_entries(copy_from_id, *current_snapshot);

	snapshot *next_node = copy_from_id->next;
	while (next_node->id != (*current_snapshot)->id)
	{
		snapshot *to_delete = next_node;
		next_node = next_node->next;
		delete_snapshot(to_delete);
		free(to_delete);
	}
	printf("ok\n");
}

/* Function: get_type
 * ----------------------------
 *  Gets the types of the entry
 *
 *  line the input line with the snapshot to delete
 * 	entry_head: the node in the beginning of the entry linked list
 */
void get_type(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	if (entry_found->is_simple == FALSE)
	{
		printf("general\n");
	}
	else
	{
		printf("simple\n");
	}
}

/* Function: sum_recursion
 * ----------------------------
 *  Sums the elements of an entry
 *
 * 	node: the current element to add to the sum
 * 	returns: the sum
 */
int sum_recursion(element *node)
{
	int count = 0;
	while (node != NULL)
	{
		if (node->type == ENTRY)
		{
			count += sum_recursion(node->entry->values);
			node = node->next;
		}
		else
		{
			count += node->value;
			node = node->next;
		}
	}
	return count;
}

/* Function: sum_command
 * ----------------------------
 *  Calls sum_command from an element recursively
 *
 * 	line: the entry to sum on
 * 	entry_head: the entry linked list
 */
void sum_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	int sum = sum_recursion(entry_found->values);
	printf("%d\n", sum);
}

/* Function: calculate_length
 * ----------------------------
 *  calculates the length of an entry
 *
 * 	node: the element of the node
 * 	return: the length linked list
 */
int calculate_length(element *node)
{
	int length = 0;
	while (node != NULL)
	{
		if (node->type == ENTRY)
		{
			length += calculate_length(node->entry->values);
			node = node->next;
		}
		else
		{
			length++;
			node = node->next;
		}
	}
	return length;
}

/* Function: length_command
 * ----------------------------
 *  Calls sum_command on the entry's elements
 *
 * 	node: the element of the node
 * 	return: the length linked list
 */
void length_command(char *line, entry **entry_head)
{
	char *key = strsep(&line, " \n");
	entry *entry_found = find_entry(&(*entry_head), key);
	if (entry_found == NULL)
	{
		printf("no such key\n");
		return;
	}
	int length = calculate_length(entry_found->values);
	printf("%d\n", length);
}

/* Function: parser
 * ----------------------------
 *   Processes command line input
 *
 *   line: the input line
 */
int parser(char *line, snapshot **snapshot_node)
{
	// Get the command from the user
	char *command = strsep(&line, " ");
	// SET
	if (strncasecmp("SET", command, 3) == 0)
	{
		set_command(line, &((*snapshot_node)->entries));
	}
	// GET
	else if (strncasecmp("GET", command, 3) == 0)
	{
		get_command(line, &((*snapshot_node)->entries));
	}
	// PUSH
	else if (strncasecmp("PUSH", command, 4) == 0)
	{
		push_command(line, &((*snapshot_node)->entries));
	}
	// APPEND
	else if (strncasecmp("APPEND", command, 6) == 0)
	{
		append_command(line, &((*snapshot_node)->entries));
	}
	// LIST KEYS
	else if (strncasecmp("LIST", command, 4) == 0)
	{
		list_command(line, &((*snapshot_node)->entries), *snapshot_node);
	}
	// DEL
	else if (strncasecmp("DEL", command, 3) == 0)
	{
		del_command(line, &((*snapshot_node)->entries), FALSE);
	}
	// PURGE
	else if (strncasecmp("PURGE", command, 3) == 0)
	{
		purge_command(line, snapshot_node);
	}
	// POP
	else if (strncasecmp("POP", command, 3) == 0)
	{
		pluck_command(line, &((*snapshot_node)->entries), TRUE);
	}
	// PICK
	else if (strncasecmp("PICK", command, 4) == 0)
	{
		pick_command(line, &((*snapshot_node)->entries));
	}
	// PLUCK
	else if (strncasecmp("PLUCK", command, 5) == 0)
	{
		pluck_command(line, &((*snapshot_node)->entries), FALSE);
	}
	// SORT
	else if (strncasecmp("SORT", command, 4) == 0)
	{
		sort_command(line, &((*snapshot_node)->entries));
	}
	// UNIQ
	else if (strncasecmp("UNIQ", command, 4) == 0)
	{
		uniq_command(line, &((*snapshot_node)->entries));
	}
	// REV
	else if (strncasecmp("REV", command, 3) == 0)
	{
		rev_command(line, &((*snapshot_node)->entries));
	}
	// SNAPSHOT
	else if (strncasecmp("SNAPSHOT", command, 8) == 0)
	{
		snapshot_command(&(*snapshot_node));
	}
	// SNAPSHOT
	else if (strncasecmp("CHECKOUT", command, 8) == 0)
	{
		checkout_snapshot(line, &(*snapshot_node));
	}
	// ROLLBACK
	else if (strncasecmp("ROLLBACK", command, 8) == 0)
	{
		rollback_snapshot(line, &(*snapshot_node));
	}
	// ROLLBACK
	else if (strncasecmp("DROP", command, 4) == 0)
	{
		drop_snapshot(line, &(*snapshot_node));
	}
	// MIN
	else if (strncasecmp("MIN", command, 3) == 0)
	{
		aggregate_command(line, &(*snapshot_node)->entries, MIN);
	}
	// MAX
	else if (strncasecmp("MAX", command, 3) == 0)
	{
		aggregate_command(line, &(*snapshot_node)->entries, MAX);
	}
	// FORWARD
	else if (strncasecmp("FORWARD", command, 7) == 0)
	{
		get_forward(line, &(*snapshot_node)->entries);
	}
	// BACKWARD
	else if (strncasecmp("BACKWARD", command, 8) == 0)
	{
		get_backward(line, &(*snapshot_node)->entries);
	}
	// TYPE
	else if (strncasecmp("TYPE", command, 8) == 0)
	{
		get_type(line, &(*snapshot_node)->entries);
	}
	// SUM
	else if (strncasecmp("SUM", command, 3) == 0)
	{
		sum_command(line, &(*snapshot_node)->entries);
	}
	// LEN
	else if (strncasecmp("LEN", command, 3) == 0)
	{
		length_command(line, &(*snapshot_node)->entries);
	}
	// HELP
	else if (strncasecmp("HELP", command, 4) == 0)
	{
		command_help();
	}
	// BYE
	else if (strncasecmp("BYE", command, 3) == 0)
	{
		command_bye();
		clean_snapshots(&(*snapshot_node));
		return FALSE;
	}
	printf("\n");
	return TRUE;
}

int main()
{
	char line[MAX_LINE];
	int loop = TRUE;

	// First snapshot, join new entries to snapshot->entries
	snapshot *snapshot_node = (snapshot *)malloc(sizeof(snapshot));
	snapshot_node->entries = NULL
								 snapshot_node->prev = NULL;
	snapshot_node->next = NULL;
	snapshot_node->id = 1;

	while (loop)
	{
		printf("> ");
		fgets(line, MAX_LINE, stdin);
		loop = parser(line, &snapshot_node);
	}
	free(snapshot_node);

	return 0;
}